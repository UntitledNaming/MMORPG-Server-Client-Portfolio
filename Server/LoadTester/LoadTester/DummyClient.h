#pragma once
//
// 봇(가짜 플레이어) 한 명의 모든 상태를 담는 구조체. characterUID 하나당 DummyClient 하나.
// 로직은 전부 LoadTestManager 쪽에 있고, 여기는 순수 데이터만 보관한다.
//
// [스레드 안전 메모]
//  - 한 소켓에는 동시에 WSARecv가 최대 1개만 걸려 있다. 즉 수신 완료(recv completion)는
//    그걸 꺼낸 워커 스레드 하나만 건드린다(두 워커가 같은 소켓을 동시에 안 본다).
//  - 이동 관련 필드는 behavior(행동) 스레드가 소유한다.
//  - 두 스레드가 같이 만지는 필드만 atomic 으로 둔다(closed, deadInGame, rttSentMs).
//
#include <winsock2.h>          // SOCKET, OVERLAPPED, WSABUF
#include <atomic>              // std::atomic (스레드 경계를 넘는 필드 보호)
#include <unordered_set>       // 내가 "알고 있는" 타 캐릭터 id 집합
#include "ByteStream.h"        // (간접적으로) 공용 타입 정의 uint8/16/32/64 등

// 봇 한 명의 접속 단계. 상태에 따라 행동/통계 처리가 갈린다.
enum class ConnState : uint8
{
    Disconnected,   // 아직 소켓 없음(초기/재접속 대기)
    Connecting,     // (예약값) 연결 중
    LoggedIn,       // 로그인+캐릭터선택 보냄. 아직 CREATE_MY 못 받음(스폰 대기)
    InField,        // 필드 진입 완료. 이동/공격/RTT 시뮬레이션 중
    Dead            // 끊긴 슬롯(서버가 끊었거나 우리가 RST했거나 종료)
};

struct DummyClient
{
    SOCKET     sock = INVALID_SOCKET;     // 이 봇의 TCP 소켓
    uint64     characterUID = 0;          // DB 캐릭터 UID(1..N). 캐릭터 선택 때 보냄
    int        index = 0;                 // 0부터 시작하는 배열 슬롯 번호(로그용)
    ConnState  state = ConnState::Disconnected;

    // --- 수신 쪽 (워커 스레드가 소유) ---
    OVERLAPPED recvOv{};                  // WSARecv용 비동기 오버랩 구조체
    WSABUF     wsaRecv{};                 // 수신 버퍼 포인터/길이 묶음
    char       recvBuf[8192];             // 들어온 바이트를 쌓아두는 링 없는 단순 버퍼
    int        recvUsed = 0;              // recvBuf에 현재 쌓여있는 유효 바이트 수

    // --- 월드/이동 (behavior 스레드가 소유) ---
    float  x = 0.f, y = 0.f, z = 0.f;     // 현재 위치
    float  anchorX = 0.f, anchorY = 0.f;  // 랜덤워크의 "중심점"(고정된 원점)
    float  headingRad = 0.f;              // 현재 진행 방향(라디안)
    bool   moving = false;                // 지금 이동 중인지(서버로 보낼 moveFlag)
    bool   spawnReceived = false;         // 이번 세션에 CREATE_MY 받았는지(재접속 시 리셋)
    bool   originSet = false;             // 첫 스폰 때 anchor를 한 번만 고정(재접속해도 유지)

    // --- 행동 타이머 (GetTickCount 기준 ms, 전부 behavior 스레드 소유) ---
    uint32 connectTick = 0;               // 접속 성공 시각(스폰 타임아웃 판정용)
    uint32 nextMoveTick = 0;              // 다음 이동 패킷 보낼 시각
    uint32 nextAttackTick = 0;            // 다음 공격(swing) 패킷 보낼 시각
    uint32 nextRttTick = 0;               // 다음 RTT핑 보낼 시각
    uint32 reconnectAtTick = 0;           // 0=재접속 예약 없음, 그 외=이 시각에 다시 접속
    bool   spawnTimeoutCounted = false;   // 스폰 타임아웃을 이미 한 번 셌는지(중복 카운트 방지)

