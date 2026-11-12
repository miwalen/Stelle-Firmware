#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <cstddef>
#include <cstdint>

#include "HardwareSerial.h"
#include "Stelle.h"

#define HEADER_1 0xAA
#define HEADER_2 0x55

namespace stelle {

// Di Arduino parameter port bertipe Stream&. Di sini HardwareSerial&, karena
// HardwareSerial sudah menyediakan available(), read(), write() seperti Stream.
bool sendFrame(HardwareSerial& port, uint8_t cmd, const uint8_t* payload, uint8_t len);
bool sendFrame(HardwareSerial& port, Cmd cmd, const uint8_t* payload, uint8_t len);
bool sendFrame(HardwareSerial& port, Cmd cmd);

void executeCommand(HardwareSerial& port, Cmd cmd, uint8_t* payload, uint8_t len);

void TaskSerialParse(void* pvParameters);
void initTaskSerialComA();
void initTaskSerialUSB();

} // namespace stelle

#endif // COMMUNICATION_H
