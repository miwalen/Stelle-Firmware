#pragma once

#include <cstdint>
#include <cstring>

#include "GeoLib.h"
#include "MechLib.h"

namespace stelle {

// ============================================================
// Opcode — satu sumber kebenaran untuk kedua ujung link
// ============================================================
enum class Cmd : uint8_t {
    Ping             = 0x01,
    SetQuatOffset    = 0x10,
    SetVec3Offset    = 0x11,
    SetUseImu        = 0x12,
    SetWifiSsid      = 0x13,
    SetWifiPassword  = 0x14,
    SetBtName        = 0x15,
    SetBtPassword    = 0x16,
    SetImuQuatOffset = 0x17,
    GetJointPos      = 0x20,  // int32 index -> Vector3Data
    GetJointQuat     = 0x21,  // int32 index -> QuaternionData
    GetJointAngleDeg = 0x22,  // int32 index -> float
    GetJointAngleRad = 0x23,  // int32 index -> float
    GetJointCount    = 0x24,  // -> u8
    SetChannels      = 0x30,
    GetChannels      = 0x31,
    GetLogUnread     = 0x40,  // -> 0x99 + teks
    GetLogAllHistory = 0x41,  // -> 0x99 + teks
    PowerOff         = 0x50,

    ReplyLog         = 0x99,  // balasan 0x40 / 0x41 (bukan perintah masuk)
};

// ============================================================
// Struct Wire-Format (Internal/Layout Kabel)
// ============================================================
struct Vector3Data {
    float x, y, z;
};

struct QuaternionData {
    float w, x, y, z;
};

constexpr int kInputChannelCount = InputChannels::SIZE;

struct InputChannelsData {
    float ch[kInputChannelCount];
};

// ============================================================
// Vector3Packet
// ============================================================
class Vector3Packet {
private:
    Vector3Data data;

public:
    void pack(Vector3 v) {
        data.x = v.x;
        data.y = v.y;
        data.z = v.z;
    }

    // Salin struct internal langsung ke buffer byte luar
    void getPacket(uint8_t* buffer) {
        memcpy(buffer, &data, sizeof(Vector3Data));
    }

    // Salin buffer byte luar langsung ke struct internal
    void setPacket(const uint8_t* buffer) {
        memcpy(&data, buffer, sizeof(Vector3Data));
    }

    size_t size() {
        return sizeof(Vector3Data);
    }

    Vector3 unpack() {
        return Vector3(data.x, data.y, data.z);
    }
};

// ============================================================
// QuaternionPacket
// ============================================================
class QuaternionPacket {
private:
    QuaternionData data;

public:
    void pack(Quaternion q) {
        data.w = q.w;
        data.x = q.x;
        data.y = q.y;
        data.z = q.z;
    }

    void getPacket(uint8_t* buffer) {
        memcpy(buffer, &data, sizeof(QuaternionData));
    }

    void setPacket(const uint8_t* buffer) {
        memcpy(&data, buffer, sizeof(QuaternionData));
    }

    size_t size() {
        return sizeof(QuaternionData);
    }

    Quaternion unpack() {
        return Quaternion(data.w, data.x, data.y, data.z);
    }
};

// ============================================================
// InputChannelsPacket
// ============================================================
class InputChannelsPacket {
private:
    InputChannelsData data;

public:
    void pack(InputChannels c) {
        for (int i = 0; i < InputChannels::SIZE; i++) {
            data.ch[i] = c.getChannel(i);
        }
    }

    void getPacket(uint8_t* buffer) {
        memcpy(buffer, &data, sizeof(InputChannelsData));
    }

    void setPacket(const uint8_t* buffer) {
        memcpy(&data, buffer, sizeof(InputChannelsData));
    }

    size_t size() {
        return sizeof(InputChannelsData);
    }

    InputChannels unpack() {
        InputChannels c;
        // Feed the wire values straight into the channel buffer.
        for (int i = 0; i < InputChannels::SIZE; i++) {
            c.set(i, data.ch[i]);
        }
        return c;
    }
};

// ============================================================
// Panjang payload masuk per perintah — tabel ini yang dipakai parser.
// Tidak ada length byte di wire, jadi tabel inilah kontraknya.
// ============================================================
constexpr uint8_t PayloadLenForCommand(Cmd cmd) {
    switch (cmd) {
        case Cmd::Ping:             return 0;
        case Cmd::SetQuatOffset:    return sizeof(QuaternionData);
        case Cmd::SetVec3Offset:    return sizeof(Vector3Data);
        case Cmd::SetUseImu:        return 1;
        case Cmd::SetWifiSsid:
        case Cmd::SetWifiPassword:
        case Cmd::SetBtName:
        case Cmd::SetBtPassword:    return 32;
        case Cmd::SetImuQuatOffset: return sizeof(QuaternionData);
        case Cmd::GetJointPos:
        case Cmd::GetJointQuat:
        case Cmd::GetJointAngleDeg:
        case Cmd::GetJointAngleRad: return sizeof(int32_t);
        case Cmd::GetJointCount:    return 0;
        case Cmd::SetChannels:      return sizeof(InputChannelsData);
        case Cmd::GetChannels:      return 0;
        case Cmd::GetLogUnread:
        case Cmd::GetLogAllHistory: return 0;
        case Cmd::PowerOff:         return 0;
        default:                    return 0;
    }
}

} // namespace stelle