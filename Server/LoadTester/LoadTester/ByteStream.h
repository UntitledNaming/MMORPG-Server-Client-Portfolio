#pragma once
//
// Minimal, self-contained packet (de)serializer for the load tester.
// It mirrors exactly what the server expects:
//   [ GAMELIB_LANHEADER (5 bytes, packed) ] [ payload ]
//   payload = [ uint16 packetType ] [ fields... ] (raw little-endian)
//
// We DO NOT reuse the server's CMessage / memory pools on purpose:
// keeping this dependency-free makes the tool standalone and bug-light.
// Only the protocol constant headers from Common\ are shared.
//
#include <cstring>
#include "ContentsType.h"       // Common\Contents : uint8/16/32/64 typedefs
#include "NetPacketHeader.h"    // Common\Network  : GAMELIB_LANHEADER, ERouteType, ServiceID

// ---- Writer ---------------------------------------------------------------
// Reserves the 5-byte header up front, appends payload, then Finalize() fills
// the header in place. Outgoing packets are tiny so a 1KB stack buffer is plenty.
class PacketWriter
{
public:
    explicit PacketWriter(uint16 packetType)
        : m_size(sizeof(GAMELIB_LANHEADER))   // reserve header room
    {
        PutU16(packetType);                    // payload starts with the type id
    }

    void PutU8(uint8 v) { Append(&v, 1); }
    void PutU16(uint16 v) { Append(&v, 2); }
    void PutU32(uint32 v) { Append(&v, 4); }
    void PutU64(uint64 v) { Append(&v, 8); }
    void PutI16(int16 v) { Append(&v, 2); }
    void PutI32(int32 v) { Append(&v, 4); }
    void PutFloat(float v) { Append(&v, 4); }
    void PutBytes(const void* p, int n) { Append(p, n); }

    // Fills the header (payload size / route / service) and returns the buffer.
    const char* Data()
    {
        GAMELIB_LANHEADER h;
        h.s_len = static_cast<uint16>(m_size - sizeof(GAMELIB_LANHEADER)); // payload only
        h.s_serviceID = ServiceID::NONE_SERVICE;
        h.s_routeType = static_cast<uint8>(ERouteType::GROUP);
        memcpy(m_buf, &h, sizeof(h));
        return m_buf;
    }
    int Length() const { return m_size; }   // header + payload, what we send()

private:
    void Append(const void* p, int n)
    {
        if (m_size + n > (int)sizeof(m_buf)) return; // outgoing never gets near 1KB
        memcpy(m_buf + m_size, p, n);
        m_size += n;
    }
    char m_buf[1024];
    int  m_size;
};

// ---- Reader ---------------------------------------------------------------
// Bounds-checked reader over a single payload (header already stripped).
// If any read would run past the end, m_ok flips to false -> caller flags a
// protocol/length error.
class PacketReader
{
public:
    PacketReader(const char* p, int len)
        : m_p(p), m_len(len), m_pos(0), m_ok(true) {}

    uint8  GetU8() { return Get<uint8>(); }
    uint16 GetU16() { return Get<uint16>(); }
    uint32 GetU32() { return Get<uint32>(); }
    uint64 GetU64() { return Get<uint64>(); }
    int16  GetI16() { return Get<int16>(); }
    int32  GetI32() { return Get<int32>(); }
    float  GetFloat() { return Get<float>(); }

    void Skip(int n) { if (m_pos + n > m_len) m_ok = false; else m_pos += n; }

    bool Ok()     const { return m_ok; }
    int  Remain() const { return m_len - m_pos; }

private:
    template<class T> T Get()
    {
        T v{};
        if (m_pos + (int)sizeof(T) > m_len) { m_ok = false; return v; }
        memcpy(&v, m_p + m_pos, sizeof(T));
        m_pos += sizeof(T);
        return v;
    }
    const char* m_p;
    int m_len;
    int m_pos;
    bool m_ok;
};
