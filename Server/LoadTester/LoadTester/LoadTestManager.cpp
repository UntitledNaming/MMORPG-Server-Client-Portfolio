#include "LoadTestManager.h"

#include <ws2tcpip.h>
#include <windows.h>
#include <cmath>
#include <cstdio>
#include <algorithm>

#include "ContentsProtocol.h"   // Common\Contents : AuthProtocol / FieldProtocol ids

#pragma comment(lib, "ws2_32.lib")

// ---- local constants (mirror Common\Contents\ContentsDefine.h) ------------
namespace
{
    // World spans the sector grid: offset .. offset + SECTOR_MAX * SECTOR_SIZE
    const float WORLD_MIN = 200000.f;                 // MAP_WORLD_OFFSET_X/Y
    const float WORLD_MAX = 200000.f + 160.f * 2500.f; // + SECTOR_X_MAX * SECTOR_SIZE = 600000
    const float WALK_SPEED = 600.f;                   // cm/s
    const float WALK_RADIUS = 5000.f;                 // ~2 sectors around spawn anchor
    const float PI_F = 3.14159265f;

    bool IsKnownType(uint16 t)
    {
        // Auth: 0..3, Field: 1000..1039 (see ContentsProtocol.h)
        return (t <= 3) || (t >= 1000 && t <= 1039);
    }
    bool CoordOk(float x, float y)
    {
        if (!std::isfinite(x) || !std::isfinite(y)) return false;
        return x >= WORLD_MIN - 2000.f && x <= WORLD_MAX + 2000.f
            && y >= WORLD_MIN - 2000.f && y <= WORLD_MAX + 2000.f;
    }
}

LoadTestManager::LoadTestManager(const LoadConfig& cfg)
    : m_cfg(cfg)
{
    for (int i = 0; i < ERR_CAT_COUNT; ++i)
        m_errors[i].store(0);
}

LoadTestManager::~LoadTestManager()
{
    if (m_iocp) CloseHandle(m_iocp);
    WSACleanup();
}

const char* LoadTestManager::ErrName(int cat)
{
    switch (cat)
    {
    case ERR_UNKNOWN_TYPE:     return "UnknownType";
    case ERR_LEN_MISMATCH:     return "LengthMismatch";
    case ERR_BAD_VALUE:        return "BadValue(HP/Coord)";
    case ERR_UNEXPECTED_CLOSE: return "UnexpectedClose";
    case ERR_SPAWN_TIMEOUT:    return "SpawnTimeout";
    case ERR_ID_CONSISTENCY:   return "IdConsistency";
    default:                   return "?";
    }
}

bool LoadTestManager::Init()
{
    WSADATA wd;
    if (WSAStartup(MAKEWORD(2, 2), &wd) != 0)
    {
        printf("[init] WSAStartup failed\n");
        return false;
    }

    srand(GetTickCount());

    m_clients.reset(new DummyClient[m_cfg.userCount]);
    for (int i = 0; i < m_cfg.userCount; ++i)
    {
        m_clients[i].index = i;
        m_clients[i].characterUID = static_cast<uint64>(i + 1); // characterUID 1..N
    }

    m_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (m_iocp == NULL)
    {
        printf("[init] CreateIoCompletionPort failed\n");
        return false;
    }

    m_running.store(true);

    unsigned hw = std::thread::hardware_concurrency();
    unsigned workerCount = hw ? hw : 4;
    for (unsigned i = 0; i < workerCount; ++i)
        m_workers.emplace_back(&LoadTestManager::WorkerLoop, this);

    m_behavior = std::thread(&LoadTestManager::BehaviorLoop, this);
    m_stats = std::thread(&LoadTestManager::StatsLoop, this);

    printf("[init] ok. workers=%u, target=%s:%u, users=%d\n",
        workerCount, m_cfg.ip.c_str(), m_cfg.port, m_cfg.userCount);
    return true;
}

