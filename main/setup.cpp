#include "setup.h"

static const char* TAG = "setup";

namespace stelle {

// ============================================================
// Definisi Mutex Global Sistem (Sentralisasi)
// ============================================================
SemaphoreHandle_t mutexPowerState        = NULL;
SemaphoreHandle_t mutexJoints            = NULL;
SemaphoreHandle_t mutexChannels          = NULL;
SemaphoreHandle_t mutexLog               = NULL;
SemaphoreHandle_t mutexWifiSSID          = NULL;
SemaphoreHandle_t mutexWifiPassword      = NULL;
SemaphoreHandle_t mutexBtName            = NULL;
SemaphoreHandle_t mutexBtPassword        = NULL;
SemaphoreHandle_t mutexUseIMU            = NULL;
SemaphoreHandle_t mutexIMUQuaternionOffset = NULL;
SemaphoreHandle_t mutexQuaternionOffset  = NULL;
SemaphoreHandle_t mutexVector3Offset     = NULL;
SemaphoreHandle_t mutexSaveConfig        = NULL;

// ============================================================
// Definisi Objek Global Sistem (Sentralisasi)
// ============================================================
TwoWire Wire1;
ADS1115 ads1;
ADS1115 ads2;
HardwareSerial SerialComA(1);
HardwareSerial Serial(0);
Preferences prefs;

KinematicChain Kinematics;
InputChannels channels;
ConfigManager config;
PowerState currentPowerState;
Logger log1;

void setupSystem() {
    ESP_LOGI(TAG, "Initializing Stelle ESP-IDF Firmware...");

    // 1. Inisialisasi Sinkronisasi & Mutex
    mutexPowerState          = xSemaphoreCreateMutex();
    mutexJoints              = xSemaphoreCreateMutex();
    mutexChannels            = xSemaphoreCreateMutex();
    mutexLog                 = xSemaphoreCreateMutex();
    mutexSaveConfig          = xSemaphoreCreateMutex();
    mutexWifiSSID            = xSemaphoreCreateMutex();
    mutexWifiPassword        = xSemaphoreCreateMutex();
    mutexBtName              = xSemaphoreCreateMutex();
    mutexBtPassword          = xSemaphoreCreateMutex();
    mutexUseIMU              = xSemaphoreCreateMutex();
    mutexIMUQuaternionOffset = xSemaphoreCreateMutex();
    mutexQuaternionOffset    = xSemaphoreCreateMutex();
    mutexVector3Offset       = xSemaphoreCreateMutex();

    // 2. Inisialisasi Logger & Power
    log1.begin();
    currentPowerState.begin();
    initTaskPowerKeeper();

    // 3. Inisialisasi Bus Peripheral Hardware (I2C SDA=25, SCL=26)
    Wire1.begin(25, 26);
    ads1.begin(Wire1, ADS1115::ADDR_GND);
    ads2.begin(Wire1, ADS1115::ADDR_SDA);
    // 4. Muat Konfigurasi dari NVS menggunakan Preferences global
    config.begin(&prefs);

    // 6. Jalankan FreeRTOS Tasks
    initTaskSerialComA();
    initTaskSerialUSB();
    initTaskForwardKinematicsComputer();
    initTaskSensorReader();

    ESP_LOGI(TAG, "All background tasks spawned successfully.");
}

} // namespace stelle

extern "C" void app_main(void) {
    stelle::setupSystem();

    // app_main selesai tugasnya setelah inisialisasi;
    // task FreeRTOS independen tetap berjalan di background.
    vTaskDelete(NULL);
}
