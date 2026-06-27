//
// MMORPG 게임서버 부하/퍼즈 테스트 프로그램 - 진입점(main)
//
// 하는 일:
//   - 더미 클라이언트 N개를 띄운다(각각 characterUID 1..N 로 DB의 캐릭터에 1:1 대응).
//   - 각 봇에게 이동/공격/RTT핑 부하를 준다.
//   - 옵션으로 테스트 도중 일부 봇을 RST(강제 비정상 끊기)로 끊어 재접속 부하를 준다.
//   - 서버가 보내는 응답 패킷을 검사해 프로토콜/정합성 오류를 잡아낸다.
//   - 서버 코드는 전혀 수정하지 않는다(외부에서 진짜 클라처럼 붙기만 함).
//
#include <cstdio>            // printf
#include <string>           // std::string, std::stoi, std::getline
#include <iostream>         // std::cin
#include "LoadTestManager.h" // 테스트 전체를 운전하는 매니저 + LoadConfig 설정 구조체

// 이 파일 안에서만 쓰는 입력 도우미들. 다른 번역단위에서 안 보이게 익명 namespace로 가둠.
namespace
{
    // 콘솔에서 정수 한 줄을 읽는다. 그냥 Enter만 치면(빈 줄) 기본값 def 를 그대로 쓴다.
    int AskInt(const char* prompt, int def)
    {
        printf("%s [%d]: ", prompt, def);   // "질문 [기본값]: " 형태로 출력
        std::string line;
        std::getline(std::cin, line);       // 한 줄 통째로 읽기(공백 포함)
        if (line.empty()) return def;       // 빈 줄 → 기본값 유지
        try { return std::stoi(line); }     // 숫자로 변환 시도
        catch (...) { return def; }         // 숫자가 아니면(예외) 기본값으로 안전 처리
    }

    // 위와 같은데 문자열 버전(서버 IP 입력용). 빈 줄이면 기본값 유지.
    std::string AskStr(const char* prompt, const std::string& def)
    {
        printf("%s [%s]: ", prompt, def.c_str());
        std::string line;
        std::getline(std::cin, line);
        return line.empty() ? def : line;
    }
}

int main()
{
    printf("==== MMORPG GameServer Load Tester ====\n");
    printf("(press Enter to accept the [default] value)\n\n"); // Enter만 치면 대괄호 안 기본값 사용

    // 기본값이 채워진 설정 구조체. 아래에서 사용자 입력으로 하나씩 덮어쓴다.
    LoadConfig cfg;
    cfg.ip             = AskStr("Server IP", cfg.ip);                                 // 접속할 서버 IP
    cfg.port           = (uint16)AskInt("Server Port", cfg.port);                     // 접속할 포트(기본 11211)
    cfg.userCount      = AskInt("User count (characterUID 1..N)", cfg.userCount);     // 띄울 봇 수 = DB 캐릭터 1..N
    cfg.uidStart       = AskInt("characterUID start (machine split offset)", cfg.uidStart); // 이 인스턴스 시작 UID(머신 분할 시 겹침 방지)
    cfg.rstPercent     = AskInt("RST disconnect percent (0-100)", cfg.rstPercent);    // 도중에 RST로 끊을 비율(%)
    cfg.rampPerSec     = AskInt("Connect ramp (connections/sec)", cfg.rampPerSec);    // 초당 접속 개수(서버 accept가 직렬이라 한꺼번에 X)
    cfg.moveIntervalMs = AskInt("Move interval per bot (ms, 0=off)", cfg.moveIntervalMs);     // 봇당 이동 패킷 주기(ms)
    cfg.attackIntervalMs = AskInt("Attack interval per bot (ms, 0=off)", cfg.attackIntervalMs); // 봇당 공격 주기(ms)
    cfg.rttIntervalMs  = AskInt("RTT ping interval per bot (ms, 0=off)", cfg.rttIntervalMs);    // 봇당 RTT핑 주기(ms)
    cfg.skillIntervalMs    = AskInt("Skill use interval per bot (ms, 0=off)", cfg.skillIntervalMs);        // 봇당 스킬 사용 주기
    cfg.pickupIntervalMs   = AskInt("Pickup try interval per bot (ms, 0=off)", cfg.pickupIntervalMs);      // 봇당 줍기 시도 주기
    cfg.useItemIntervalMs  = AskInt("Use consumable interval per bot (ms, 0=off)", cfg.useItemIntervalMs); // 봇당 포션 사용 주기
    cfg.swapIntervalMs     = AskInt("Swap slot interval per bot (ms, 0=off)", cfg.swapIntervalMs);         // 봇당 슬롯 스왑 주기
    cfg.deleteItemIntervalMs = AskInt("Delete item interval per bot (ms, 0=off, destructive)", cfg.deleteItemIntervalMs); // 봇당 삭제 주기(파괴적, 기본 off)
    cfg.cleanupIntervalMs = AskInt("Inventory cleanup interval per bot (ms, 0=off)", cfg.cleanupIntervalMs); // 봇당 인벤 청소 주기(주운 아이템 삭제 → insert 부하 지속)
    cfg.rstAfterSec    = AskInt("First RST wave after (sec)", cfg.rstAfterSec);                 // 첫 RST 웨이브가 터지는 시점(초)
    cfg.churnIntervalSec = AskInt("Repeat RST wave every (sec, 0=single wave)", cfg.churnIntervalSec); // RST 웨이브 반복 주기(0=1회만)
    cfg.reconnectCooldownSec = AskInt("Reconnect RST'd bots after (sec, 0=no reconnect)", cfg.reconnectCooldownSec); // 끊긴 봇 재접속까지 쿨다운(0=재접속 안 함)
    cfg.durationSec    = AskInt("Total test duration (sec)", cfg.durationSec);                  // 전체 테스트 시간(초)

    // 봇이 0명 이하면 할 일이 없으니 즉시 종료.
    if (cfg.userCount <= 0)
    {
        printf("user count must be > 0\n");
        return 1;
    }

    printf("\n");

    // 설정대로 매니저를 만들고 초기화(WSAStartup + IOCP + 워커 스레드 생성).
    LoadTestManager mgr(cfg);
    if (!mgr.Init())
        return 1;   // 초기화 실패면 종료

    // 실제 테스트 수행: 램프 접속 → 시뮬레이션 → RST 웨이브 → 정리 → 리포트 출력.
    mgr.Run();

    // 콘솔이 바로 닫히지 않게 Enter 대기(리포트를 눈으로 확인할 시간).
    printf("\ndone. press Enter to exit.\n");
    std::string dummy;
    std::getline(std::cin, dummy);
    return 0;
}