void LoadTestManager::Run()
{
    DWORD startTick = GetTickCount();

    // ---- ramp up: accept() on the server is a single serial loop, so opening
    // thousands at once would back up regardless of server health. Stagger it.
    printf("[run] connecting %d users at %d/sec ...\n", m_cfg.userCount, m_cfg.rampPerSec);
    for (int i = 0; i < m_cfg.userCount; ++i)
    {
        ConnectOne(m_clients[i]);
        if (m_cfg.rampPerSec > 0 && ((i + 1) % m_cfg.rampPerSec) == 0)
            Sleep(1000);
    }
    printf("[run] ramp done. simulating for %d sec ...\n", m_cfg.durationSec);

    bool churnEnabled = (m_cfg.rstPercent > 0);
    bool singleWaveDone = false;
    DWORD nextChurnTick = startTick + (DWORD)m_cfg.rstAfterSec * 1000;

    // reconnects are staggered just like the initial ramp so a big wave doesn't
    // stall this loop with a burst of blocking connect() calls.
    int reconnPerPass = (m_cfg.rampPerSec > 0) ? (m_cfg.rampPerSec / 10 + 1) : 100000;

    while (m_running.load())
    {
        DWORD now = GetTickCount();
        DWORD el = now - startTick;

        // --- churn: RST a % of currently in-field bots ---
        if (churnEnabled && !singleWaveDone && (int)(now - nextChurnTick) >= 0)
        {
            int target = (m_inField.load() * m_cfg.rstPercent) / 100;
            int done = 0;
            for (int i = 0; i < m_cfg.userCount && done < target; ++i)
            {
                DummyClient& c = m_clients[i];
                if (!c.closed.load() && c.state == ConnState::InField)
                {
                    RequestRST(c);   // schedules a reconnect if cooldown > 0
                    ++done;
                }
            }
            printf("[run] RST wave: %d connections (reconnect=%s)\n",
                done, m_cfg.reconnectCooldownSec > 0 ? "yes" : "no");

            if (m_cfg.churnIntervalSec > 0)
                nextChurnTick = now + (DWORD)m_cfg.churnIntervalSec * 1000; // repeat
            else
                singleWaveDone = true;                                      // one-shot
        }

        // --- reconnect RST'd bots whose cooldown elapsed (staggered) ---
        if (m_cfg.reconnectCooldownSec > 0 && !m_shuttingDown.load())
        {
            int reconnDone = 0;
            for (int i = 0; i < m_cfg.userCount && reconnDone < reconnPerPass; ++i)
            {
                DummyClient& c = m_clients[i];
                if (c.state == ConnState::Dead && c.reconnectAtTick != 0
                    && (int)(now - c.reconnectAtTick) >= 0)
                {
                    ResetForReconnect(c);
                    ++reconnDone;
                }
            }
        }

        if (el >= (DWORD)m_cfg.durationSec * 1000)
            break;

        Sleep(100);
    }

    // ---- shutdown -----------------------------------------------------------
    double elapsed = (GetTickCount() - startTick) / 1000.0;
    m_shuttingDown.store(true);
    m_running.store(false);

    for (int i = 0; i < m_cfg.userCount; ++i)
        HandleDisconnect(m_clients[i], false); // graceful close (no error counted)

    // wake workers blocked on GQCS
    for (size_t i = 0; i < m_workers.size(); ++i)
        PostQueuedCompletionStatus(m_iocp, 0, 0, NULL);

    for (auto& t : m_workers) if (t.joinable()) t.join();
    if (m_behavior.joinable()) m_behavior.join();
    if (m_stats.joinable())    m_stats.join();

    PrintReport(elapsed);
}

// ---------------------------------------------------------------------------
// connection lifecycle
// ---------------------------------------------------------------------------
void LoadTestManager::ConnectOne(DummyClient& c)
{
    c.sock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (c.sock == INVALID_SOCKET)
        return;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_cfg.port);
    inet_pton(AF_INET, m_cfg.ip.c_str(), &addr.sin_addr);

    if (connect(c.sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        closesocket(c.sock);
        c.sock = INVALID_SOCKET;
        return;
    }

    if (CreateIoCompletionPort((HANDLE)c.sock, m_iocp, (ULONG_PTR)&c, 0) == NULL)
    {
        closesocket(c.sock);
        c.sock = INVALID_SOCKET;
        return;
    }

    c.connectTick = GetTickCount();
    c.state = ConnState::LoggedIn;     // we send login+select right away
    m_connected.fetch_add(1);

    PostRecv(c);                       // catch the spawn stream
    SendLoginAndSelect(c);
}

