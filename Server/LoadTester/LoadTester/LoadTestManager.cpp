#define NOMINMAX              // windows.h의 min/max 매크로가 std::min/std::max를 깨지 않게(첫 include 전에 정의 필수)
#include "LoadTestManager.h"

#include <ws2tcpip.h>          // inet_pton 등
#include <windows.h>           // IOCP, GetTickCount, Sleep 등 Win32
#include <cmath>               // cosf/sinf/sqrtf, std::isfinite
#include <cstdio>              // printf
#include <cstdarg>             // va_list (ReportError의 가변인자)
#include <cwchar>              // _vsnwprintf_s (와이드 포맷)
#include <string>              // (간접) 문자열 유틸
#include <unordered_map>       // LogClass.h가 내부적으로 요구 → 먼저 포함시켜 줌
#include <algorithm>           // std::min/std::max/std::sort

#include "ContentsProtocol.h"   // Common\Contents : AuthProtocol / FieldProtocol 패킷 id
#include "LogClass.h"           // NetworkLib\common_files : 파일 로그(LOG 매크로). windows.h/unordered_map 뒤에 와야 함

#pragma comment(lib, "ws2_32.lib")  // 윈속 라이브러리 링크

// ---- 로컬 상수 (Common\Contents\ContentsDefine.h 값을 그대로 복제) ----------
namespace
{
    // 월드는 섹터 격자 전체에 걸쳐 있다: 오프셋 .. 오프셋 + SECTOR_MAX * SECTOR_SIZE
    const float WORLD_MIN = 200000.f;                 // MAP_WORLD_OFFSET_X/Y (월드 시작 좌표)
    const float WORLD_MAX = 200000.f + 160.f * 2500.f; // + SECTOR_X_MAX * SECTOR_SIZE = 600000 (월드 끝)
    const float WALK_SPEED = 600.f;                   // 봇 이동 속도 cm/s (로컬 클라 MaxWalkSpeed와 통일)
    const float WALK_RADIUS = 5000.f;                 // 스폰 anchor 주변 약 2섹터 반경 안에서만 걷기
    const float PI_F = 3.14159265f;
    const uint32 RESPAWN_REQ_DELAY_MS = 1500;         // 죽은 뒤 부활 요청까지 대기(사망화면 흉내)

    // 받은 패킷 타입 id가 우리가 아는 범위인지. 모르는 값이면 서버 이상으로 의심.
    bool IsKnownType(uint16 t)
    {
        // Auth: 0..3, Field: 1000..1041 (ContentsProtocol.h 참고; 1041 = SC_HIT_TO_OTHERPLAYER)
        return (t <= 3) || (t >= 1000 && t <= 1041);
    }

    // 좌표가 정상(유한값 + 월드 범위 안, 여유 2000cm)인지. NaN/무한/월드 밖이면 false.
    bool CoordOk(float x, float y)
    {
        if (!std::isfinite(x) || !std::isfinite(y)) return false; // NaN/무한 차단
        return x >= WORLD_MIN - 2000.f && x <= WORLD_MAX + 2000.f
            && y >= WORLD_MIN - 2000.f && y <= WORLD_MAX + 2000.f;
    }

    // ---- 슬롯 타입 상수 (Common\Contents\ContentsEnum.h 의 SLOT_TYPE 값 복제) -------------------
    // 봇이 아이템 요청을 보낼 때 쓴다. 서버는 정의 밖 값이면 Disconnect 하므로 정확히 맞춰야 한다.
    const uint8 SLOT_INVENTORY = 1;   // SLOT_TYPE::INVENTORY
    const uint8 SLOT_EQUIPMENT = 2;   // SLOT_TYPE::EQUIPMENT
    const uint8 SLOT_QUICKSLOT = 3;   // SLOT_TYPE::QUICKSLOT
    const int   INVENTORY_SLOT_MAX = 40;  // UserInventory::INVENTORY_SLOT_MAX
    const int   QUICK_SLOT_MAX     = 2;   // UserQuickSlot::QUICK_SLOT_MAX
    const int   SKILL_SLOT_COUNT   = 4;   // UserConst::USER_SKILL_SLOT_COUNT (0=버프, 1~3=공격)
    const int16 CLEANUP_SLOT_FIRST = 4;   // 인벤 청소 시작 슬롯(시드 슬롯 0~3은 건너뜀)
    const int   USER_MAX_LEVEL     = 10;  // UserConst::USER_MAX_LEVEL

    // ---- 아이템 메타 테이블 (서버 ItemTable.cpp 의 BASESET 복제) -----------------------------
    // 서버 응답에 실려오는 아이템이 "테이블에 존재하는 itemID인지, 개수가 maxStack을 넘지 않는지"를
    // 더미 단독으로 검증하기 위한 최소 사본. 서버를 끌어오지 않고 값만 복제한다(ByteStream.h 설계 의도와 동일).
    struct ItemMeta
    {
        uint32 itemID;
        uint8  itemType;   // 1=CONSUMABLE, 2=EQUIPMENT (ITEM_TYPE 값)
        uint16 maxStack;   // 이 개수를 넘는 count가 오면 서버 버그(BadValue)
    };
    const ItemMeta g_itemMeta[] =
    {
        { 10001, 1, 500 }, // SMALL_HP_POTION (소비, 최대 500스택 - 부하테스트용 연속 소비)
        { 10002, 1, 500 }, // SMALL_MP_POTION (소비, 최대 500스택 - 부하테스트용 연속 소비)
        { 10003, 2, 1  }, // NORMAL_HELMET   (장비, 1스택)
        { 10004, 2, 1  }, // NORMAL_CHEST
        { 10005, 2, 1  }, // NORMAL_PANTS
        { 10006, 2, 1  }, // NORMAL_BOOTS
        { 10007, 2, 1  }, // NORMAL_WEAPON
        { 10008, 2, 1  }, // MAGIC_HELMET
        { 10009, 2, 1  }, // MAGIC_CHEST
        { 10010, 2, 1  }, // MAGIC_PANTS
        { 10011, 2, 1  }, // MAGIC_BOOTS
        { 10012, 2, 1  }, // MAGIC_WEAPON
    };
    const ItemMeta* GetItemMeta(uint32 itemID)
    {
        for (size_t i = 0; i < sizeof(g_itemMeta) / sizeof(ItemMeta); ++i)
            if (g_itemMeta[i].itemID == itemID) return &g_itemMeta[i];
        return nullptr;   // 테이블에 없는 itemID → 서버가 만들어낸 적 없는 아이템(버그)
    }

    // 랜덤스탯 한 칸이 정상 범위인지. 서버 룰 테이블(FieldDropItemPool.h g_*RandomStatRules)의
    // "가장 느슨한(매직) 상한"으로 검사한다. 타입은 RANDOM_STAT_TYPE 열거값(1=ATK..6=MP_REGEN).
    // value는 룰상 최소 1 이상(0이면 '스탯 없음'으로 취급되어 애초에 안 실려옴).
    bool RandomStatOk(uint8 type, int16 value)
    {
        if (type < 1 || type > 6) return false;   // ATK(1)~MP_REGEN(6) 밖이면 비정상
        if (value < 1) return false;              // 룰 최소값이 1 이상
        int16 upper = 0;
        switch (type)
        {
        case 1: upper = 4;  break; // ATK     (magic 2~4)
        case 2: upper = 1;  break; // DEF     (1~1)
        case 3: upper = 30; break; // MAX_HP  (magic 20~30)
        case 4: upper = 30; break; // MAX_MP  (magic 20~30)
        case 5: upper = 2;  break; // HP_REGEN(magic 1~2)
        case 6: upper = 2;  break; // MP_REGEN(magic 1~2)
        }
        return value <= upper;
    }
}

// 생성자: 설정을 복사하고 오류 카운터를 0으로 초기화.
LoadTestManager::LoadTestManager(const LoadConfig& cfg)
    : m_cfg(cfg)
{
    for (int i = 0; i < ERR_CAT_COUNT; ++i)
        m_errors[i].store(0);
    for (int i = 0; i < FIELD_TYPE_COUNT; ++i)   // 수신 종류별 카운트 0으로
        m_recvByType[i].store(0);
}

// 소멸자: IOCP 핸들 닫고 윈속 정리.
LoadTestManager::~LoadTestManager()
{
    if (m_iocp) CloseHandle(m_iocp);
    WSACleanup();
}

// 오류 종류 enum → 사람이 읽는 짧은 이름(콘솔/로그 표시용).
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

// 더미가 잡은 오류(=서버 이상 의심)를 카운트 + 파일 로그로 동시에 남긴다.
// 파일: C:\Users\Public\LogFolder\YYYYMM\YYYYMM_LoadTester.txt (ERROR 레벨)
// fmt 이하 가변인자는 와이드 포맷. 어떤 봇(uid/idx)에서 무슨 값으로 터졌는지까지 기록.
void LoadTestManager::ReportError(int cat, const DummyClient& c, const wchar_t* fmt, ...)
{
    m_errors[cat].fetch_add(1);   // 종류별 카운트 증가(여러 스레드가 호출하므로 atomic)

    // 가변인자를 와이드 문자열 한 줄(detail)로 포맷. 넘치면 잘라냄(_TRUNCATE).
    WCHAR detail[256];
    va_list ap;
    va_start(ap, fmt);
    _vsnwprintf_s(detail, _countof(detail), _TRUNCATE, fmt, ap);
    va_end(ap);

    // "오류이름 | uid | idx | 상세" 형태로 파일에 기록(%S=narrow, %llu=uint64, %s=wide).
    LOG(L"LoadTester", dfLOG_LEVEL_ERROR, L"%S | uid=%llu idx=%d | %s",
        ErrName(cat), (unsigned long long)c.characterUID, c.index, detail);
}

