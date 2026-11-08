#ifndef MUTEXES_H
#define MUTEXES_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "Logger.h"
// ==========================================
// MUTEX
// ==========================================
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
// ==========================================
// INITIALIZATION
// ==========================================

void initMutexes();

#endif // MUTEXES_H