void LoadTestManager::PostRecv(DummyClient& c)
{
    if (c.closed.load()) return;

    memset(&c.recvOv, 0, sizeof(c.recvOv));
    c.wsaRecv.buf = c.recvBuf + c.recvUsed;
    c.wsaRecv.len = (ULONG)(sizeof(c.recvBuf) - c.recvUsed);

    DWORD flags = 0, bytes = 0;
    int r = WSARecv(c.sock, &c.wsaRecv, 1, &bytes, &flags, &c.recvOv, NULL);
    if (r == SOCKET_ERROR)
    {
        int e = WSAGetLastError();
        if (e != WSA_IO_PENDING)
            HandleDisconnect(c, true);
    }
}

void LoadTestManager::HandleDisconnect(DummyClient& c, bool serverInitiated)
{
    // single-close guard: whichever thread gets here first does the cleanup.
    if (c.closed.exchange(true))
        return;

    if (serverInitiated && !m_shuttingDown.load() && !c.rstRequested)
        m_errors[ERR_UNEXPECTED_CLOSE].fetch_add(1);

    if (c.state == ConnState::InField)
        m_inField.fetch_sub(1);

    if (c.sock != INVALID_SOCKET)
    {
        closesocket(c.sock);
        c.sock = INVALID_SOCKET;
    }
    m_connected.fetch_sub(1);
    c.state = ConnState::Dead;
}

void LoadTestManager::RequestRST(DummyClient& c)
{
    if (c.closed.load()) return;
    c.rstRequested = true;

    // linger 0 -> closesocket sends RST instead of a graceful FIN
    linger lg;
    lg.l_onoff = 1;
    lg.l_linger = 0;
    setsockopt(c.sock, SOL_SOCKET, SO_LINGER, (char*)&lg, sizeof(lg));

    HandleDisconnect(c, false); // we initiated it -> not counted as a server error

    // schedule the comeback (cooldown lets the server fully release the old
    // session first, avoiding a same-UID duplicate-login overlap).
    if (m_cfg.reconnectCooldownSec > 0)
        c.reconnectAtTick = GetTickCount() + (DWORD)m_cfg.reconnectCooldownSec * 1000;
}

void LoadTestManager::ResetForReconnect(DummyClient& c)
{
    // wipe per-session state but KEEP origin anchor / characterUID / index,
    // so the bot walks around the same monster sector forever instead of
    // drifting away with each logout-saved position.
    c.closed.store(false);
    c.rstRequested = false;
    c.recvUsed = 0;
    c.spawnReceived = false;
    c.spawnTimeoutCounted = false;
    c.reconnectAtTick = 0;
    c.knownOthers.clear();
    c.state = ConnState::Disconnected;

    m_reconnects.fetch_add(1);
    ConnectOne(c);   // fresh socket + login + select
}

// ---------------------------------------------------------------------------
// IOCP worker: recv completions
// ---------------------------------------------------------------------------
void LoadTestManager::WorkerLoop()
{
    for (;;)
    {
        DWORD bytes = 0;
        ULONG_PTR key = 0;
        OVERLAPPED* ov = nullptr;

        BOOL ok = GetQueuedCompletionStatus(m_iocp, &bytes, &key, &ov, 1000);

        if (key == 0) // shutdown sentinel or 1s timeout
        {
            if (m_shuttingDown.load()) break;
            continue;
        }

        DummyClient* c = reinterpret_cast<DummyClient*>(key);

        if (!ok)                     // failed completion (RST / reset)
        {
            HandleDisconnect(*c, true);
            continue;
        }
        if (bytes == 0)              // graceful close by peer
        {
            HandleDisconnect(*c, true);
            continue;
        }

        m_recvBytes.fetch_add(bytes);
        c->recvUsed += bytes;
        ParseFrames(*c);

        if (!c->closed.load())
            PostRecv(*c);
    }
}

// ---------------------------------------------------------------------------
// framing + validation
// ---------------------------------------------------------------------------
void LoadTestManager::ParseFrames(DummyClient& c)
{
    for (;;)
    {
        if (c.recvUsed < (int)sizeof(GAMELIB_LANHEADER))
            break;

        GAMELIB_LANHEADER h;
        memcpy(&h, c.recvBuf, sizeof(h));

        // a length that can't fit our buffer means a corrupt / desynced stream
        if (h.s_len > (int)sizeof(c.recvBuf) - (int)sizeof(h))
        {
            m_errors[ERR_LEN_MISMATCH].fetch_add(1);
            HandleDisconnect(c, false);
            return;
        }

        int full = (int)sizeof(h) + h.s_len;
        if (c.recvUsed < full)
            break; // wait for the rest of this frame

        PacketReader r(c.recvBuf + sizeof(h), h.s_len);
        uint16 type = r.GetU16();
        m_recvPackets.fetch_add(1);
        HandlePacket(c, type, r);

        memmove(c.recvBuf, c.recvBuf + full, c.recvUsed - full);
        c.recvUsed -= full;
    }
}