// 초기화: 윈속 시작 + 봇 배열 준비 + IOCP 생성 + 워커/행동/통계 스레드 기동.
bool LoadTestManager::Init()
{
    WSADATA wd;
    if (WSAStartup(MAKEWORD(2, 2), &wd) != 0)   // 윈속 2.2 초기화
    {
        printf("[init] WSAStartup failed\n");
        return false;
    }

    srand(GetTickCount());   // 랜덤워크/공격 등에 쓰는 rand 시드

    // 봇 배열 할당. 각 봇에 슬롯번호와 characterUID(1..N) 부여.
    m_clients.reset(new DummyClient[m_cfg.userCount]);
    for (int i = 0; i < m_cfg.userCount; ++i)
    {
        m_clients[i].index = i;
        m_clients[i].characterUID = static_cast<uint64>(i + 1); // characterUID 1..N
    }

    // 모든 소켓이 묶일 IOCP 생성.
    m_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (m_iocp == NULL)
    {
        printf("[init] CreateIoCompletionPort failed\n");
        return false;
    }

    m_running.store(true);

    // 워커 스레드 수 = 하드웨어 스레드 수(못 구하면 4개).
    unsigned hw = std::thread::hardware_concurrency();
    unsigned workerCount = hw ? hw : 4;
    for (unsigned i = 0; i < workerCount; ++i)
        m_workers.emplace_back(&LoadTestManager::WorkerLoop, this);  // 수신 처리

    m_behavior = std::thread(&LoadTestManager::BehaviorLoop, this);  // 주기적 송신
    m_stats = std::thread(&LoadTestManager::StatsLoop, this);        // 1초 통계

    printf("[init] ok. workers=%u, target=%s:%u, users=%d\n",
        workerCount, m_cfg.ip.c_str(), m_cfg.port, m_cfg.userCount);
    return true;
}

// 테스트 본체: 램프 접속 → 시뮬레이션 루프(RST 웨이브/재접속) → 정리 → 리포트.
void LoadTestManager::Run()
{
    DWORD startTick = GetTickCount();

    // ---- 램프업: 서버 accept()는 단일 직렬 루프라 한꺼번에 수천 개를 열면
    // 서버 상태와 무관하게 적체된다. 그래서 초당 개수를 나눠서 천천히 연다.
    printf("[run] connecting %d users at %d/sec ...\n", m_cfg.userCount, m_cfg.rampPerSec);
    for (int i = 0; i < m_cfg.userCount; ++i)
    {
        ConnectOne(m_clients[i]);
        if (m_cfg.rampPerSec > 0 && ((i + 1) % m_cfg.rampPerSec) == 0)
            Sleep(1000);   // rampPerSec개 열 때마다 1초 쉬기
    }
    printf("[run] ramp done. simulating for %d sec ...\n", m_cfg.durationSec);

    bool churnEnabled = (m_cfg.rstPercent > 0);   // RST 웨이브를 쓸지
    bool singleWaveDone = false;                  // 1회성 웨이브를 이미 했는지
    DWORD nextChurnTick = startTick + (DWORD)m_cfg.rstAfterSec * 1000; // 첫 웨이브 시각

    // 재접속도 초기 램프처럼 분산시켜, 큰 웨이브가 블로킹 connect() 폭주로
    // 이 루프를 멈춰 세우지 않게 한다.
    int reconnPerPass = (m_cfg.rampPerSec > 0) ? (m_cfg.rampPerSec / 10 + 1) : 100000;

    while (m_running.load())
    {
        DWORD now = GetTickCount();
        DWORD el = now - startTick;   // 경과 시간

        // --- churn: 현재 필드에 있는 봇의 일정 %를 RST로 끊기 ---
        if (churnEnabled && !singleWaveDone && (int)(now - nextChurnTick) >= 0)
        {
            int target = (m_inField.load() * m_cfg.rstPercent) / 100; // 끊을 목표 수
            int done = 0;
            for (int i = 0; i < m_cfg.userCount && done < target; ++i)
            {
                DummyClient& c = m_clients[i];
                if (!c.closed.load() && c.state == ConnState::InField)
                {
                    RequestRST(c);   // 쿨다운>0이면 재접속도 함께 예약
                    ++done;
                }
            }
            printf("[run] RST wave: %d connections (reconnect=%s)\n",
                done, m_cfg.reconnectCooldownSec > 0 ? "yes" : "no");

            if (m_cfg.churnIntervalSec > 0)
                nextChurnTick = now + (DWORD)m_cfg.churnIntervalSec * 1000; // 반복
            else
                singleWaveDone = true;                                      // 1회만
        }

        // --- 쿨다운이 지난 RST된 봇 재접속 (분산 처리) ---
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

        if (el >= (DWORD)m_cfg.durationSec * 1000)   // 정해진 시간 다 됨 → 종료
            break;

        Sleep(100);   // 이 컨트롤 루프는 100ms 간격으로 충분
    }

    // ---- 종료 처리 ----------------------------------------------------------
    double elapsed = (GetTickCount() - startTick) / 1000.0;
    m_shuttingDown.store(true);   // 이제부터의 끊김은 서버 오류로 안 셈
    m_running.store(false);

    // 종료 끊기도 한꺼번에 하면 FIN/RST이 폭주해 일부 유실(서버 half-open/TIME_WAIT 적체) 우려 →
    // 접속 램프와 같은 속도(rampPerSec)로 나눠 끊는다.
    for (int i = 0; i < m_cfg.userCount; ++i)
    {
        HandleDisconnect(m_clients[i], false); // 정상 종료(오류 카운트 안 함)
        if (m_cfg.rampPerSec > 0 && ((i + 1) % m_cfg.rampPerSec) == 0)
            Sleep(1000);   // rampPerSec개 끊을 때마다 1초 쉬기
    }

    // GQCS에 막혀있는 워커들을 깨우기(0바이트/키0 완료를 워커 수만큼 던짐)
    for (size_t i = 0; i < m_workers.size(); ++i)
        PostQueuedCompletionStatus(m_iocp, 0, 0, NULL);

    for (auto& t : m_workers) if (t.joinable()) t.join();
    if (m_behavior.joinable()) m_behavior.join();
    if (m_stats.joinable())    m_stats.join();

    PrintReport(elapsed);   // 최종 리포트
}

// ---------------------------------------------------------------------------
// 접속 수명주기
// ---------------------------------------------------------------------------
// 소켓 하나를 열어 서버에 connect 후 IOCP에 묶고, 수신 1건을 건 뒤 로그인+선택을 보낸다.
void LoadTestManager::ConnectOne(DummyClient& c)
{
    // 오버랩 가능한 TCP 소켓 생성.
    c.sock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (c.sock == INVALID_SOCKET)
        return;

    // 접속 주소 구성.
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_cfg.port);
    inet_pton(AF_INET, m_cfg.ip.c_str(), &addr.sin_addr);

    // 블로킹 connect(램프로 빈도를 조절하므로 블로킹이어도 OK).
    if (connect(c.sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        closesocket(c.sock);
        c.sock = INVALID_SOCKET;
        return;
    }

    // 소켓을 IOCP에 등록. completion key로 이 봇의 포인터(&c)를 넣는다(그래서 배열 주소가 고정돼야 함).
    if (CreateIoCompletionPort((HANDLE)c.sock, m_iocp, (ULONG_PTR)&c, 0) == NULL)
    {
        closesocket(c.sock);
        c.sock = INVALID_SOCKET;
        return;
    }

    c.connectTick = GetTickCount();
    c.state = ConnState::LoggedIn;     // 곧바로 로그인+선택을 보내므로 이 상태로
    m_connected.fetch_add(1);
    m_connectOk.fetch_add(1);          // connect() 성공 누적

    PostRecv(c);                       // 스폰 스트림을 받기 위해 수신 먼저 걸기
    SendLoginAndSelect(c);             // 로그인 + 캐릭터 선택 송신
}

// 비동기 수신 1건을 건다(남은 버퍼 뒤쪽에 이어받도록). 실패면 끊기 처리.
void LoadTestManager::PostRecv(DummyClient& c)
{
    if (c.closed.load()) return;

    memset(&c.recvOv, 0, sizeof(c.recvOv));        // 오버랩 구조체 초기화
    c.wsaRecv.buf = c.recvBuf + c.recvUsed;        // 이미 쌓인 데이터 뒤에 이어받기
    c.wsaRecv.len = (ULONG)(sizeof(c.recvBuf) - c.recvUsed);

    DWORD flags = 0, bytes = 0;
    int r = WSARecv(c.sock, &c.wsaRecv, 1, &bytes, &flags, &c.recvOv, NULL);
    if (r == SOCKET_ERROR)
    {
        int e = WSAGetLastError();
        if (e != WSA_IO_PENDING)       // PENDING은 정상(비동기 진행 중), 그 외는 진짜 오류
            HandleDisconnect(c, true);
    }
}

// 끊기 정리. closed 플래그로 어느 스레드가 먼저 와도 정리는 딱 한 번만.
void LoadTestManager::HandleDisconnect(DummyClient& c, bool serverInitiated)
{
    // 단일 정리 가드: 먼저 도착한 스레드만 실제 정리 수행.
    if (c.closed.exchange(true))
        return;

    // 우리가 안 끊었는데(서버발) + 종료중도 아니고 + RST 요청도 아니면 → 서버 이상.
    if (serverInitiated && !m_shuttingDown.load() && !c.rstRequested)
        ReportError(ERR_UNEXPECTED_CLOSE, c, L"server closed connection (state=%d)", (int)c.state);

    if (c.state == ConnState::InField)   // 필드에 있던 봇이면 inField 카운트 감소
        m_inField.fetch_sub(1);

    if (c.sock != INVALID_SOCKET)
    {
        closesocket(c.sock);
        c.sock = INVALID_SOCKET;
    }
    m_connected.fetch_sub(1);
    c.state = ConnState::Dead;
}

// connect는 성공했는데 서버가 리셋한 경우를 "스폰 전(세션 안 됨)" vs "세션 중"으로 나눠 센다.
// 우리가 일부러 끊은 것(RST 웨이브)·종료 중·OPERATION_ABORTED(995=우리 closesocket)는 제외.
void LoadTestManager::CountConnReset(const DummyClient& c, int err)
{
    if (c.rstRequested || m_shuttingDown.load())
        return;
    if (err != WSAECONNRESET && err != WSAECONNABORTED && err != WSAETIMEDOUT)
        return;   // 다른 코드(예: 995 OPERATION_ABORTED=우리 teardown)는 세지 않음

    if (c.state == ConnState::InField)
        m_resetInField.fetch_add(1);    // 쓰던 세션이 끊김 = 진짜 mid-session 드롭
    else
        m_resetPreField.fetch_add(1);   // 스폰 전 = 안 받아들여짐/초기 거부(백로그 풀 의심)
}

