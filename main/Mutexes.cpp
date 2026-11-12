#include "Mutexes.h"

SemaphoreHandle_t mutexPowerState  = NULL;
SemaphoreHandle_t mutexJoints = NULL;
SemaphoreHandle_t mutexChannels = NULL;
SemaphoreHandle_t mutexLog = NULL;
SemaphoreHandle_t mutexWifiSSID = NULL;
SemaphoreHandle_t mutexWifiPassword = NULL;
SemaphoreHandle_t mutexBtName = NULL;
SemaphoreHandle_t mutexBtPassword = NULL;
SemaphoreHandle_t mutexUseIMU = NULL;
SemaphoreHandle_t mutexIMUQuaternionOffset = NULL;
SemaphoreHandle_t mutexQuaternionOffset = NULL;
SemaphoreHandle_t mutexVector3Offset = NULL;
SemaphoreHandle_t mutexSaveConfig = NULL;

void initMutexes() {

    // Mutex umum
    mutexPowerState   = xSemaphoreCreateMutex();
    mutexJoints   = xSemaphoreCreateMutex();
    mutexChannels = xSemaphoreCreateMutex();
    mutexLog      = xSemaphoreCreateMutex();
    mutexSaveConfig      = xSemaphoreCreateMutex();
    mutexWifiSSID              = xSemaphoreCreateMutex();
    mutexWifiPassword          = xSemaphoreCreateMutex();
    mutexBtName                = xSemaphoreCreateMutex();
    mutexBtPassword            = xSemaphoreCreateMutex();
    mutexUseIMU                = xSemaphoreCreateMutex();
    mutexIMUQuaternionOffset   = xSemaphoreCreateMutex();
    mutexQuaternionOffset      = xSemaphoreCreateMutex();
    mutexVector3Offset         = xSemaphoreCreateMutex();
}