void LoadTestManager::HandlePacket(DummyClient& c, uint16 type, PacketReader& r)
{
    using namespace FieldProtocol;

    switch (type)
    {
    case PACKET_SC_CREATE_MY_CHARACTER:
    {
        uint64 id = r.GetU64();
        float x = r.GetFloat(), y = r.GetFloat(), z = r.GetFloat(), yaw = r.GetFloat();
        int16 hp = r.GetI16(), mp = r.GetI16();
        uint16 lv = r.GetU16(); int32 exp = r.GetI32();
        (void)id; (void)yaw; (void)mp; (void)lv; (void)exp;
        if (!r.Ok()) { m_errors[ERR_LEN_MISMATCH].fetch_add(1); break; }
        if (!CoordOk(x, y) || hp < 0) m_errors[ERR_BAD_VALUE].fetch_add(1);
        // (variable inventory/equip/quickslot tail is intentionally not parsed;
        //  the frame is consumed by header length, so this is safe.)
        // current position always follows the server's spawn point...
        c.x = x; c.y = y; c.z = z;
        // ...but the random-walk anchor is pinned on the FIRST spawn and kept
        // across reconnects, so logout-position drift can't accumulate and the
        // bot stays in its monster sector even under continuous churn.
        if (!c.originSet)
        {
            c.originSet = true;
            c.anchorX = x;
            c.anchorY = y;
        }
        if (!c.spawnReceived)
        {
            c.spawnReceived = true;
            c.headingRad = (float)(rand() % 628) / 100.f; // 0..2pi
            c.state = ConnState::InField;
            m_inField.fetch_add(1);
            uint32 now = GetTickCount();
            c.nextMoveTick = now + (m_cfg.moveIntervalMs ? rand() % m_cfg.moveIntervalMs : 0);
            c.nextAttackTick = now + (m_cfg.attackIntervalMs ? rand() % m_cfg.attackIntervalMs : 0);
            c.nextRttTick = now + (m_cfg.rttIntervalMs ? rand() % m_cfg.rttIntervalMs : 0);
        }
        break;
    }
    case PACKET_SC_CREATE_OTHER_CHARACTER:
    {
        uint64 id = r.GetU64();
        float x = r.GetFloat(), y = r.GetFloat(), z = r.GetFloat(), yaw = r.GetFloat();
        int16 hp = r.GetI16(), maxhp = r.GetI16();
        uint8 mf = r.GetU8();
        (void)z; (void)yaw; (void)mf;
        if (!r.Ok() || r.Remain() != 0) { m_errors[ERR_LEN_MISMATCH].fetch_add(1); break; }
        if (!CoordOk(x, y) || maxhp <= 0 || hp < 0 || hp > maxhp)
            m_errors[ERR_BAD_VALUE].fetch_add(1);
        if (!c.knownOthers.insert(id).second)
            m_errors[ERR_ID_CONSISTENCY].fetch_add(1); // duplicate create
        break;
    }
    case PACKET_SC_DELETE_CHARACTER:
    {
        uint64 id = r.GetU64();
        if (!r.Ok() || r.Remain() != 0) { m_errors[ERR_LEN_MISMATCH].fetch_add(1); break; }
        if (c.knownOthers.erase(id) == 0)
            m_errors[ERR_ID_CONSISTENCY].fetch_add(1); // delete of unknown id
        break;
    }
    case PACKET_SC_RTT_ECHO:
    {
        uint64 t = r.GetU64();
        if (!r.Ok() || r.Remain() != 0) { m_errors[ERR_LEN_MISMATCH].fetch_add(1); break; }
        double lat = (double)(GetTickCount64() - t);
        {
            std::lock_guard<std::mutex> lk(m_rttMtx);
            m_rttSamples.push_back(lat);
        }
        break;
    }
    case PACKET_SC_ATTACK_HIT_RESULT:
    {
        uint8 pc = r.GetU8(), mc = r.GetU8();
        for (int i = 0; i < pc; ++i) { r.GetU64(); int16 nh = r.GetI16(); if (r.Ok() && nh < 0) m_errors[ERR_BAD_VALUE].fetch_add(1); }
        for (int i = 0; i < mc; ++i) { r.GetU64(); int16 nh = r.GetI16(); if (r.Ok() && nh < 0) m_errors[ERR_BAD_VALUE].fetch_add(1); }
        if (!r.Ok() || r.Remain() != 0) m_errors[ERR_LEN_MISMATCH].fetch_add(1);
        break;
    }
    default:
        // Other known packets (monster create/move, sync, skill, exp, item...)
        // are accepted without deep parsing to avoid false positives.
        if (!IsKnownType(type))
            m_errors[ERR_UNKNOWN_TYPE].fetch_add(1);
        break;
    }
}