// 봇을 RST(비정상 끊기)로 끊는다. SO_LINGER 0 → close가 FIN 대신 RST를 보냄.
void LoadTestManager::RequestRST(DummyClient& c)
{
    if (c.closed.load()) return;
    c.rstRequested = true;   // 이 끊김은 "우리가 일부러" → 서버 오류로 안 셈

    // linger 0 설정 → closesocket이 우아한 FIN이 아니라 RST를 보낸다.
    linger lg;
    lg.l_onoff = 1;
    lg.l_linger = 0;
    setsockopt(c.sock, SOL_SOCKET, SO_LINGER, (char*)&lg, sizeof(lg));

    HandleDisconnect(c, false); // 우리가 시작한 끊김 → 서버 오류 아님

    // 복귀(재접속) 예약. 쿨다운은 서버가 이전 세션을 완전히 해제할 시간을 줘
    // 같은 UID 중복로그인 겹침을 피한다.
    if (m_cfg.reconnectCooldownSec > 0)
        c.reconnectAtTick = GetTickCount() + (DWORD)m_cfg.reconnectCooldownSec * 1000;
}

// 죽은 슬롯을 새 접속으로 재사용. 세션 상태는 지우되 origin anchor/UID/index는 유지.
void LoadTestManager::ResetForReconnect(DummyClient& c)
{
    // 세션별 상태만 초기화하고 origin anchor / characterUID / index는 보존한다.
    // 그래야 봇이 로그아웃-저장 위치로 매번 드리프트하지 않고 같은 몬스터 섹터를 계속 돈다.
    c.closed.store(false);
    c.rstRequested = false;
    c.recvUsed = 0;
    c.spawnReceived = false;
    c.spawnTimeoutCounted = false;
    c.reconnectAtTick = 0;
    c.knownOthers.clear();
    c.knownMonsters.clear();
    c.knownDropItems.clear();
    c.pickupTargetUID.store(0);   // 줍기 타깃도 리셋(이전 세션 드롭은 무효)
    c.lastExp = 0;                // 경험치 역행 검사 기준 초기화
    c.lastLevel = 1;
    c.entityId = 0;
    c.deadInGame.store(false);
    c.respawnReqSent = false;
    c.respawnReqTick = 0;
    c.state = ConnState::Disconnected;

    m_reconnects.fetch_add(1);
    ConnectOne(c);   // 새 소켓 + 로그인 + 선택
}

// ---------------------------------------------------------------------------
// IOCP 워커: 수신 완료 처리
// ---------------------------------------------------------------------------
void LoadTestManager::WorkerLoop()
{
    for (;;)
    {
        DWORD bytes = 0;
        ULONG_PTR key = 0;
        OVERLAPPED* ov = nullptr;

        // 완료 1건 대기(타임아웃 1초). key=0이면 종료 신호 또는 타임아웃.
        BOOL ok = GetQueuedCompletionStatus(m_iocp, &bytes, &key, &ov, 1000);
        DWORD gle = ok ? 0 : GetLastError();   // 실패면 즉시 에러코드 캡처(뒤 호출이 last-error 덮기 전에)

        if (key == 0) // 종료 sentinel 또는 1초 타임아웃
        {
            if (m_shuttingDown.load()) break;   // 종료 중이면 워커 탈출
            continue;                           // 그냥 타임아웃이면 계속
        }

        DummyClient* c = reinterpret_cast<DummyClient*>(key); // key = 봇 포인터

        if (!ok)                     // 실패한 완료(서버 리셋 / 우리 teardown)
        {
            CountConnReset(*c, (int)gle);   // 서버발 리셋이면 스폰 전/세션 중으로 집계(우리발은 내부에서 제외)
            HandleDisconnect(*c, true);
            continue;
        }
        if (bytes == 0)              // 상대가 우아하게 닫음(FIN)
        {
            HandleDisconnect(*c, true);
            continue;
        }

        m_recvBytes.fetch_add(bytes);
        c->recvUsed += bytes;        // 받은 만큼 버퍼 사용량 증가
        ParseFrames(*c);             // 완성된 프레임들 처리

        if (!c->closed.load())
            PostRecv(*c);            // 다음 수신 다시 걸기
    }
}

// ---------------------------------------------------------------------------
// 프레이밍 + 검증
// ---------------------------------------------------------------------------
// 수신 버퍼에서 완성된 프레임을 하나씩 떼어 HandlePacket으로 넘긴다.
void LoadTestManager::ParseFrames(DummyClient& c)
{
    for (;;)
    {
        if (c.recvUsed < (int)sizeof(GAMELIB_LANHEADER))   // 헤더도 다 안 옴 → 대기
            break;

        GAMELIB_LANHEADER h;
        memcpy(&h, c.recvBuf, sizeof(h));   // 앞 5바이트 = 헤더

        // 버퍼에 들어갈 수 없는 길이 → 손상/디싱크 스트림으로 판단.
        if (h.s_len > (int)sizeof(c.recvBuf) - (int)sizeof(h))
        {
            ReportError(ERR_LEN_MISMATCH, c, L"corrupt frame len=%d (buffer overflow)", (int)h.s_len);
            HandleDisconnect(c, false);
            return;
        }

        int full = (int)sizeof(h) + h.s_len;   // 이 프레임의 전체 크기(헤더+페이로드)
        if (c.recvUsed < full)
            break; // 이 프레임의 나머지가 아직 안 옴 → 대기

        PacketReader r(c.recvBuf + sizeof(h), h.s_len);  // 페이로드만 읽는 리더
        uint16 type = r.GetU16();                        // 페이로드 첫 2바이트 = 타입
        m_recvPackets.fetch_add(1);
        if (type >= FIELD_TYPE_BASE && type < FIELD_TYPE_BASE + FIELD_TYPE_COUNT)
            m_recvByType[type - FIELD_TYPE_BASE].fetch_add(1);   // 확산 전송 분포 집계
        HandlePacket(c, type, r);                        // 타입별 검증/반응

        // 처리한 프레임을 버퍼 앞으로 당겨 제거.
        memmove(c.recvBuf, c.recvBuf + full, c.recvUsed - full);
        c.recvUsed -= full;
    }
}

