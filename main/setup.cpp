#include "setup.h"

#include "Communication.h"
#include "ForwardKinematicsComputer.h"
#include "Mutexes.h"
#include "SensorReader.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

static const char* TAG = "setup";

namespace stelle {

// ============================================================
// Definisi Objek Global Sistem (Sentralisasi)
// ============================================================
TwoWire Wire1;
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

    // 1. Inisialisasi NVS Flash
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // 2. Inisialisasi Sinkronisasi & Mutex
    initMutexes();

    // 3. Inisialisasi Logger & Power
    log1.begin();
    currentPowerState.begin();
    initTaskPowerKeeper();

    // 4. Inisialisasi Bus Peripheral Hardware (I2C SDA=25, SCL=26)
    Wire1.begin(25, 26);

    // 5. Muat Konfigurasi dari NVS menggunakan Preferences global
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
