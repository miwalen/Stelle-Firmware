#ifndef HARDWARESERIAL_H
#define HARDWARESERIAL_H

#include <cstddef>
#include <cstdint>

#include "driver/uart.h"

namespace stelle {

// Pengganti HardwareSerial ESP-IDF.
//
// API sama seperti HardwareSerial Arduino: begin(baud, config, rxPin, txPin),
// available(), read(), write(), setRxBufferSize(), flush(), end().
//
//   HardwareSerial SerialComA(1);
//   SerialComA.begin(115200, SERIAL_8N1, 16, 17);
class HardwareSerial {
public:
    explicit HardwareSerial(int uartNum);
    ~HardwareSerial();

    HardwareSerial(const HardwareSerial&) = delete;
    HardwareSerial& operator=(const HardwareSerial&) = delete;

    // Harus dipanggil sebelum begin(), sama seperti Arduino.
    void setRxBufferSize(size_t size);

    bool begin(unsigned long baud, uint32_t config = SERIAL_8N1,
               int8_t rxPin = -1, int8_t txPin = -1);
    void end();

    int available();
    int read();
    int peek();
    size_t write(uint8_t value);
    size_t write(const uint8_t* data, size_t len);
    void flush();

    operator bool() const { return _begun; }

private:
    int _uartNum;
    size_t _rxBufferSize = 256;
    bool _begun = false;
};

// Konstanta konfigurasi frame, nilainya mengikuti UART Arduino.
constexpr uint32_t SERIAL_8N1 = 0x800001c;

} // namespace stelle

#endif // HARDWARESERIAL_H
