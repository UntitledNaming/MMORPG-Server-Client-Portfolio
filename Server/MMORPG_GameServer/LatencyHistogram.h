#pragma once
#include <cstdint>
#include <windows.h>
#include <intrin.h>   // _BitScanReverse64

/////////////////////////////////////////////
// 부하 탐지 측정값 저장 위한 클래스
//  - 지연/처리시간 같은 "분포" 지표 전용
//    (TPS 같은 단순 카운트류는 InterlockedIncrement + 1초 차분으로 따로 처리)
//  - 측정값 단위는 ns(나노초). 로그스케일 버킷에 "개수"만 카운팅

/////////////////////////////////////////////

class LatencyHistogram
{
public:
    LatencyHistogram() = default;
    ~LatencyHistogram() = default;

    // 버킷 개수. 마지막 칸(BUCKET_COUNT-1)은 그 이상을 다 받는 오버플로 통합칸
    static const int BUCKET_COUNT = 30;

    //  버킷 0 = [0, 2^SHIFT) ns 통합칸.
    //  시계 해상도(QPC ~100ns) 미만은 변별 불가라 한 칸에 몰아도 무손실.
    //  SHIFT=6 →0번칸은 [0,64ns), 무경합/즉시처리 베이스라인 대표칸.
    static const int SHIFT = 6;

    // ns 값 →버킷 인덱스. idx = floor(log2(ns)) - SHIFT + 1, [0, BUCKET_COUNT-1]로 자름(clamp)
    static int Index(unsigned long long ns)
    {
        if (ns == 0)
            return 0;

        unsigned long msb = 0;
        _BitScanReverse64(&msb, ns);   // 최상위 1비트 위치 = floor(log2(ns))

        long idx = (long)msb - SHIFT + 1;
        if (idx < 0)
            idx = 0;
        if (idx >= BUCKET_COUNT)
            idx = BUCKET_COUNT - 1;
        return (int)idx;
    }

    // 버킷 i가 담는 ns 구간의 하한값. 버킷 i(>=1)는 [2^(i+SHIFT-1), 2^(i+SHIFT)) 담음
    static unsigned long long BucketLowerBound(int i)
    {
        if (i == 0)
            return 0;
        return 1ULL << (i + SHIFT - 1);
    }

    // 측정값 1건 기록 (측정 스레드가 hot path에서 호출, O(1) 비트연산)
    void Record(unsigned long long ns)
    {
        InterlockedIncrement64(&hist[Index(ns)]);
        InterlockedIncrement64(&count);
    }

    // 누적 개수로 p백분위가 속한 버킷의 하한 ns를 근사 반환 (2배 간격 →약 2배 오차)
    unsigned long long Percentile(double p) const
    {
        if (count == 0)
            return 0;

        unsigned long long target = (unsigned long long)(count * p / 100.0);  // p번째가 몇 번째 표본인가
        unsigned long long cumulative = 0;
        for (int i = 0; i < BUCKET_COUNT; ++i)
        {
            cumulative += hist[i];     // 0번 칸부터 개수를 쌓아감
            if (cumulative >= target)                         // 쌓은 게 target을 넘는 첫 칸 =
                return BucketLowerBound(i);                   // p번째 표본이 사는 칸 →ns로 환산
        }                                                    
        return BucketLowerBound(BUCKET_COUNT - 1);            // (max 빼니까 끝 칸 하한으로. 사실상 도달 안 함)
    }

    // 모니터 스레드가 1초마다 현재값 복사 + 0으로 리셋(다음 구간 새로 누적)
    void SnapshotAndReset(LatencyHistogram& out)
    {
        out.count = count;
        count = 0;

        for (int i = 0; i < BUCKET_COUNT; i++)
        {
            out.hist[i] = hist[i];
            hist[i] = 0;
        }
    }

    long long GetCount() const { return count; }

private:
    long long hist[LatencyHistogram::BUCKET_COUNT] = {};
    long long count = 0;
};