    // 컨텐츠 부하용 추가 행동 타이머 (스킬/줍기/사용/스왑/삭제). 0이면 아직 미예약.
    uint32 nextSkillTick = 0;             // 다음 스킬 사용 시각
    uint8  nextSkillSlot = 0;             // 이번에 쓸 스킬 슬롯(0=버프, 1~3=공격). 매번 0~3 순환
    uint32 nextPickupTick = 0;            // 다음 줍기 시도 시각(pickupTargetUID가 있을 때만 실제 송신)
    uint32 nextUseItemTick = 0;           // 다음 소모품 사용 시각
    uint8  useQuickIdx = 0;               // 이번에 쓸 퀵슬롯 인덱스(0=HP포션, 1=MP포션). 매번 0~1 순환
    uint32 nextSwapTick = 0;              // 다음 슬롯 스왑 시각
    uint32 nextDeleteTick = 0;            // 다음 아이템 삭제 시각(설정에서 켤 때만)
    uint32 nextCleanupTick = 0;           // 다음 인벤 청소 시각(픽업으로 쌓인 아이템 삭제)
    int16  nextCleanupSlot = 4;           // 이번에 청소 시도할 인벤 슬롯(4~39 순환). 시드 슬롯 0~3은 보존(CLEANUP_SLOT_FIRST와 동일)

    // --- 정합성 추적 (전부 worker 스레드 전용. 한 소켓의 수신완료는 워커 하나만 처리하므로 락 불필요) ---
    std::unordered_set<uint64> knownOthers;    // 서버가 "보이게 하라"고 한 타 캐릭터 id들
    std::unordered_set<uint64> knownMonsters;  // 서버가 "보이게 하라"고 한 몬스터 id들(시야 진입/이탈/죽음/부활)
    std::unordered_set<uint64> knownDropItems; // 서버가 "보이게 하라"고 한 필드 드롭 아이템 dropUID들(스폰/이탈/줍힘/만료)

    // --- 줍기 타깃 (스레드 경계) ---
    // worker가 드롭을 보면(1024) 비어있을 때 여기에 dropUID를 꽂고, 그 드롭이 사라지면(1025) 0으로 지운다.
    // behavior 스레드는 이 값만 읽어서 "주울 대상이 있으면" 줍기 패킷을 보낸다.
    // 컨테이너(knownDropItems)를 두 스레드가 공유하지 않으려고, 행동에 필요한 "현재 타깃 1개"만 atomic으로 노출한다.
    std::atomic<uint64> pickupTargetUID{ 0 };  // 0 = 지금 주울 드롭 없음

    // --- 경험치/레벨 검증 (worker 전용) ---
    int32  lastExp   = 0;   // 마지막으로 본 누적 경험치(역행 검사용). 레벨업 때는 줄 수 있어 levelUp==false일 때만 비교
    uint16 lastLevel = 1;   // 마지막으로 본 레벨(역행 검사용)

    // --- 게임 내 죽음/부활 (서버 respawn 흐름에 대응) ---
    uint64 entityId       = 0;              // 내 서버 id(== 브로드캐스트의 sessionID). CREATE_MY에서 저장
    std::atomic<bool> deadInGame{ false };  // worker가 내 HP<=0 보면 set, RESPAWN_RES_TO_ME에서 clear
    bool   respawnReqSent = false;          // behavior 스레드: 이번 죽음에 부활요청 이미 보냈는지
    uint32 respawnReqTick = 0;              // behavior 스레드: 부활요청 보낼 시각(사망화면 흉내). 0=미예약

    // --- RTT 측정 (behavior가 송신 시각 기록, worker가 에코에서 읽음) ---
    // 서버 에코는 서버 자기 시각(클라 snapshot용)이라 왕복 측정에 못 씀 → 로컬 송신 시각으로 잰다.
    std::atomic<uint64> rttSentMs{ 0 };    // 마지막 RTT 송신 시각 GetTickCount64(). 0=대기중인 핑 없음

    // --- 수명 가드 (스레드 경계) ---
    std::atomic<bool> closed{ false };      // 누가 먼저 닫든 정리는 딱 한 번만 하도록 하는 플래그
    bool   rstRequested = false;            // RST로 끊기 직전에만 set(서버 오류로 오인 안 하려고)
};