// 받은 패킷 타입별 검증/반응. 깊게 파싱하는 건 죽음/부활/정합성에 관계된 것 위주.
void LoadTestManager::HandlePacket(DummyClient& c, uint16 type, PacketReader& r)
{
    using namespace FieldProtocol;

    switch (type)
    {
    case PACKET_SC_CREATE_MY_CHARACTER:   // 1000: 내 캐릭터 스폰(접속 후 첫 신호)
    {
        // 고정부: id, x,y,z, yaw, HP, MP, level, exp. (그 뒤 인벤/장비/퀵슬롯은 가변)
        uint64 id = r.GetU64();
        float x = r.GetFloat(), y = r.GetFloat(), z = r.GetFloat(), yaw = r.GetFloat();
        int16 hp = r.GetI16(), mp = r.GetI16();
        uint16 lv = r.GetU16(); int32 exp = r.GetI32();
        c.entityId = id;   // 내 서버 id 저장(히트결과 self-death / 부활 식별용)
        (void)yaw; (void)mp;   // 지금 행동에 안 쓰는 값들
        if (!r.Ok()) { ReportError(ERR_LEN_MISMATCH, c, L"CREATE_MY truncated"); break; }
        if (!CoordOk(x, y) || hp < 0)
            ReportError(ERR_BAD_VALUE, c, L"CREATE_MY x=%.0f y=%.0f hp=%d", x, y, (int)hp);
        if (lv < 1 || lv > USER_MAX_LEVEL)
            ReportError(ERR_BAD_VALUE, c, L"CREATE_MY level=%d", (int)lv);
        c.lastExp = exp;   // 이후 GAIN_EXP 역행/범위 검사 기준
        c.lastLevel = lv;
        // 가변 꼬리(인벤/장비/퀵슬롯)를 끝까지 파싱하며 검증: itemID 존재, count<=maxStack, 슬롯/랜덤스탯 정상.
        ValidateInventoryTail(c, r);
        // 현재 위치는 항상 서버 스폰 지점을 따라간다...
        c.x = x; c.y = y; c.z = z;
        // ...하지만 랜덤워크 anchor는 "첫 스폰"에만 고정하고 재접속해도 유지한다.
        // 그래야 로그아웃-위치 드리프트가 누적되지 않고, 계속 끊겨도 봇이 자기 몬스터 섹터에 머문다.
        if (!c.originSet)
        {
            c.originSet = true;
            c.anchorX = x;
            c.anchorY = y;
        }
        if (!c.spawnReceived)   // 이번 세션 첫 스폰이면 필드 진입 + 행동 타이머 시작
        {
            c.spawnReceived = true;
            c.headingRad = (float)(rand() % 628) / 100.f; // 0..2pi 랜덤 초기 방향
            c.state = ConnState::InField;
            m_inField.fetch_add(1);
            uint32 now = GetTickCount();
            // 첫 행동 시각을 봇마다 랜덤 위상으로 흩어 부하가 한 틱에 몰리지 않게 함.
            c.nextMoveTick = now + (m_cfg.moveIntervalMs ? rand() % m_cfg.moveIntervalMs : 0);
            c.nextAttackTick = now + (m_cfg.attackIntervalMs ? rand() % m_cfg.attackIntervalMs : 0);
            c.nextRttTick = now + (m_cfg.rttIntervalMs ? rand() % m_cfg.rttIntervalMs : 0);
            // 컨텐츠 행동 타이머도 같은 방식으로 랜덤 위상 시작.
            c.nextSkillTick = now + (m_cfg.skillIntervalMs ? rand() % m_cfg.skillIntervalMs : 0);
            c.nextPickupTick = now + (m_cfg.pickupIntervalMs ? rand() % m_cfg.pickupIntervalMs : 0);
            c.nextUseItemTick = now + (m_cfg.useItemIntervalMs ? rand() % m_cfg.useItemIntervalMs : 0);
            c.nextSwapTick = now + (m_cfg.swapIntervalMs ? rand() % m_cfg.swapIntervalMs : 0);
            c.nextDeleteTick = now + (m_cfg.deleteItemIntervalMs ? rand() % m_cfg.deleteItemIntervalMs : 0);
        }
        break;
    }
    case PACKET_SC_CREATE_OTHER_CHARACTER:   // 1001: 타 캐릭터 스폰(주변에 누가 보임)
    {
        // 현재 서버 구조: id, x,y,z, yaw, moveFlag(u8). HP/MaxHP 없음
        // (피격 시에만 1015/1023/1041로 HP·ratio를 별도 송신하도록 바뀜)
        uint64 id = r.GetU64();
        float x = r.GetFloat(), y = r.GetFloat(), z = r.GetFloat(), yaw = r.GetFloat();
        uint8 mf = r.GetU8();
        (void)z; (void)yaw; (void)mf;
        if (!r.Ok() || r.Remain() != 0) { ReportError(ERR_LEN_MISMATCH, c, L"CREATE_OTHER size (remain=%d)", r.Remain()); break; }
        if (!CoordOk(x, y))
            ReportError(ERR_BAD_VALUE, c, L"CREATE_OTHER id=%llu x=%.0f y=%.0f", (unsigned long long)id, x, y);
        if (!c.knownOthers.insert(id).second)   // 이미 아는 id를 또 create → 중복(AOI 재진입이면 정상이라 heuristic)
            ReportError(ERR_ID_CONSISTENCY, c, L"CREATE_OTHER dup id=%llu", (unsigned long long)id); // duplicate create
        break;
    }
    case PACKET_SC_DELETE_CHARACTER:   // 1002: 타 캐릭터 사라짐(시야 이탈/로그아웃)
    {
        uint64 id = r.GetU64();
        if (!r.Ok() || r.Remain() != 0) { ReportError(ERR_LEN_MISMATCH, c, L"DELETE_CHARACTER size"); break; }
        if (c.knownOthers.erase(id) == 0)   // 모르는 id를 delete → 정합성 의심
            ReportError(ERR_ID_CONSISTENCY, c, L"DELETE unknown id=%llu", (unsigned long long)id); // delete of unknown id
        break;
    }
    case PACKET_SC_RTT_ECHO:   // 1010: RTT 에코
    {
        // 서버는 클라가 보낸 시각을 버리고 자기 시각(클라 snapshot용)을 에코하므로,
        // RTT는 에코값이 아니라 로컬에 저장해 둔 송신 시각(rttSentMs)으로 측정한다.
        uint64 t = r.GetU64();
        (void)t;
        if (!r.Ok() || r.Remain() != 0) { ReportError(ERR_LEN_MISMATCH, c, L"RTT_ECHO size"); break; }
        uint64 sent = c.rttSentMs.load();
        if (sent != 0)   // 대기 중인 핑이 있을 때만 측정
        {
            double lat = (double)(GetTickCount64() - sent);   // 왕복 지연(ms)
            std::lock_guard<std::mutex> lk(m_rttMtx);
            m_rttSamples.push_back(lat);
        }
        break;
    }
    case PACKET_SC_ATTACK_HIT_RESULT:   // 1015: 공격 피격 결과(플레이어/몬스터)
    {
        // player: { u64 id, i16 HP, u8 ratio }  /  monster: { u64 id, i16 HP }
        uint8 pc = r.GetU8(), mc = r.GetU8();   // 피격된 플레이어 수, 몬스터 수
        for (int i = 0; i < pc; ++i)
        {
            uint64 id    = r.GetU64();
            int16  nh    = r.GetI16();
            uint8  ratio = r.GetU8();   // 본인=절대HP가 옴, ratio는 타인 표시용
            (void)ratio;
            if (r.Ok() && nh < 0) ReportError(ERR_BAD_VALUE, c, L"HIT_RESULT player id=%llu hp=%d", (unsigned long long)id, (int)nh);
            // 내가 맞아 HP<=0 → 죽음. 실제 부활 요청은 behavior 스레드가 보냄.
            if (r.Ok() && id == c.entityId && nh <= 0 && !c.deadInGame.load())
            {
                c.deadInGame.store(true);
                m_gameDeaths.fetch_add(1);
            }
        }
        for (int i = 0; i < mc; ++i) { uint64 mid = r.GetU64(); int16 nh = r.GetI16(); if (r.Ok() && nh < 0) ReportError(ERR_BAD_VALUE, c, L"HIT_RESULT monster id=%llu hp=%d", (unsigned long long)mid, (int)nh); }
        if (!r.Ok() || r.Remain() != 0) ReportError(ERR_LEN_MISMATCH, c, L"HIT_RESULT size");
        break;
    }
    case PACKET_SC_HIT_TOPLAYER:   // 1023: 몬스터가 나(플레이어)를 때림
    {
        // 몬스터가 플레이어를 때림: { u64 monsterId, u64 targetId, i16 newHP }.
        // 내가 맞아 HP<=0 이면 죽음(몬스터 사망도 부활 대상). 부활 요청은 behavior 스레드가 보냄.
        uint64 mid = r.GetU64();
        uint64 tid = r.GetU64();
        int16  nh  = r.GetI16();
        if (!r.Ok() || r.Remain() != 0) { ReportError(ERR_LEN_MISMATCH, c, L"HIT_TOPLAYER size"); break; }
        // 때린 몬스터와 맞은 대상이 둘 다 0 = 누가 누구를 때렸는지 비어있는 깨진 패킷
        if (mid == 0 && tid == 0)
            ReportError(ERR_BAD_VALUE, c, L"HIT_TOPLAYER both ids zero");
        if (nh < 0) ReportError(ERR_BAD_VALUE, c, L"HIT_TOPLAYER target=%llu hp=%d", (unsigned long long)tid, (int)nh);
        if (tid == c.entityId && nh <= 0 && !c.deadInGame.load())   // 대상이 나 + HP<=0 → 죽음
        {
            c.deadInGame.store(true);
            m_gameDeaths.fetch_add(1);
        }
        break;
    }
    case PACKET_SC_RESPAWN_RES_TO_ME:   // 1039: 내 부활 응답
    {
        // 제자리 부활: { i16 HP, i16 MP } (위치 없음)
        int16 hp = r.GetI16(), mp = r.GetI16();
        (void)mp;
        if (!r.Ok() || r.Remain() != 0) { ReportError(ERR_LEN_MISMATCH, c, L"RESPAWN_TO_ME size"); break; }
        if (hp <= 0) ReportError(ERR_BAD_VALUE, c, L"RESPAWN_TO_ME hp=%d", (int)hp);   // 부활인데 HP<=0이면 서버 버그
        c.deadInGame.store(false);   // 부활 완료 → behavior 스레드가 행동 재개
        m_respawns.fetch_add(1);
        break;
    }
    case PACKET_SC_RESPAWN_RES_TO_OTHER:   // 1040: 주변 누군가 부활했다는 통지
    {
        uint64 id = r.GetU64();   // 누가 부활했는지(주변 통지). 죽어도 despawn 안 하므로 알던 id.
        (void)id;
        if (!r.Ok() || r.Remain() != 0) ReportError(ERR_LEN_MISMATCH, c, L"RESPAWN_TO_OTHER size");
        break;
    }
    case PACKET_SC_CREATE_MONSTER:   // 1019: 몬스터 스폰(시야 진입/부활)
    {
        // { u64 id, f x,y,z, f yaw, u16 type, i16 HP, i16 MaxHP }
        uint64 id = r.GetU64();
        float x = r.GetFloat(), y = r.GetFloat(), z = r.GetFloat(), yaw = r.GetFloat();
        uint16 mtype = r.GetU16(); int16 hp = r.GetI16(), maxhp = r.GetI16();
        (void)z; (void)yaw; (void)mtype;
        if (!r.Ok() || r.Remain() != 0) { ReportError(ERR_LEN_MISMATCH, c, L"CREATE_MONSTER size (remain=%d)", r.Remain()); break; }
        if (!CoordOk(x, y))
            ReportError(ERR_BAD_VALUE, c, L"CREATE_MONSTER id=%llu x=%.0f y=%.0f", (unsigned long long)id, x, y);
        if (hp <= 0 || maxhp <= 0 || hp > maxhp)   // 살아 스폰되는데 HP 이상치 → 서버 버그
            ReportError(ERR_BAD_VALUE, c, L"CREATE_MONSTER id=%llu hp=%d/%d", (unsigned long long)id, (int)hp, (int)maxhp);
        if (!c.knownMonsters.insert(id).second)   // 이미 아는 몬스터를 또 create → 중복/누락
            ReportError(ERR_ID_CONSISTENCY, c, L"CREATE_MONSTER dup id=%llu", (unsigned long long)id);
        break;
    }
    case PACKET_SC_DELETE_MONSTER:   // 1020: 몬스터 사라짐(시야 이탈/죽음)
    {
        uint64 id = r.GetU64();
        if (!r.Ok() || r.Remain() != 0) { ReportError(ERR_LEN_MISMATCH, c, L"DELETE_MONSTER size"); break; }
        if (c.knownMonsters.erase(id) == 0)   // 모르는 몬스터를 delete → 정합성 의심
            ReportError(ERR_ID_CONSISTENCY, c, L"DELETE_MONSTER unknown id=%llu", (unsigned long long)id);
        break;
    }
    case PACKET_SC_MOVE_MONSTER:   // 1021: 몬스터 이동(몬스터 around 에만 뿌림 → create 범위와 대칭)
    {
        // { u64 id, f x,y,z, f speed, f desX, f desY }
        uint64 id = r.GetU64();
        float x = r.GetFloat(), y = r.GetFloat(), z = r.GetFloat();
        float speed = r.GetFloat(), dx = r.GetFloat(), dy = r.GetFloat();
        (void)z; (void)speed; (void)dx; (void)dy;
        if (!r.Ok() || r.Remain() != 0) { ReportError(ERR_LEN_MISMATCH, c, L"MOVE_MONSTER size (remain=%d)", r.Remain()); break; }
        if (!CoordOk(x, y))
            ReportError(ERR_BAD_VALUE, c, L"MOVE_MONSTER id=%llu x=%.0f y=%.0f", (unsigned long long)id, x, y);
        if (c.knownMonsters.find(id) == c.knownMonsters.end())   // create 없이 move → 핸드오프/순서 버그
            ReportError(ERR_ID_CONSISTENCY, c, L"MOVE_MONSTER unknown id=%llu", (unsigned long long)id);
        break;
    }
    case PACKET_SC_STOP_MONSTER:   // 1022: 몬스터 정지(몬스터 around 에만 뿌림 → create 범위와 대칭)
    {
        // { u64 id, f x,y,z }
        uint64 id = r.GetU64();
        float x = r.GetFloat(), y = r.GetFloat(), z = r.GetFloat();
        (void)z;
        if (!r.Ok() || r.Remain() != 0) { ReportError(ERR_LEN_MISMATCH, c, L"STOP_MONSTER size (remain=%d)", r.Remain()); break; }
        if (!CoordOk(x, y))
            ReportError(ERR_BAD_VALUE, c, L"STOP_MONSTER id=%llu x=%.0f y=%.0f", (unsigned long long)id, x, y);
        if (c.knownMonsters.find(id) == c.knownMonsters.end())   // create 없이 stop → 핸드오프/순서 버그
            ReportError(ERR_ID_CONSISTENCY, c, L"STOP_MONSTER unknown id=%llu", (unsigned long long)id);
        break;
    }

    // ---- 타 캐릭터 이동/연출 SC: id가 내가 아는 캐릭터(시야 안)인지 + 좌표 정상인지 ----
    case PACKET_SC_UPDATE_CHARACTER_MOVEMENT_INPUT:   // 1004: 타캐릭 이동 입력 브로드캐스트
    {
        // { u64 id, u64 serverTs, f x,y,z, f yaw, u8 moveFlag }
        uint64 id = r.GetU64(); uint64 ts = r.GetU64();
        float x = r.GetFloat(), y = r.GetFloat(), z = r.GetFloat(), yaw = r.GetFloat();
        uint8 mf = r.GetU8();
        (void)ts; (void)z; (void)yaw; (void)mf;
        if (!r.Ok() || r.Remain() != 0) { ReportError(ERR_LEN_MISMATCH, c, L"MOVE_OTHER size (remain=%d)", r.Remain()); break; }
        if (!CoordOk(x, y))
            ReportError(ERR_BAD_VALUE, c, L"MOVE_OTHER id=%llu x=%.0f y=%.0f", (unsigned long long)id, x, y);
        if (id != c.entityId && c.knownOthers.find(id) == c.knownOthers.end())   // create 없이 이동 → 정합성 의심
            ReportError(ERR_ID_CONSISTENCY, c, L"MOVE_OTHER unknown id=%llu", (unsigned long long)id);
        break;
    }
    case PACKET_SC_SWING_LEFT_ATTACK:   // 1013: 타캐릭 swing 공격 브로드캐스트
    {
        // { u64 id, f yaw, u8 swingIdx }
        uint64 id = r.GetU64(); float yaw = r.GetFloat(); uint8 swingIdx = r.GetU8();
        (void)yaw;
        if (!r.Ok() || r.Remain() != 0) { ReportError(ERR_LEN_MISMATCH, c, L"SWING_OTHER size"); break; }
        if (swingIdx < 1 || swingIdx > 4)   // 서버가 1~4만 허용하므로 브로드캐스트도 그 범위여야 함
            ReportError(ERR_BAD_VALUE, c, L"SWING_OTHER id=%llu swing=%u", (unsigned long long)id, (unsigned)swingIdx);
        if (id != c.entityId && c.knownOthers.find(id) == c.knownOthers.end())
            ReportError(ERR_ID_CONSISTENCY, c, L"SWING_OTHER unknown id=%llu", (unsigned long long)id);
        break;
    }

    // ---- 스킬 ----
    case PACKET_SC_USE_SKILL_RES:   // 1017: 내 스킬 사용 결과(나에게만)
    {
        // { u8 skillSlot, u8 success(1/0) }
        uint8 slot = r.GetU8(), success = r.GetU8();
        if (!r.Ok() || r.Remain() != 0) { ReportError(ERR_LEN_MISMATCH, c, L"SKILL_RES size"); break; }
        if (slot >= SKILL_SLOT_COUNT || success > 1)
            ReportError(ERR_BAD_VALUE, c, L"SKILL_RES slot=%u success=%u", (unsigned)slot, (unsigned)success);
        if (success) m_skillOk.fetch_add(1); else m_skillFail.fetch_add(1);   // 쿨타임/마나로 실패는 정상
        break;
    }
    case PACKET_SC_USE_SKILL_BROADCAST:   // 1018: 타캐릭 스킬 사용 브로드캐스트
    {
        // { u64 id, u8 skillSlot }
        uint64 id = r.GetU64(); uint8 slot = r.GetU8();
        if (!r.Ok() || r.Remain() != 0) { ReportError(ERR_LEN_MISMATCH, c, L"SKILL_BROAD size"); break; }
        if (slot >= SKILL_SLOT_COUNT)
            ReportError(ERR_BAD_VALUE, c, L"SKILL_BROAD id=%llu slot=%u", (unsigned long long)id, (unsigned)slot);
        if (id != c.entityId && c.knownOthers.find(id) == c.knownOthers.end())
            ReportError(ERR_ID_CONSISTENCY, c, L"SKILL_BROAD unknown id=%llu", (unsigned long long)id);
        break;
    }

    // ---- 필드 드롭 아이템 (몬스터처럼 시야 진입/이탈 정합성 추적 + 값 검증) ----
    case PACKET_SC_CREATE_FIELD_DROP_ITEM:   // 1024: 드롭 생성
    {
        // { u64 dropID, u32 itemID, f x,y,z, u16 count }
        uint64 dropID = r.GetU64(); uint32 itemID = r.GetU32();
        float x = r.GetFloat(), y = r.GetFloat(), z = r.GetFloat(); uint16 count = r.GetU16();
        (void)z;
        if (!r.Ok() || r.Remain() != 0) { ReportError(ERR_LEN_MISMATCH, c, L"DROP_CREATE size (remain=%d)", r.Remain()); break; }
        if (!CoordOk(x, y))
            ReportError(ERR_BAD_VALUE, c, L"DROP_CREATE drop=%llu x=%.0f y=%.0f", (unsigned long long)dropID, x, y);
        const ItemMeta* meta = GetItemMeta(itemID);
        if (meta == nullptr)   // 테이블에 없는 itemID
            ReportError(ERR_BAD_VALUE, c, L"DROP_CREATE drop=%llu unknown itemID=%u", (unsigned long long)dropID, itemID);
        else if (count < 1 || count > meta->maxStack)   // maxStack 초과 = 서버 버그(복사류)
            ReportError(ERR_BAD_VALUE, c, L"DROP_CREATE drop=%llu itemID=%u count=%u>max=%u", (unsigned long long)dropID, itemID, (unsigned)count, (unsigned)meta->maxStack);
        if (!c.knownDropItems.insert(dropID).second)   // 같은 dropUID 또 생성 → 중복(복사류 신호)
            ReportError(ERR_ID_CONSISTENCY, c, L"DROP_CREATE dup drop=%llu", (unsigned long long)dropID);
        // 줍기 타깃이 비어 있으면 이 드롭을 타깃으로(behavior가 줍기 시도)
        uint64 expected = 0;
        c.pickupTargetUID.compare_exchange_strong(expected, dropID);
        break;
    }
    case PACKET_SC_DELETE_FIELD_DROP_ITEM:   // 1025: 드롭 사라짐(주워짐/만료/시야 이탈)
    {
        uint64 dropID = r.GetU64();
        if (!r.Ok() || r.Remain() != 0) { ReportError(ERR_LEN_MISMATCH, c, L"DROP_DELETE size"); break; }
        if (c.knownDropItems.erase(dropID) == 0)
            ReportError(ERR_ID_CONSISTENCY, c, L"DROP_DELETE unknown drop=%llu", (unsigned long long)dropID);
        uint64 cur = c.pickupTargetUID.load();   // 줍기 타깃이 사라졌으면 비운다
        if (cur == dropID) c.pickupTargetUID.compare_exchange_strong(cur, 0);
        break;
    }

    // ---- 줍기 성공 응답(나에게만). itemID/개수/슬롯/랜덤스탯 검증 ----
    case PACKET_SC_PICKUP_EQUIPMENT_ITEMS:   // 1030: 장비 줍기 결과
    {
        // { u32 itemID, i16 slotIndex, u16 count, u8 randomStatCount, per:[u8 type, i16 value] }
        uint32 itemID = r.GetU32(); int16 slotIndex = r.GetI16(); uint16 count = r.GetU16(); uint8 rsCount = r.GetU8();
        const ItemMeta* meta = GetItemMeta(itemID);
        if (meta == nullptr || meta->itemType != 2)
            ReportError(ERR_BAD_VALUE, c, L"PICKUP_EQUIP bad itemID=%u", itemID);
        else if (count < 1 || count > meta->maxStack)
            ReportError(ERR_BAD_VALUE, c, L"PICKUP_EQUIP itemID=%u count=%u>max=%u", itemID, (unsigned)count, (unsigned)meta->maxStack);
        if (slotIndex < 0 || slotIndex >= INVENTORY_SLOT_MAX)
            ReportError(ERR_BAD_VALUE, c, L"PICKUP_EQUIP slotIndex=%d", (int)slotIndex);
        if (rsCount > 6)
            ReportError(ERR_BAD_VALUE, c, L"PICKUP_EQUIP randomStatCount=%u", (unsigned)rsCount);
        for (uint8 i = 0; i < rsCount && r.Ok(); ++i)   // 랜덤스탯 각 칸 룰 검사
        {
            uint8 stype = r.GetU8(); int16 sval = r.GetI16();
            if (r.Ok() && !RandomStatOk(stype, sval))
                ReportError(ERR_BAD_VALUE, c, L"PICKUP_EQUIP randomStat type=%u val=%d", (unsigned)stype, (int)sval);
        }
        if (!r.Ok() || r.Remain() != 0) { ReportError(ERR_LEN_MISMATCH, c, L"PICKUP_EQUIP size (remain=%d)", r.Remain()); break; }
        m_pickups.fetch_add(1);
        break;
    }
    case PACKET_SC_PICKUP_CONSUMABLE_ITEMS:   // 1031: 소모품 줍기 결과(스택 갱신)
    {
        // { u32 itemID, i16 slotIndex, u16 newCount }
        uint32 itemID = r.GetU32(); 
        const ItemMeta* meta = GetItemMeta(itemID);
        if (meta == nullptr || meta->itemType != 1)
            ReportError(ERR_BAD_VALUE, c, L"PICKUP_CONSUME bad itemID=%u", itemID);
        int16 slotIndex = r.GetI16(); uint16 newCount = r.GetU16();
        if (r.Ok())
        {
            if (slotIndex < 0 || slotIndex >= INVENTORY_SLOT_MAX)
                ReportError(ERR_BAD_VALUE, c, L"PICKUP_CONSUME slotIndex=%d", (int)slotIndex);
            if (meta && newCount > meta->maxStack)   // 스택이 maxStack 초과 = 복사류 버그
                ReportError(ERR_BAD_VALUE, c, L"PICKUP_CONSUME itemID=%u newCount=%u>max=%u", itemID, (unsigned)newCount, (unsigned)meta->maxStack);
        }

        if (!r.Ok() || r.Remain() != 0) { ReportError(ERR_LEN_MISMATCH, c, L"PICKUP_CONSUME size (remain=%d)", r.Remain()); break; }
        m_pickups.fetch_add(1);
        break;
    }

    // ---- 아이템 사용/장착/해제/삭제/스왑 응답(나에게만). 성공 플래그/슬롯값 위주 검증 ----
    case PACKET_SC_USE_CONSUMABLE_ITEM:   // 1032: 소모품 사용 결과
    {
        // { u8 success, [성공 시 u8 slotType, u16 newCount, i16 slotIndex] }
        uint8 success = r.GetU8();
        if (!r.Ok()) { ReportError(ERR_LEN_MISMATCH, c, L"USE_CONSUME size"); break; }
        if (success > 1) { ReportError(ERR_BAD_VALUE, c, L"USE_CONSUME success=%u", (unsigned)success); break; }
        if (success)
        {
            uint8 slotType = r.GetU8(); uint16 newCount = r.GetU16(); int16 slotIndex = r.GetI16();
            (void)slotIndex;
            if (!r.Ok()) { ReportError(ERR_LEN_MISMATCH, c, L"USE_CONSUME body size"); break; }
            if (slotType != SLOT_INVENTORY && slotType != SLOT_QUICKSLOT)
                ReportError(ERR_BAD_VALUE, c, L"USE_CONSUME slotType=%u", (unsigned)slotType);
            if (newCount > 500)   // 포션 maxStack=500(부하테스트용). 사용 후 개수가 그보다 크면 비정상
                ReportError(ERR_BAD_VALUE, c, L"USE_CONSUME newCount=%u", (unsigned)newCount);
        }
        break;
    }
    case PACKET_SC_EQUIP_ITEM:     // 1033: 장착 결과(가변 꼬리는 형식 의존이라 성공 플래그/언더런만 검사)
    case PACKET_SC_UNEQUIP_ITEM:   // 1034: 해제 결과
    {
        uint8 success = r.GetU8();
        if (!r.Ok()) { ReportError(ERR_LEN_MISMATCH, c, L"EQUIP/UNEQUIP size"); break; }
        if (success > 1)
            ReportError(ERR_BAD_VALUE, c, L"EQUIP/UNEQUIP success=%u type=%u", (unsigned)success, (unsigned)type);
        break;
    }
    case PACKET_SC_DELETE_ITEM:   // 1035: 삭제 결과 { u8 success }
    case PACKET_SC_SWAP_SLOT:     // 1036: 스왑 결과 { u8 success }
    {
        uint8 success = r.GetU8();
        if (!r.Ok() || r.Remain() != 0) { ReportError(ERR_LEN_MISMATCH, c, L"DELETE/SWAP size"); break; }
        if (success > 1)
            ReportError(ERR_BAD_VALUE, c, L"DELETE/SWAP success=%u type=%u", (unsigned)success, (unsigned)type);
        break;
    }

    // ---- 경험치/레벨업(나에게만). exp 음수/역행, 레벨 범위/역행 검사 ----
    case PACKET_SC_GAIN_EXP:   // 1037
    {
        // { u8 levelUp, i32 curExp, [레벨업 시 u16 curLevel, i16 HP, i16 MP] }
        uint8 levelUp = r.GetU8(); int32 curExp = r.GetI32();
        if (!r.Ok()) { ReportError(ERR_LEN_MISMATCH, c, L"GAIN_EXP size"); break; }
        if (curExp < 0)
            ReportError(ERR_BAD_VALUE, c, L"GAIN_EXP curExp=%d", (int)curExp);
        if (!levelUp)
        {
            if (curExp < c.lastExp)   // 레벨업이 아닌데 경험치가 줄면 비정상
                ReportError(ERR_BAD_VALUE, c, L"GAIN_EXP backward %d->%d", (int)c.lastExp, (int)curExp);
            c.lastExp = curExp;
        }
        else
        {
            uint16 curLevel = r.GetU16(); int16 newHP = r.GetI16(), newMP = r.GetI16();
            if (!r.Ok()) { ReportError(ERR_LEN_MISMATCH, c, L"GAIN_EXP levelup body"); break; }
            if (curLevel < 1 || curLevel > USER_MAX_LEVEL || curLevel < c.lastLevel)
                ReportError(ERR_BAD_VALUE, c, L"GAIN_EXP level %u->%u", (unsigned)c.lastLevel, (unsigned)curLevel);
            if (newHP <= 0 || newMP < 0)
                ReportError(ERR_BAD_VALUE, c, L"GAIN_EXP levelup hp=%d mp=%d", (int)newHP, (int)newMP);
            c.lastLevel = curLevel;
            c.lastExp = curExp;
        }
        break;
    }

    // ---- 몬스터가 타 플레이어를 때림(ratio 표시용) ----
    case PACKET_SC_HIT_TO_OTHERPLAYER:   // 1041: { u64 monsterId, u64 targetId, u8 ratio }
    {
        uint64 mid = r.GetU64(); uint64 tid = r.GetU64(); uint8 ratio = r.GetU8();
        (void)mid; (void)tid;
        if (!r.Ok() || r.Remain() != 0) { ReportError(ERR_LEN_MISMATCH, c, L"HIT_OTHER size"); break; }
        if (ratio > 100)   // HP 비율은 0~100
            ReportError(ERR_BAD_VALUE, c, L"HIT_OTHER ratio=%u", (unsigned)ratio);
        break;
    }

    default:
        // 남은 알려진 패킷(내 위치 보정 1005, 무브모드 1008 등)은 깊게 파싱하지 않고 통과.
        // 모르는 타입이면 서버 이상으로 보고.
        if (!IsKnownType(type))
            ReportError(ERR_UNKNOWN_TYPE, c, L"unknown packet type=%u", (unsigned)type);
        break;
    }
}