// ---------------------------------------------------------------------------
// outgoing
// ---------------------------------------------------------------------------
void LoadTestManager::SendRaw(DummyClient& c, const PacketWriter& w)
{
    if (c.closed.load()) return;

    const char* d = const_cast<PacketWriter&>(w).Data();
    int len = w.Length();
    int off = 0;
    while (off < len)
    {
        int s = send(c.sock, d + off, len - off, 0);
        if (s <= 0) { HandleDisconnect(c, false); return; }
        off += s;
    }
    m_sentPackets.fetch_add(1);
    m_sentBytes.fetch_add(len);
}

void LoadTestManager::SendLoginAndSelect(DummyClient& c)
{
    // 1) login request: 64-byte token (server ignores its content for now)
    {
        char token[64] = { 0 };
        PacketWriter w(AuthProtocol::PACKET_CS_GAME_LOGIN_REQ);
        w.PutBytes(token, sizeof(token));
        SendRaw(c, w);
    }
    // 2) character select by UID (server loads from DB, moves session to Field)
    {
        PacketWriter w(AuthProtocol::PACKET_CS_GAME_CHARACTER_SELECT);
        w.PutU64(c.characterUID);
        SendRaw(c, w);
    }
}

void LoadTestManager::DoMove(DummyClient& c, uint32 now)
{
    float step = WALK_SPEED * (m_cfg.moveIntervalMs / 1000.f);
    int roll = rand() % 100;

    if (roll < 10) // brief stop
    {
        c.moving = false;
    }
    else
    {
        c.moving = true;
        if (roll < 35) // occasional heading wiggle
            c.headingRad += ((rand() % 200 - 100) / 100.f);

        float nx = c.x + cosf(c.headingRad) * step;
        float ny = c.y + sinf(c.headingRad) * step;

        // stay within WALK_RADIUS of the spawn anchor
        float dx = nx - c.anchorX, dy = ny - c.anchorY;
        if (sqrtf(dx * dx + dy * dy) > WALK_RADIUS)
        {
            c.headingRad += PI_F; // turn back toward the anchor
            nx = c.x + cosf(c.headingRad) * step;
            ny = c.y + sinf(c.headingRad) * step;
        }
        // clamp to world bounds
        nx = std::max(WORLD_MIN + 100.f, std::min(WORLD_MAX - 100.f, nx));
        ny = std::max(WORLD_MIN + 100.f, std::min(WORLD_MAX - 100.f, ny));
        c.x = nx; c.y = ny;
    }

    float yawDeg = c.headingRad * 180.f / PI_F;
    PacketWriter w(FieldProtocol::PACKET_CS_UPDATE_CHARACTER_MOVEMENT_INPUT);
    w.PutFloat(c.x);
    w.PutFloat(c.y);
    w.PutFloat(c.z);
    w.PutFloat(yawDeg);
    w.PutU8(c.moving ? 1 : 0);
    SendRaw(c, w);
}

void LoadTestManager::DoAttack(DummyClient& c, uint32 now)
{
    float yawDeg = c.headingRad * 180.f / PI_F;
    PacketWriter w(FieldProtocol::PACKET_CS_SWING_LEFT_ATTACK);
    w.PutFloat(yawDeg);
    w.PutU8((uint8)(rand() % 4 + 1)); // swingIdx must be 1..4 (server disconnects otherwise)
    SendRaw(c, w);
}

void LoadTestManager::DoRtt(DummyClient& c, uint32 now)
{
    PacketWriter w(FieldProtocol::PACKET_CS_RTT_SEND);
    w.PutU64((uint64)GetTickCount64());
    SendRaw(c, w);
}

