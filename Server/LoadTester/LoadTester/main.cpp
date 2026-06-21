//
// MMORPG GameServer - Load / Fuzz Tester
//
// Spawns N dummy clients (characterUID 1..N), drives movement/attack/RTT load,
// optionally RSTs a % of them mid-test, and validates the server's response
// stream for protocol/consistency errors. Server code is NOT modified.
//
#include <cstdio>
#include <string>
#include <iostream>
#include "LoadTestManager.h"

namespace
{
    // Reads a line; empty input keeps the default.
    int AskInt(const char* prompt, int def)
    {
        printf("%s [%d]: ", prompt, def);
        std::string line;
        std::getline(std::cin, line);
        if (line.empty()) return def;
        try { return std::stoi(line); }
        catch (...) { return def; }
    }
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
    printf("(press Enter to accept the [default] value)\n\n");

    LoadConfig cfg;
    cfg.ip             = AskStr("Server IP", cfg.ip);
    cfg.port           = (uint16)AskInt("Server Port", cfg.port);
    cfg.userCount      = AskInt("User count (characterUID 1..N)", cfg.userCount);
    cfg.rstPercent     = AskInt("RST disconnect percent (0-100)", cfg.rstPercent);
    cfg.rampPerSec     = AskInt("Connect ramp (connections/sec)", cfg.rampPerSec);
    cfg.moveIntervalMs = AskInt("Move interval per bot (ms, 0=off)", cfg.moveIntervalMs);
    cfg.attackIntervalMs = AskInt("Attack interval per bot (ms, 0=off)", cfg.attackIntervalMs);
    cfg.rttIntervalMs  = AskInt("RTT ping interval per bot (ms, 0=off)", cfg.rttIntervalMs);
    cfg.rstAfterSec    = AskInt("First RST wave after (sec)", cfg.rstAfterSec);
    cfg.churnIntervalSec = AskInt("Repeat RST wave every (sec, 0=single wave)", cfg.churnIntervalSec);
    cfg.reconnectCooldownSec = AskInt("Reconnect RST'd bots after (sec, 0=no reconnect)", cfg.reconnectCooldownSec);
    cfg.durationSec    = AskInt("Total test duration (sec)", cfg.durationSec);

    if (cfg.userCount <= 0)
    {
        printf("user count must be > 0\n");
        return 1;
    }

    printf("\n");

    LoadTestManager mgr(cfg);
    if (!mgr.Init())
        return 1;

    mgr.Run();

    printf("\ndone. press Enter to exit.\n");
    std::string dummy;
    std::getline(std::cin, dummy);
    return 0;
}
