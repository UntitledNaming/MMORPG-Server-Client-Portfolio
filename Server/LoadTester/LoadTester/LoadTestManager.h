#pragma once
//
// 부하 테스트 전체를 운전하는 매니저.
//   - 접속 N개를 천천히 늘리고(ramp)
//   - 이동/공격/RTT 행동을 돌리고
//   - RST 끊기를 주입하고
//   - 서버 응답 스트림을 검증하고
//   - 실시간/최종 통계를 출력한다.
//
#include <winsock2.h>    // SOCKET, IOCP
#include <memory>        // std::unique_ptr
#include <vector>        // 워커 스레드 목록, RTT 샘플
#include <thread>        // std::thread
#include <atomic>        // 통계 카운터(여러 스레드가 증가)
#include <mutex>         // RTT 샘플 보호
#include <string>        // 설정의 IP 문자열
#include "DummyClient.h" // 봇 상태 구조체

// 테스트 파라미터. main.cpp 가 사용자 입력으로 채운다. 주석의 값은 기본값.
struct LoadConfig
{
    std::string ip = "127.0.0.1";   // 접속 대상 서버 IP
    uint16 port = 11211;            // 접속 대상 포트

    int  userCount = 100;       // 봇 수 = characterUID 1 .. userCount
    int  rstPercent = 0;        // 테스트 도중 RST로 끊을 봇 비율(%)
    int  rampPerSec = 500;      // 초당 새로 여는 접속 수(서버 accept가 직렬이라 한 번에 X)

    int  moveIntervalMs = 200;  // 봇당 이동 패킷 주기
    int  attackIntervalMs = 1000; // 봇당 공격(swing) 주기
    int  rttIntervalMs = 1000;  // 봇당 RTT핑 주기

    // --- 컨텐츠 부하 주기(0=해당 행동 끔). 아이템/스킬 경로를 부하에 포함시켜 DB write-back 등 실부하를 흉내낸다. ---
    int  skillIntervalMs   = 3000;  // 봇당 스킬 사용 주기(슬롯 0~3 순환: 버프+공격 스킬)
    int  pickupIntervalMs  = 1000;  // 봇당 줍기 시도 주기(주울 드롭이 보일 때만 실제 송신)
    int  useItemIntervalMs = 4000;  // 봇당 소모품(포션) 사용 주기 → ItemCountUpdate DB 잡 생성
    int  swapIntervalMs    = 6000;  // 봇당 슬롯 스왑 주기 → 슬롯 write-back 경로
    int  deleteItemIntervalMs = 0;  // 봇당 아이템 삭제 주기. 기본 0(끔): 파괴적이라 시드 아이템을 소진시킴
    int  cleanupIntervalMs = 1000;  // 봇당 인벤 청소 주기. 인벤 4~39를 순회 삭제해 픽업으로 쌓인 아이템을 비운다(insert 부하가 마르지 않게). 0=끔

    int  rstAfterSec = 30;          // 첫 RST 웨이브 시점(ramp 끝난 뒤)
    int  churnIntervalSec = 0;      // RST 웨이브 반복 주기(0=1회만)
    int  reconnectCooldownSec = 2;  // RST된 봇이 다시 붙기까지(0=재접속 안 함)
    int  durationSec = 60;          // 전체 실행 시간
};

// 더미가 "와이어(주고받는 바이트)"만 보고도 잡아낼 수 있는 서버 이상 종류들.
enum ErrCat
{
    ERR_UNKNOWN_TYPE = 0,   // 서버가 모르는 패킷 id를 보냄
    ERR_LEN_MISMATCH,       // 페이로드 크기가 스키마와 안 맞음
    ERR_BAD_VALUE,          // HP가 [0,Max] 밖, 좌표가 NaN/월드 밖
    ERR_UNEXPECTED_CLOSE,   // 우리가 안 끊었는데 서버가 끊음
    ERR_SPAWN_TIMEOUT,      // 캐릭터 선택 후 CREATE_MY_CHARACTER가 안 옴
    ERR_ID_CONSISTENCY,     // 중복 create / 모르는 id delete
    ERR_CAT_COUNT           // (개수 세는 용도. 항상 맨 끝)
};

class LoadTestManager
{
public:
    explicit LoadTestManager(const LoadConfig& cfg);
    ~LoadTestManager();

    bool Init();   // WSAStartup + IOCP 생성 + 워커 스레드 기동
    void Run();    // 램프 → 시뮬레이션 → RST 웨이브 → 정리 → 리포트

private:
    // --- 접속 수명주기 ---
    void ConnectOne(DummyClient& c);                       // 소켓 하나 열고 로그인+선택 송신
    void PostRecv(DummyClient& c);                         // 비동기 수신 1건 걸기
    void HandleDisconnect(DummyClient& c, bool serverInitiated); // 정리(딱 한 번만)
    void CountConnReset(const DummyClient& c, int err);    // connect 후 리셋(10054 등)을 스폰 전/세션 중으로 나눠 집계
    void RequestRST(DummyClient& c);                       // SO_LINGER0로 RST 끊기 + 재접속 예약
    void ResetForReconnect(DummyClient& c);                // 죽은 슬롯을 새 접속으로 재사용

    // --- 스레드들 ---
    void WorkerLoop();     // IOCP 수신 완료 처리(여러 개)
    void BehaviorLoop();   // 주기적 송신(이동/공격/RTT/부활) 1개
    void StatsLoop();      // 1초마다 콘솔 스냅샷 1개

