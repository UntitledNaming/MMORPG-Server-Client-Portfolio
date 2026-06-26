#pragma once
#include "LatencyHistogram.h"  
#include "ContentsType.h"
#include "ContentsEnum.h"

struct MonitorSnapshot
{
    long long timestampNs;      // 모니터가 읽은 측정 시점
    int       concurrentUsers;  // 그 순간 동접수 (CSV x축)

    // ── 히스토그램 (라이브 그룹에서 SnapshotAndReset로 복사받을 자리) ──
    LatencyHistogram fieldRecv[(int)FieldRecvType::Max];
    LatencyHistogram fieldRecvLockEnter;
    LatencyHistogram fieldUpdate[(int)FrameProcType::Max];
    LatencyHistogram fieldUpdateLockEnter;
    LatencyHistogram fieldBroadcast[(int)BroadCastType::Max];
    LatencyHistogram authRecv[(int)AuthRecvType::Max];
    LatencyHistogram authRecvLockEnter;
    LatencyHistogram authUpdateLockEnter;
    LatencyHistogram authUpdateWhole;
    LatencyHistogram dbQueryProcTime[(int)DBJobCount::Max];
    LatencyHistogram dbProcTime[(int)DBProcType::Max];

    // 머신 관련 측정값
    float     totalCPUUsage;
    float     userCPUUsage;
    float     kernnelCPUUsage;
    

    // DB 관련 측정값
    long long dbJobTPS[(int)DBJobCount::Max];
    long long dbJobQueueCount;                 // 발산 신호

    long long storeQueueCount; 

    // 네트워크 관련 측정값
    long      sendIOTPS;
    long      recvIOTPS;
    double    tcpSegmentSend;
    double    tcpTransmit;

    uint64    syncCount;
    long      fieldFrame;
    long      authFrame;
    
};