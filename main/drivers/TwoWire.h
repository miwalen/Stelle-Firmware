#ifndef TWOWIRE_H
#define TWOWIRE_H

#include <cstddef>
#include <cstdint>

namespace stelle {

// Pengganti TwoWire / Wire ESP-IDF.
//
// API sama seperti Wire.h Arduino: begin(), beginTransmission(),
// write(), endTransmission(), requestFrom(), read(), available().
// Bedanya hanya di dalam: memakai driver i2c_master.
//
//   Wire1.begin(25, 26);            // SDA=25, SCL=26
//   Wire1.beginTransmission(0x48);
//   Wire1.write(0x01);
//   Wire1.endTransmission();
class TwoWire {
public:
    TwoWire() = default;
    ~TwoWire();

    TwoWire(const TwoWire&) = delete;
    TwoWire& operator=(const TwoWire&) = delete;

    // pinSda / pinScl diizinkan -1 untuk memakai pin yang sudah dikonfigurasi.
    bool begin(int pinSda = -1, int pinScl = -1, uint32_t frequency = 100000);
    void end();

    bool isReady() const { return _dev != nullptr || _bus != nullptr; }

    // Transmit buffer
    void beginTransmission(uint8_t address);
    size_t write(uint8_t value);
    size_t write(const uint8_t* data, size_t len);
    uint8_t endTransmission(bool sendStop = true);

    // Receive
    uint8_t requestFrom(uint8_t address, uint8_t quantity, bool sendStop = true);
    int available() const { return _rxLength - _rxIndex; }
    int read();
    int peek() const;

private:
    bool ensureDevice(uint8_t address);

    struct BusHandle;
    BusHandle* _bus = nullptr;
    void* _dev = nullptr;

    uint8_t _address = 0;
    uint8_t _txBuffer[64] = {};
    uint8_t _txLength = 0;

    uint8_t _rxBuffer[64] = {};
    uint8_t _rxLength = 0;
    uint8_t _rxIndex = 0;
};



} // namespace stelle

#endif // TWOWIRE_H