// CREATE_MY_CHARACTER(1000)의 가변 꼬리(인벤토리/장비/퀵슬롯)를 끝까지 파싱하며 검증한다.
// 목적: 서버가 보낸 초기 인벤이 itemID 테이블에 있는지, count가 maxStack을 넘지 않는지,
//       슬롯 인덱스/랜덤스탯이 정상 범위인지. (worker 스레드에서만 호출 → 락 불필요)
void LoadTestManager::ValidateInventoryTail(DummyClient& c, PacketReader& r)
{
    // ---- 인벤토리: u8 count, per [i16 slotIndex, u32 itemID, u16 count, u8 rsCount, (u8 type,i16 val)*] ----
    uint8 invCount = r.GetU8();
    for (uint8 i = 0; i < invCount && r.Ok(); ++i)
    {
        int16  slotIndex = r.GetI16();
        uint32 itemID    = r.GetU32();
        uint16 count     = r.GetU16();
        uint8  rsCount   = r.GetU8();
        if (!r.Ok()) break;
        if (slotIndex < 0 || slotIndex >= INVENTORY_SLOT_MAX)
            ReportError(ERR_BAD_VALUE, c, L"CREATE_MY inv slotIndex=%d", (int)slotIndex);
        const ItemMeta* meta = GetItemMeta(itemID);
        if (meta == nullptr)
            ReportError(ERR_BAD_VALUE, c, L"CREATE_MY inv unknown itemID=%u", itemID);
        else if (count < 1 || count > meta->maxStack)   // maxStack 초과 검사
            ReportError(ERR_BAD_VALUE, c, L"CREATE_MY inv itemID=%u count=%u>max=%u", itemID, (unsigned)count, (unsigned)meta->maxStack);
        if (rsCount > 6)
            ReportError(ERR_BAD_VALUE, c, L"CREATE_MY inv rsCount=%u", (unsigned)rsCount);
        for (uint8 s = 0; s < rsCount && r.Ok(); ++s)   // 랜덤스탯 각 칸
        {
            uint8 stype = r.GetU8(); int16 sval = r.GetI16();
            if (r.Ok() && !RandomStatOk(stype, sval))
                ReportError(ERR_BAD_VALUE, c, L"CREATE_MY inv randomStat type=%u val=%d", (unsigned)stype, (int)sval);
        }
    }
    // ---- 장비: u8 count, per [u8 equipSlot, u32 itemID, u8 rsCount, (u8 type,i16 val)*] ----
    uint8 eqCount = r.GetU8();
    for (uint8 i = 0; i < eqCount && r.Ok(); ++i)
    {
        uint8  equipSlot = r.GetU8();
        uint32 itemID    = r.GetU32();
        uint8  rsCount   = r.GetU8();
        if (!r.Ok()) break;
        if (equipSlot < 1 || equipSlot > 5)   // EQUIP_SLOT: HELMET(1)..WEAPON(5)
            ReportError(ERR_BAD_VALUE, c, L"CREATE_MY equip slot=%u", (unsigned)equipSlot);
        const ItemMeta* meta = GetItemMeta(itemID);
        if (meta == nullptr || meta->itemType != 2)   // 장비 슬롯엔 장비 아이템만
            ReportError(ERR_BAD_VALUE, c, L"CREATE_MY equip bad itemID=%u", itemID);
        if (rsCount > 6)
            ReportError(ERR_BAD_VALUE, c, L"CREATE_MY equip rsCount=%u", (unsigned)rsCount);
        for (uint8 s = 0; s < rsCount && r.Ok(); ++s)
        {
            uint8 stype = r.GetU8(); int16 sval = r.GetI16();
            if (r.Ok() && !RandomStatOk(stype, sval))
                ReportError(ERR_BAD_VALUE, c, L"CREATE_MY equip randomStat type=%u val=%d", (unsigned)stype, (int)sval);
        }
    }
    // ---- 퀵슬롯: u8 count, per [u8 slotIndex, u32 itemID, u16 count] ----
    uint8 qsCount = r.GetU8();
    for (uint8 i = 0; i < qsCount && r.Ok(); ++i)
    {
        uint8  slotIndex = r.GetU8();
        uint32 itemID    = r.GetU32();
        uint16 count     = r.GetU16();
        if (!r.Ok()) break;
        if (slotIndex >= QUICK_SLOT_MAX)
            ReportError(ERR_BAD_VALUE, c, L"CREATE_MY quick slotIndex=%u", (unsigned)slotIndex);
        const ItemMeta* meta = GetItemMeta(itemID);
        if (meta == nullptr)
            ReportError(ERR_BAD_VALUE, c, L"CREATE_MY quick unknown itemID=%u", itemID);
        else if (count < 1 || count > meta->maxStack)
            ReportError(ERR_BAD_VALUE, c, L"CREATE_MY quick itemID=%u count=%u>max=%u", itemID, (unsigned)count, (unsigned)meta->maxStack);
    }

    // 꼬리까지 다 읽었으면 남는 바이트가 0이어야 한다(스키마 불일치 탐지).
    if (!r.Ok())
        ReportError(ERR_LEN_MISMATCH, c, L"CREATE_MY tail truncated");
    else if (r.Remain() != 0)
        ReportError(ERR_LEN_MISMATCH, c, L"CREATE_MY tail remain=%d", r.Remain());
}

