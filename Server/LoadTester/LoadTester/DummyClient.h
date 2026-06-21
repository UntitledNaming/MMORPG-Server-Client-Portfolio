#pragma once
//
// Per-connection state for one simulated player (one DB characterUID).
// Plain data struct; all logic lives in LoadTestManager.
//
// Threading note: a socket has at most ONE outstanding WSARecv at a time, so
// recv-side fields are only ever touched by the single worker that dequeued
// that completion (never two at once). The behavior thread owns the movement
// fields. The only cross-thread field is `closed` (atomic guard).
//
#include <winsock2.h>
#include <atomic>
#include <unordered_set>
#include "ByteStream.h"

enum class ConnState : uint8
{
    Disconnected,
    Connecting,
    LoggedIn,   // login+select sent, waiting for CREATE_MY_CHARACTER
    InField,    // spawned, simulating
    Dead
};

struct DummyClient
{
    SOCKET     sock = INVALID_SOCKET;
    uint64     characterUID = 0;
    int        index = 0;                 // 0-based slot, just for logging
    ConnState  state = ConnState::Disconnected;

    // --- recv side (worker thread) ---
    OVERLAPPED recvOv{};
    WSABUF     wsaRecv{};
    char       recvBuf[8192];
    int        recvUsed = 0;

    // --- world / movement (behavior thread) ---
    float  x = 0.f, y = 0.f, z = 0.f;     // current position
    float  anchorX = 0.f, anchorY = 0.f;  // FIXED origin = random-walk center
    float  headingRad = 0.f;              // current heading for the walk
    bool   moving = false;
    bool   spawnReceived = false;         // got CREATE_MY this session (reset on reconnect)
    bool   originSet = false;             // anchor pinned on the FIRST spawn, kept across reconnects

    // --- behavior timers (GetTickCount ms) ---
    uint32 connectTick = 0;
    uint32 nextMoveTick = 0;
    uint32 nextAttackTick = 0;
    uint32 nextRttTick = 0;
    uint32 reconnectAtTick = 0;           // 0 = no pending reconnect; else when to re-dial
    bool   spawnTimeoutCounted = false;

    // --- correctness tracking ---
    std::unordered_set<uint64> knownOthers; // ids we've been told to render

    // --- lifecycle guard (cross-thread) ---
    std::atomic<bool> closed{ false };
    bool   rstRequested = false;           // set only right before we RST it
};