    // --- 수신 파싱/검증 ---
    void ParseFrames(DummyClient& c);                      // 버퍼에서 완성된 프레임을 떼어 처리
    void HandlePacket(DummyClient& c, uint16 type, PacketReader& r); // 타입별 검증/반응
    void ValidateInventoryTail(DummyClient& c, PacketReader& r);     // CREATE_MY 꼬리(인벤/장비/퀵슬롯) 검증

    // --- 송신 ---
    void SendRaw(DummyClient& c, const PacketWriter& w);   // 완성된 패킷을 끝까지 send
    void SendLoginAndSelect(DummyClient& c);               // 로그인(토큰) + 캐릭터선택(UID)
    void DoMove(DummyClient& c, uint32 now);               // 랜덤워크 한 스텝 + 이동 패킷
    void DoAttack(DummyClient& c, uint32 now);             // swing 공격 패킷
    void DoRtt(DummyClient& c, uint32 now);                // RTT핑 패킷(+로컬 송신시각 기록)
    void DoRespawn(DummyClient& c);                        // CS_RESPAWN_REQ(죽었을 때만 서버가 허용)

    // --- 컨텐츠 부하 행동(아이템/스킬). 슬롯은 시드(아래 seed SQL)가 깔아둔 고정 배치를 가정한다. ---
    void DoSkill(DummyClient& c);                          // CS_USE_SKILL(슬롯 0~3 순환). 버프/공격 경로 가동
    void DoPickup(DummyClient& c);                         // CS_PICK_UP_ITEM(현재 pickupTargetUID 대상)
    void DoUseItem(DummyClient& c);                        // CS_USE_ITEM(퀵슬롯 포션) → 개수 감소 DB 잡
    void DoSwapSlot(DummyClient& c);                       // CS_SWAP_SLOT(인벤 슬롯끼리) → 슬롯 write-back
    void DoDeleteItem(DummyClient& c);                     // CS_DELETE_ITEM(인벤 슬롯). 설정에서 켤 때만
    void DoCleanupInventory(DummyClient& c);              // 인벤 4~39 순회 삭제(주운 아이템 청소 → insert 부하 지속)

    void PrintReport(double elapsedSec);                   // 최종 리포트 출력

    // 오류 카운트 + 파일 로그(LogClass)를 한 번에. fmt는 와이드 문자열.
    void ReportError(int cat, const DummyClient& c, const wchar_t* fmt, ...);

private:
    LoadConfig m_cfg;                 // 실행 설정(복사본)
    HANDLE     m_iocp = nullptr;      // 모든 소켓이 묶이는 IOCP 핸들

    std::unique_ptr<DummyClient[]> m_clients;  // 봇 배열. 재할당 없이 주소가 고정돼야 함(IOCP key로 포인터 사용)

    std::vector<std::thread> m_workers; // 수신 워커들
    std::thread m_behavior;             // 행동 스레드
    std::thread m_stats;                // 통계 스레드

    std::atomic<bool> m_running{ false };      // 메인 루프/스레드 계속 돌지
    std::atomic<bool> m_shuttingDown{ false }; // 종료 중(이때의 끊김은 서버 오류로 안 셈)

    // --- 집계 통계(여러 스레드가 증가하므로 atomic) ---
    std::atomic<int>       m_connected{ 0 };   // 현재 살아있는 접속 수
    std::atomic<int>       m_inField{ 0 };     // 현재 필드 진입 상태인 봇 수
    std::atomic<long long> m_connectOk{ 0 };     // connect() 성공 누적(게이지 m_connected와 별개)
    std::atomic<long long> m_resetPreField{ 0 }; // connect는 됐으나 스폰 전 리셋(백로그 풀/초기 거부 의심)
    std::atomic<long long> m_resetInField{ 0 };  // 이미 InField로 돌던 세션이 서버 리셋으로 끊김
    std::atomic<long long> m_sentPackets{ 0 }; // 보낸 패킷 총수
    std::atomic<long long> m_recvPackets{ 0 }; // 받은 패킷 총수
    std::atomic<long long> m_sentBytes{ 0 };   // 보낸 바이트 총량
    std::atomic<long long> m_recvBytes{ 0 };   // 받은 바이트 총량
    std::atomic<long long> m_errors[ERR_CAT_COUNT]; // 종류별 오류 카운트
    std::atomic<long long> m_reconnects{ 0 };  // 재접속 횟수
    std::atomic<long long> m_gameDeaths{ 0 };  // 봇이 게임 내에서 HP<=0 으로 죽은 횟수
    std::atomic<long long> m_respawns{ 0 };    // RESPAWN_RES_TO_ME 수신(부활 완료) 횟수
    std::atomic<long long> m_pickups{ 0 };     // 줍기 성공 수신 수(1030/1031)
    std::atomic<long long> m_skillOk{ 0 };     // 스킬 사용 성공 응답 수(1017 success=1)
    std::atomic<long long> m_skillFail{ 0 };   // 스킬 사용 실패 응답 수(1017 success=0, 쿨타임/마나)

    // 수신 패킷 종류별 카운트(확산 전송 분석용). Field 프로토콜 1000~1041 만 인덱싱.
    // 어떤 SC가 얼마나 많이 오는지 = fan-out(패킷 1개를 주변 N명에게 복제해 뿌리는 확산 전송)의 분포를 본다.
    static const uint16 FIELD_TYPE_BASE  = 1000;
    static const int    FIELD_TYPE_COUNT = 42;   // 1000..1041
    std::atomic<long long> m_recvByType[FIELD_TYPE_COUNT];

    // 현재 1초 창의 RTT 샘플(ms). m_rttMtx 로 보호.
    std::mutex          m_rttMtx;
    std::vector<double> m_rttSamples;

    static const char* ErrName(int cat);   // 오류 종류 → 사람이 읽는 이름
};