// ---------------------------------------------------------------------------
// 송신
// ---------------------------------------------------------------------------
// 완성된 패킷을 끝까지(부분 송신 대비 루프) 보낸다. 실패하면 끊기 처리.
void LoadTestManager::SendRaw(DummyClient& c, const PacketWriter& w)
{
    if (c.closed.load()) return;

    const char* d = const_cast<PacketWriter&>(w).Data();   // 헤더 채우고 시작 포인터 획득
    int len = w.Length();
    int off = 0;
    while (off < len)   // 부분 송신될 수 있으니 다 보낼 때까지 반복
    {
        int s = send(c.sock, d + off, len - off, 0);
        if (s <= 0)
        {
            CountConnReset(c, WSAGetLastError());   // send 실패 코드로 집계(우리발/종료 중은 내부에서 제외)
            HandleDisconnect(c, false);             // 실패 → 끊기
            return;
        }
        off += s;
    }
    m_sentPackets.fetch_add(1);
    m_sentBytes.fetch_add(len);
}

// 접속 직후 보내는 로그인 + 캐릭터 선택. 실제 클라 M1LoginWidget 흐름 그대로.
void LoadTestManager::SendLoginAndSelect(DummyClient& c)
{
    // 1) 로그인 요청: 64바이트 토큰(서버는 지금 토큰 내용을 검증하지 않음)
    {
        char token[64] = { 0 };
        PacketWriter w(AuthProtocol::PACKET_CS_GAME_LOGIN_REQ);
        w.PutBytes(token, sizeof(token));
        SendRaw(c, w);
    }
    // 2) UID로 캐릭터 선택(서버가 DB에서 로드 후 세션을 Field로 이동)
    {
        PacketWriter w(AuthProtocol::PACKET_CS_GAME_CHARACTER_SELECT);
        w.PutU64(c.characterUID);
        SendRaw(c, w);
    }
}