// ---------------------------------------------------------------------------
// behavior + stats threads
// ---------------------------------------------------------------------------
void LoadTestManager::BehaviorLoop()
{
    while (m_running.load())
    {
        uint32 now = GetTickCount();
        for (int i = 0; i < m_cfg.userCount; ++i)
        {
            DummyClient& c = m_clients[i];
            if (c.closed.load() || c.state != ConnState::InField)
                continue;

            if (m_cfg.moveIntervalMs > 0 && (int)(now - c.nextMoveTick) >= 0)
            {
                DoMove(c, now);
                c.nextMoveTick = now + m_cfg.moveIntervalMs + (rand() % (m_cfg.moveIntervalMs / 2 + 1));
            }
            if (m_cfg.attackIntervalMs > 0 && (int)(now - c.nextAttackTick) >= 0)
            {
                DoAttack(c, now);
                c.nextAttackTick = now + m_cfg.attackIntervalMs + (rand() % (m_cfg.attackIntervalMs / 2 + 1));
            }
            if (m_cfg.rttIntervalMs > 0 && (int)(now - c.nextRttTick) >= 0)
            {
                DoRtt(c, now);
                c.nextRttTick = now + m_cfg.rttIntervalMs;
            }
        }
        Sleep(15);
    }
}

void LoadTestManager::StatsLoop()
{
    long long lastRecvP = 0, lastSentP = 0;
    while (m_running.load())
    {
        Sleep(1000);

        // spawn-timeout sweep
        uint32 now = GetTickCount();
        for (int i = 0; i < m_cfg.userCount; ++i)
        {
            DummyClient& c = m_clients[i];
            if (c.state == ConnState::LoggedIn && !c.spawnTimeoutCounted
                && (int)(now - c.connectTick) > 5000)
            {
                m_errors[ERR_SPAWN_TIMEOUT].fetch_add(1);
                c.spawnTimeoutCounted = true;
            }
        }

        // rtt percentiles over this window
        std::vector<double> samples;
        {
            std::lock_guard<std::mutex> lk(m_rttMtx);
            samples.swap(m_rttSamples);
        }
        double p50 = -1, p95 = -1, p99 = -1;
        if (!samples.empty())
        {
            std::sort(samples.begin(), samples.end());
            auto pick = [&](double q) { return samples[(size_t)(q * (samples.size() - 1))]; };
            p50 = pick(0.50); p95 = pick(0.95); p99 = pick(0.99);
        }

        long long recvP = m_recvPackets.load();
        long long sentP = m_sentPackets.load();
        long long recvPS = recvP - lastRecvP;
        long long sentPS = sentP - lastSentP;
        lastRecvP = recvP; lastSentP = sentP;

        long long errTotal = 0;
        for (int i = 0; i < ERR_CAT_COUNT; ++i) errTotal += m_errors[i].load();

        printf("[stat] conn=%d field=%d | tx/s=%lld rx/s=%lld | rtt(ms) p50=%.1f p95=%.1f p99=%.1f | err=%lld\n",
            m_connected.load(), m_inField.load(), sentPS, recvPS, p50, p95, p99, errTotal);
    }
}

void LoadTestManager::PrintReport(double elapsedSec)
{
    printf("\n================ LOAD TEST REPORT ================\n");
    printf("elapsed            : %.1f sec\n", elapsedSec);
    printf("target users       : %d\n", m_cfg.userCount);
    printf("sent packets/bytes : %lld / %lld\n", m_sentPackets.load(), m_sentBytes.load());
    printf("recv packets/bytes : %lld / %lld\n", m_recvPackets.load(), m_recvBytes.load());
    printf("reconnects         : %lld\n", m_reconnects.load());
    if (elapsedSec > 0)
    {
        printf("avg tx/s, rx/s     : %.0f / %.0f\n",
            m_sentPackets.load() / elapsedSec, m_recvPackets.load() / elapsedSec);
    }
    printf("-------- detected server errors --------\n");
    long long total = 0;
    for (int i = 0; i < ERR_CAT_COUNT; ++i)
    {
        long long v = m_errors[i].load();
        total += v;
        printf("  %-20s : %lld\n", ErrName(i), v);
    }
    printf("  %-20s : %lld\n", "TOTAL", total);
    printf("=================================================\n");
}
