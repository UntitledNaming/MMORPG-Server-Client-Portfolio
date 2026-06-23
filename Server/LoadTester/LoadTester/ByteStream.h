#pragma once
//
// 부하 테스터 전용의 아주 작은 자체 패킷 직렬화/역직렬화기.
// 서버가 기대하는 바이트 배치를 그대로 흉내 낸다:
//   [ GAMELIB_LANHEADER (5바이트, pack1) ] [ 페이로드 ]
//   페이로드 = [ uint16 패킷타입 ] [ 필드들... ] (그냥 리틀엔디언 raw)
//
// 일부러 서버의 CMessage / 메모리풀을 안 끌어온다:
//   의존성을 없애 도구를 단독으로 가볍게 유지하기 위함. Common\ 의 프로토콜 상수 헤더만 공유.
//
#include <cstring>              // memcpy
#include "ContentsType.h"       // Common\Contents : uint8/16/32/64 타입 정의
#include "NetPacketHeader.h"    // Common\Network  : GAMELIB_LANHEADER, ERouteType, ServiceID

// ---- Writer(송신용) -------------------------------------------------------
// 앞쪽 5바이트(헤더)를 미리 비워두고 페이로드를 이어 붙인 뒤, Data()에서 헤더를 채워 넣는다.
// 보내는 패킷은 다 작아서 1KB 스택 버퍼면 충분하다.
class PacketWriter
{
public:
    // 생성과 동시에 헤더 자리를 예약하고, 페이로드 맨 앞에 패킷 타입(uint16)을 박는다.
    explicit PacketWriter(uint16 packetType)
        : m_size(sizeof(GAMELIB_LANHEADER))   // 헤더 크기만큼 자리를 비워둔 채 시작
    {
        PutU16(packetType);                    // 페이로드 첫 2바이트 = 패킷 타입 id
    }

    // 각 타입별 append 헬퍼(전부 raw 바이트로 그대로 복사).
    void PutU8(uint8 v) { Append(&v, 1); }
    void PutU16(uint16 v) { Append(&v, 2); }
    void PutU32(uint32 v) { Append(&v, 4); }
    void PutU64(uint64 v) { Append(&v, 8); }
    void PutI16(int16 v) { Append(&v, 2); }
    void PutI32(int32 v) { Append(&v, 4); }
    void PutFloat(float v) { Append(&v, 4); }
    void PutBytes(const void* p, int n) { Append(p, n); } // 토큰 같은 고정 바이트 묶음용

    // 비워뒀던 헤더(페이로드 크기/라우트/서비스)를 채우고 버퍼 시작 포인터를 돌려준다.
    const char* Data()
    {
        GAMELIB_LANHEADER h;
        h.s_len = static_cast<uint16>(m_size - sizeof(GAMELIB_LANHEADER)); // 헤더 뺀 페이로드 길이만
        h.s_serviceID = ServiceID::NONE_SERVICE;                           // 서비스 구분 안 씀
        h.s_routeType = static_cast<uint8>(ERouteType::GROUP);             // 그룹 라우팅(필드그룹으로)
        memcpy(m_buf, &h, sizeof(h));                                      // 맨 앞 5바이트에 헤더 기록
        return m_buf;
    }
    int Length() const { return m_size; }   // 헤더+페이로드 = 실제로 send() 할 총 바이트

private:
    // 버퍼 끝에 n바이트 이어붙이기. 1KB를 넘길 일은 없지만 안전상 넘으면 그냥 무시.
    void Append(const void* p, int n)
    {
        if (m_size + n > (int)sizeof(m_buf)) return; // 송신 패킷은 1KB 근처도 안 감
        memcpy(m_buf + m_size, p, n);
        m_size += n;
    }
    char m_buf[1024];   // 헤더+페이로드 누적 버퍼
    int  m_size;        // 현재까지 쓴 바이트 수
};

// ---- Reader(수신용) -------------------------------------------------------
// 페이로드 하나(헤더는 이미 떼어낸 상태)를 경계검사하며 읽는다.
// 끝을 넘어 읽으려 하면 m_ok 가 false 로 바뀌고, 호출부는 그걸 보고 길이/프로토콜 오류로 잡는다.
class PacketReader
{
public:
    PacketReader(const char* p, int len)
        : m_p(p), m_len(len), m_pos(0), m_ok(true) {}

    // 타입별 읽기. 끝을 넘으면 0/기본값을 돌려주고 m_ok를 내린다.
    uint8  GetU8() { return Get<uint8>(); }
    uint16 GetU16() { return Get<uint16>(); }
    uint32 GetU32() { return Get<uint32>(); }
    uint64 GetU64() { return Get<uint64>(); }
    int16  GetI16() { return Get<int16>(); }
    int32  GetI32() { return Get<int32>(); }
    float  GetFloat() { return Get<float>(); }

    // n바이트 건너뛰기(안 읽고 넘길 때). 범위 넘으면 m_ok 내림.
    void Skip(int n) { if (m_pos + n > m_len) m_ok = false; else m_pos += n; }

    bool Ok()     const { return m_ok; }            // 지금까지 읽기가 전부 정상이었나
    int  Remain() const { return m_len - m_pos; }   // 아직 안 읽은 남은 바이트(0이어야 깔끔)

private:
    // 실제 읽기 본체. 경계를 넘으면 false 처리 후 기본값 반환.
    template<class T> T Get()
    {
        T v{};
        if (m_pos + (int)sizeof(T) > m_len) { m_ok = false; return v; } // 끝 초과 → 오류 표시
        memcpy(&v, m_p + m_pos, sizeof(T));
        m_pos += sizeof(T);
        return v;
    }
    const char* m_p;   // 페이로드 시작
    int m_len;         // 페이로드 길이
    int m_pos;         // 현재 읽기 위치
    bool m_ok;         // 경계 위반이 한 번이라도 있었는지
};