// 랜덤워크 한 스텝을 계산해 이동 입력 패킷을 보낸다.
void LoadTestManager::DoMove(DummyClient& c, uint32 now)
{
    float step = WALK_SPEED * (m_cfg.moveIntervalMs / 1000.f);   // 이번 주기에 이동할 거리
    int roll = rand() % 100;

    if (roll < 10) // 10% 확률로 잠깐 멈춤
    {
        c.moving = false;
    }
    else
    {
        c.moving = true;
        if (roll < 35) // 가끔(25%) 방향을 살짝 틀기
            c.headingRad += ((rand() % 200 - 100) / 100.f);

        float nx = c.x + cosf(c.headingRad) * step;
        float ny = c.y + sinf(c.headingRad) * step;

        // 스폰 anchor 기준 WALK_RADIUS 안에 머물기
        float dx = nx - c.anchorX, dy = ny - c.anchorY;
        if (sqrtf(dx * dx + dy * dy) > WALK_RADIUS)
        {
            c.headingRad += PI_F; // 반대로 돌아 anchor 쪽으로
            nx = c.x + cosf(c.headingRad) * step;
            ny = c.y + sinf(c.headingRad) * step;
        }
        // 월드 경계 안으로 clamp
        nx = std::max(WORLD_MIN + 100.f, std::min(WORLD_MAX - 100.f, nx));
        ny = std::max(WORLD_MIN + 100.f, std::min(WORLD_MAX - 100.f, ny));
        c.x = nx; c.y = ny;
    }

    // 이동 입력 패킷: x, y, z, yaw(도), moveFlag.
    float yawDeg = c.headingRad * 180.f / PI_F;
    PacketWriter w(FieldProtocol::PACKET_CS_UPDATE_CHARACTER_MOVEMENT_INPUT);
    w.PutFloat(c.x);
    w.PutFloat(c.y);
    w.PutFloat(c.z);
    w.PutFloat(yawDeg);
    w.PutU8(c.moving ? 1 : 0);
    SendRaw(c, w);
}

// swing 공격 패킷을 보낸다. swingIdx는 1..4만 허용(서버가 그 외엔 Disconnect).
void LoadTestManager::DoAttack(DummyClient& c, uint32 now)
{
    float yawDeg = c.headingRad * 180.f / PI_F;
    PacketWriter w(FieldProtocol::PACKET_CS_SWING_LEFT_ATTACK);
    w.PutFloat(yawDeg);
    w.PutU8((uint8)(rand() % 4 + 1)); // swingIdx는 1..4 (그 외엔 서버가 끊음)
    SendRaw(c, w);
}

// RTT핑을 보낸다. 송신 시각을 로컬(rttSentMs)에 기록해 두고, 에코가 오면 그걸로 왕복 계산.
void LoadTestManager::DoRtt(DummyClient& c, uint32 now)
{
    // 서버는 이 payload를 버리고 자기 시각을 에코하므로, 왕복 측정용 송신 시각은 로컬에 보관한다.
    uint64 sendMs = (uint64)GetTickCount64();
    c.rttSentMs.store(sendMs);
    PacketWriter w(FieldProtocol::PACKET_CS_RTT_SEND);
    w.PutU64(sendMs);
    SendRaw(c, w);
}

// 본인 부활 요청을 보낸다(죽었을 때만 서버가 받아줌).
void LoadTestManager::DoRespawn(DummyClient& c)
{
    // 본인 부활 요청. payload 없음(클라 M1RespawnWidget와 동일). 서버는 죽었을 때만 허용.
    PacketWriter w(FieldProtocol::PACKET_CS_RESPAWN_REQ);
    SendRaw(c, w);
}

// 스킬 사용. 슬롯 0~3을 순환(0=버프, 1~3=공격). 서버는 [0,4) 밖이면 Disconnect 하므로 범위 준수.
// 버프는 쿨타임/마나/지속 경로를, 공격 스킬은 히트 판정·경험치 경로를 가동시킨다.
void LoadTestManager::DoSkill(DummyClient& c)
{
    PacketWriter w(FieldProtocol::PACKET_CS_USE_SKILL);
    w.PutU8(c.nextSkillSlot);                                  // 이번에 쓸 스킬 슬롯
    SendRaw(c, w);
    c.nextSkillSlot = (uint8)((c.nextSkillSlot + 1) % SKILL_SLOT_COUNT);  // 다음엔 다음 슬롯
}

// 줍기. 지금 보이는 드롭 타깃(pickupTargetUID)이 있을 때만 송신.
// 성공하면 서버가 드롭을 지우고(1025) 그때 타깃이 0으로 비워진다(중복 줍기 방지는 서버 액터 모델이 보장).
void LoadTestManager::DoPickup(DummyClient& c)
{
    uint64 target = c.pickupTargetUID.load();
    if (target == 0) return;                                   // 주울 게 없으면 패스
    PacketWriter w(FieldProtocol::PACKET_CS_PICK_UP_ITEM);
    w.PutU64(target);
    SendRaw(c, w);
}

// 소모품 사용. 퀵슬롯 0(HP포션)·1(MP포션)을 번갈아 사용한다(시드가 퀵슬롯에 포션을 깔아둔다고 가정).
// 슬롯이 비어 있어도 서버는 실패 응답(정상)을 주므로, 그 처리 경로도 함께 검증된다.
// 사용 성공 시 개수 감소 → ItemCountUpdate DB 잡이 꾸준히 생성되어 DB 부하의 "정상 스트림"이 된다.
void LoadTestManager::DoUseItem(DummyClient& c)
{
    PacketWriter w(FieldProtocol::PACKET_CS_USE_ITEM);
    w.PutU8(SLOT_QUICKSLOT);                                   // 슬롯 타입 = 퀵슬롯
    w.PutI16((int16)c.useQuickIdx);                            // 0=HP, 1=MP
    SendRaw(c, w);
    c.useQuickIdx = (uint8)((c.useQuickIdx + 1) % QUICK_SLOT_MAX);  // 다음엔 반대 포션
}

// 슬롯 스왑. 인벤토리 슬롯 0 <-> 1 교환(시드가 두 슬롯에 아이템을 깔아둠).
// 슬롯 write-back(coalescing: dirty 모아서 주기 flush) 경로를 부하에 포함시킨다.
void LoadTestManager::DoSwapSlot(DummyClient& c)
{
    PacketWriter w(FieldProtocol::PACKET_CS_SWAP_SLOT);
    w.PutU8(SLOT_INVENTORY);  w.PutI16(0);                     // from: 인벤 0
    w.PutU8(SLOT_INVENTORY);  w.PutI16(1);                     // to:   인벤 1
    SendRaw(c, w);
}

// 아이템 삭제. 인벤 슬롯 3(시드의 장비 슬롯)을 삭제. 파괴적이라 기본 off(설정 deleteItemIntervalMs=0).
// 켜면 시드 아이템을 점차 소진해 빈 슬롯이 되고, 이후엔 서버가 실패 응답(정상)을 준다.
// 용도: 삭제 경로가 부하 중에도 서버를 죽이지 않는지 확인.
void LoadTestManager::DoDeleteItem(DummyClient& c)
{
    PacketWriter w(FieldProtocol::PACKET_CS_DELETE_ITEM);
    w.PutU8(SLOT_INVENTORY);                                   // 슬롯 타입 = 인벤토리
    w.PutI16(3);                                               // 슬롯 인덱스(시드 장비 위치)
    SendRaw(c, w);
}

