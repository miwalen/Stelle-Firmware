#ifndef STELLE_H
#define STELLE_H

#include "ADS1115.h"
#include "ConfigManager.h"
#include "HardwareSerial.h"
#include "Logger.h"
#include "MechLib.h"
#include "Packets.h"
#include "PowerManager.h"
#include "TwoWire.h"

// Deklarasi objek global, sama seperti blok di bagian atas Stelle.ino.
// Definisi ada di Stelle.cpp.
namespace stelle {

// Arduino: HardwareSerial SerialComA(1);
extern HardwareSerial SerialComA;

// Arduino: objek `Serial` (USB-Serial/JTAG di board ini).
extern HardwareSerial Serial;

extern ADS1115 ads1;
extern ADS1115 ads2;

extern KinematicChain Kinematics;
extern InputChannels channels;

extern Vector3 offsetPos;
extern Quaternion offsetRot;
extern Quaternion offsetRotIMU;

} // namespace stelle

#endif // STELLE_H
