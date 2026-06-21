#pragma once
//
// Drives the whole load test: ramps up N connections, runs movement/attack/RTT
// behavior, injects RST disconnects, validates the server's response stream,
// and prints live + final stats.
//
#include <winsock2.h>
#include <memory>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <string>
#include "DummyClient.h"

struct LoadConfig
{
    std::string ip = "127.0.0.1";
    uint16 port = 11211;

    int  userCount = 100;       // characterUID 1 .. userCount
    int  rstPercent = 0;        // % of users to RST mid-test
    int  rampPerSec = 500;      // connections opened per second (accept is serial!)

    int  moveIntervalMs = 200;  // movement input cadence per bot
    int  attackIntervalMs = 1000;
    int  rttIntervalMs = 1000;

    int  rstAfterSec = 30;          // when the first RST wave fires (after ramp)
    int  churnIntervalSec = 0;      // repeat the RST wave every N sec (0 = single wave)
    int  reconnectCooldownSec = 2;  // RST'd bots reconnect after this (0 = no reconnect)
    int  durationSec = 60;          // total run time
};

// Categories of server misbehavior the dummy can detect from the wire alone.
enum ErrCat
{
    ERR_UNKNOWN_TYPE = 0,   // server sent an unknown packet id
    ERR_LEN_MISMATCH,       // payload size doesn't match the schema
    ERR_BAD_VALUE,          // HP out of [0,Max], NaN / out-of-world coords
    ERR_UNEXPECTED_CLOSE,   // server dropped a connection we didn't RST
    ERR_SPAWN_TIMEOUT,      // no CREATE_MY_CHARACTER after select
    ERR_ID_CONSISTENCY,     // dup create / delete of unknown id
    ERR_CAT_COUNT
};

class LoadTestManager
{
public:
    explicit LoadTestManager(const LoadConfig& cfg);
    ~LoadTestManager();

    bool Init();   // WSAStartup + IOCP + worker threads
    void Run();    // ramp -> simulate -> RST wave -> drain -> report

private:
    // connection lifecycle
    void ConnectOne(DummyClient& c);
    void PostRecv(DummyClient& c);
    void HandleDisconnect(DummyClient& c, bool serverInitiated);
    void RequestRST(DummyClient& c);
    void ResetForReconnect(DummyClient& c);   // reuse a dead slot for a fresh connection

    // threads
    void WorkerLoop();     // IOCP recv completions
    void BehaviorLoop();   // periodic sends
    void StatsLoop();      // 1s console snapshot

    // recv parsing / validation
    void ParseFrames(DummyClient& c);
    void HandlePacket(DummyClient& c, uint16 type, PacketReader& r);

    // outgoing
    void SendRaw(DummyClient& c, const PacketWriter& w);
    void SendLoginAndSelect(DummyClient& c);
    void DoMove(DummyClient& c, uint32 now);
    void DoAttack(DummyClient& c, uint32 now);
    void DoRtt(DummyClient& c, uint32 now);

    void PrintReport(double elapsedSec);

private:
    LoadConfig m_cfg;
    HANDLE     m_iocp = nullptr;

    std::unique_ptr<DummyClient[]> m_clients;  // stable storage (no reallocation)

    std::vector<std::thread> m_workers;
    std::thread m_behavior;
    std::thread m_stats;

    std::atomic<bool> m_running{ false };
    std::atomic<bool> m_shuttingDown{ false };

    // --- aggregate stats ---
    std::atomic<int>       m_connected{ 0 };
    std::atomic<int>       m_inField{ 0 };
    std::atomic<long long> m_sentPackets{ 0 };
    std::atomic<long long> m_recvPackets{ 0 };
    std::atomic<long long> m_sentBytes{ 0 };
    std::atomic<long long> m_recvBytes{ 0 };
    std::atomic<long long> m_errors[ERR_CAT_COUNT];
    std::atomic<long long> m_reconnects{ 0 };

    // RTT samples for the current 1s window (ms). Protected by m_rttMtx.
    std::mutex          m_rttMtx;
    std::vector<double> m_rttSamples;

    static const char* ErrName(int cat);
};
