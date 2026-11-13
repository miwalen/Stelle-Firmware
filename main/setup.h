#ifndef SETUP_H
#define SETUP_H

#include "ConfigManager.h"
#include "HardwareSerial.h"
#include "Logger.h"
#include "MechLib.h"
#include "PowerManager.h"
#include "Preferences.h"
#include "TwoWire.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace stelle {

// ============================================================
// Global Mutex Handles
// ============================================================
extern SemaphoreHandle_t mutexPowerState;
extern SemaphoreHandle_t mutexJoints;
extern SemaphoreHandle_t mutexChannels;
extern SemaphoreHandle_t mutexLog;
extern SemaphoreHandle_t mutexWifiSSID;
extern SemaphoreHandle_t mutexWifiPassword;
extern SemaphoreHandle_t mutexBtName;
extern SemaphoreHandle_t mutexBtPassword;
extern SemaphoreHandle_t mutexUseIMU;
extern SemaphoreHandle_t mutexIMUQuaternionOffset;
extern SemaphoreHandle_t mutexQuaternionOffset;
extern SemaphoreHandle_t mutexVector3Offset;
extern SemaphoreHandle_t mutexSaveConfig;

// ============================================================
// Global Objects / Instances Sistem
// ============================================================
extern TwoWire Wire1;
extern HardwareSerial SerialComA;
extern HardwareSerial Serial;
extern Preferences prefs;

extern KinematicChain Kinematics;
extern InputChannels channels;
extern ConfigManager config;
extern PowerState currentPowerState;
extern Logger log1;

// Entrypoint inisialisasi sistem firmware
void setupSystem();

} // namespace stelle

#endif // SETUP_H
