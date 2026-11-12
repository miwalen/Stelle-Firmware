#ifndef SETUP_H
#define SETUP_H

#include "ConfigManager.h"
#include "HardwareSerial.h"
#include "Logger.h"
#include "MechLib.h"
#include "PowerManager.h"
#include "TwoWire.h"

namespace stelle {

// ============================================================
// Global Objects / Instances Sistem
// ============================================================
extern TwoWire Wire1;
extern HardwareSerial SerialComA;
extern HardwareSerial Serial;

extern KinematicChain Kinematics;
extern InputChannels channels;
extern ConfigManager config;
extern PowerState currentPowerState;
extern Logger log1;

// Entrypoint inisialisasi sistem firmware
void setupSystem();

} // namespace stelle

#endif // SETUP_H
