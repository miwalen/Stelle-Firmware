#ifndef Packets_H
#define Packets_H

#include <Arduino.h>
#include "GeoLib.h"
#include "MechLib.h"

// ============================================================
// Struct Wire-Format (Internal/Layout Kabel)
// ============================================================
struct Vector3Data {
    float x, y, z;
};

struct QuaternionData {
    float w, x, y, z;
};

struct InputChannelsData {
    float ch[InputChannels::SIZE];
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
        c.set(data.ch);
        return c;
    }
};

#endif // Packets_H