// 인벤 청소. 인벤 슬롯 4~39를 순회하며 삭제를 시도한다(시드 슬롯 0~3은 스왑/삭제 행동이 쓰므로 보존).
// 빈 슬롯이면 서버가 실패 응답(정상, success=0)을 주고, 찬 슬롯이면 삭제 → 다음 픽업이 그 자리에 다시 insert.
// 목적: 픽업으로 인벤(40칸)이 꽉 차 InsertItemJob(=DB insert)이 마르는 걸 막아 insert 부하를 지속시킨다.
void LoadTestManager::DoCleanupInventory(DummyClient& c)
{
    PacketWriter w(FieldProtocol::PACKET_CS_DELETE_ITEM);
    w.PutU8(SLOT_INVENTORY);                                   // 슬롯 타입 = 인벤토리
    w.PutI16(c.nextCleanupSlot);                              // 이번에 비울 슬롯
    SendRaw(c, w);

    // 다음엔 다음 슬롯. CLEANUP_SLOT_FIRST..(INVENTORY_SLOT_MAX-1) 순환.
    c.nextCleanupSlot++;
    if (c.nextCleanupSlot >= INVENTORY_SLOT_MAX)
        c.nextCleanupSlot = CLEANUP_SLOT_FIRST;
}

// ---------------------------------------------------------------------------
// 행동 + 통계 스레드
// ---------------------------------------------------------------------------
// 모든 봇을 순회하며 주기가 된 봇에게 이동/공격/RTT/부활을 시킨다(15ms 틱).
void LoadTestManager::BehaviorLoop()
{
    while (m_running.load())
    {
        uint32 now = GetTickCount();
        for (int i = 0; i < m_cfg.userCount; ++i)
        {
            DummyClient& c = m_clients[i];
            if (c.closed.load() || c.state != ConnState::InField)   // 필드 진입한 봇만
                continue;

            // 게임 내 죽음: 이동/공격/RTT 멈추고, 잠깐 뒤 부활 요청 1회(사망화면 흉내).
            if (c.deadInGame.load())
            {
                if (c.respawnReqTick == 0)
                    c.respawnReqTick = now + RESPAWN_REQ_DELAY_MS;   // 부활 요청 시각 예약
                else if (!c.respawnReqSent && (int)(now - c.respawnReqTick) >= 0)
                {
                    DoRespawn(c);              // 시간이 되면 1회만 부활 요청
                    c.respawnReqSent = true;
                }
                continue;   // 죽어있는 동안은 다른 행동 안 함
            }
            // 살아있음: 죽음 관련 예약 초기화(다음 죽음 대비)
            c.respawnReqSent = false;
            c.respawnReqTick = 0;

            // 이동 주기 도달 → 이동. 다음 주기는 약간의 랜덤 지터를 더해 한 틱에 몰리지 않게.
            if (m_cfg.moveIntervalMs > 0 && (int)(now - c.nextMoveTick) >= 0)
            {
                DoMove(c, now);
                c.nextMoveTick = now + m_cfg.moveIntervalMs + (rand() % (m_cfg.moveIntervalMs / 2 + 1));
            }
            // 공격 주기 도달 → swing.
            if (m_cfg.attackIntervalMs > 0 && (int)(now - c.nextAttackTick) >= 0)
            {
                DoAttack(c, now);
                c.nextAttackTick = now + m_cfg.attackIntervalMs + (rand() % (m_cfg.attackIntervalMs / 2 + 1));
            }
            // RTT 주기 도달 → 핑.
            if (m_cfg.rttIntervalMs > 0 && (int)(now - c.nextRttTick) >= 0)
            {
                DoRtt(c, now);
                c.nextRttTick = now + m_cfg.rttIntervalMs;
            }
            // 스킬 사용 주기.
            if (m_cfg.skillIntervalMs > 0 && (int)(now - c.nextSkillTick) >= 0)
            {
                DoSkill(c);
                c.nextSkillTick = now + m_cfg.skillIntervalMs + (rand() % (m_cfg.skillIntervalMs / 2 + 1));
            }
            // 줍기 주기(타깃 있을 때만 실제 송신).
            if (m_cfg.pickupIntervalMs > 0 && (int)(now - c.nextPickupTick) >= 0)
            {
                DoPickup(c);
                c.nextPickupTick = now + m_cfg.pickupIntervalMs;
            }
            // 소모품 사용 주기.
            if (m_cfg.useItemIntervalMs > 0 && (int)(now - c.nextUseItemTick) >= 0)
            {
                DoUseItem(c);
                c.nextUseItemTick = now + m_cfg.useItemIntervalMs + (rand() % (m_cfg.useItemIntervalMs / 2 + 1));
            }
            // 슬롯 스왑 주기.
            if (m_cfg.swapIntervalMs > 0 && (int)(now - c.nextSwapTick) >= 0)
            {
                DoSwapSlot(c);
                c.nextSwapTick = now + m_cfg.swapIntervalMs + (rand() % (m_cfg.swapIntervalMs / 2 + 1));
            }
            // 아이템 삭제 주기(기본 off).
            if (m_cfg.deleteItemIntervalMs > 0 && (int)(now - c.nextDeleteTick) >= 0)
            {
                DoDeleteItem(c);
                c.nextDeleteTick = now + m_cfg.deleteItemIntervalMs;
            }
            // 인벤 청소 주기(픽업으로 쌓인 아이템 삭제 → insert 부하 지속).
            if (m_cfg.cleanupIntervalMs > 0 && (int)(now - c.nextCleanupTick) >= 0)
            {
                DoCleanupInventory(c);
                c.nextCleanupTick = now + m_cfg.cleanupIntervalMs + (rand() % (m_cfg.cleanupIntervalMs / 2 + 1));
            }
        }
        Sleep(15);   // 약 66Hz 틱
    }
}

// 1초마다 스폰 타임아웃을 점검하고, RTT 백분위/처리량/오류수를 콘솔에 출력.
void LoadTestManager::StatsLoop()
{
    long long lastRecvP = 0, lastSentP = 0;   // 직전 창의 누적값(초당 계산용)
    while (m_running.load())
    {
        Sleep(1000);

        // 스폰 타임아웃 점검: 선택 보낸 뒤 5초 안에 CREATE_MY가 안 온 봇.
        uint32 now = GetTickCount();
        for (int i = 0; i < m_cfg.userCount; ++i)
        {
            DummyClient& c = m_clients[i];
            if (c.state == ConnState::LoggedIn && !c.spawnTimeoutCounted
                && (int)(now - c.connectTick) > 5000)
            {
                ReportError(ERR_SPAWN_TIMEOUT, c, L"no CREATE_MY within 5s after select");
                c.spawnTimeoutCounted = true;   // 한 번만 카운트
            }
        }

        // 이번 1초 창의 RTT 샘플을 통째로 빼와서 백분위 계산(락 보유 시간 최소화).
        std::vector<double> samples;
        {
            std::lock_guard<std::mutex> lk(m_rttMtx);
            samples.swap(m_rttSamples);
        }
        // 전체 구간 min/max/avg 누적(이번 창 샘플을 통째로 접어 넣음).
        for (double v : samples)
        {
            if (m_rttCountTotal == 0 || v < m_rttMinTotal) m_rttMinTotal = v;
            if (m_rttCountTotal == 0 || v > m_rttMaxTotal) m_rttMaxTotal = v;
            m_rttSumTotal += v;
            ++m_rttCountTotal;
        }
        double p50 = -1, p95 = -1, p99 = -1;
        if (!samples.empty())
        {
            std::sort(samples.begin(), samples.end());
            auto pick = [&](double q) { return samples[(size_t)(q * (samples.size() - 1))]; };
            p50 = pick(0.50); p95 = pick(0.95); p99 = pick(0.99);
        }

        // 초당 송수신 패킷 수(이번 누적 - 직전 누적).
        long long recvP = m_recvPackets.load();
        long long sentP = m_sentPackets.load();
        long long recvPS = recvP - lastRecvP;
        long long sentPS = sentP - lastSentP;
        lastRecvP = recvP; lastSentP = sentP;

        long long errTotal = 0;
        for (int i = 0; i < ERR_CAT_COUNT; ++i) errTotal += m_errors[i].load();

        // 한 줄 요약: 접속/필드 수 | 초당 송수신 | RTT 백분위 | 누적 오류.
        printf("[stat] conn=%d field=%d | tx/s=%lld rx/s=%lld | rtt(ms) p50=%.1f p95=%.1f p99=%.1f | rstPre=%lld rstIn=%lld | err=%lld\n",
            m_connected.load(), m_inField.load(), sentPS, recvPS, p50, p95, p99,
            m_resetPreField.load(), m_resetInField.load(), errTotal);
    }
}

// 테스트 종료 후 최종 리포트 출력(누적 통계 + 종류별 오류 수).
void LoadTestManager::PrintReport(double elapsedSec)
{
    printf("\n================ LOAD TEST REPORT ================\n");
    printf("elapsed            : %.1f sec\n", elapsedSec);
    printf("target users       : %d\n", m_cfg.userCount);
    printf("sent packets/bytes : %lld / %lld\n", m_sentPackets.load(), m_sentBytes.load());
    printf("recv packets/bytes : %lld / %lld\n", m_recvPackets.load(), m_recvBytes.load());
    printf("reconnects         : %lld\n", m_reconnects.load());
    printf("connect ok         : %lld\n", m_connectOk.load());
    printf("reset pre-field    : %lld  (스폰 전 리셋: 백로그 풀/초기 거부 의심)\n", m_resetPreField.load());
    printf("reset in-field     : %lld  (세션 중 리셋: 진짜 서버 드롭)\n", m_resetInField.load());
    printf("deaths / respawns  : %lld / %lld\n", m_gameDeaths.load(), m_respawns.load());
    printf("item pickups       : %lld\n", m_pickups.load());
    printf("skill ok / fail    : %lld / %lld\n", m_skillOk.load(), m_skillFail.load());
    if (m_rttCountTotal > 0)
        printf("rtt(ms) min/avg/max: %.1f / %.1f / %.1f  (n=%lld)\n",
            m_rttMinTotal, m_rttSumTotal / (double)m_rttCountTotal, m_rttMaxTotal, m_rttCountTotal);
    else
        printf("rtt(ms) min/avg/max: (no samples)\n");
    if (elapsedSec > 0)
    {
        printf("avg tx/s, rx/s     : %.0f / %.0f\n",
            m_sentPackets.load() / elapsedSec, m_recvPackets.load() / elapsedSec);
    }
    // 수신 패킷 종류별 분포(확산 전송 분석). 어떤 SC가 부하의 대부분을 차지하는지 보여준다.
    printf("-------- recv packet types (field 1000~1041) --------\n");
    for (int i = 0; i < FIELD_TYPE_COUNT; ++i)
    {
        long long v = m_recvByType[i].load();
        if (v > 0) printf("  %-5u : %lld\n", (unsigned)(FIELD_TYPE_BASE + i), v);
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
