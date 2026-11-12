// Pengganti Stelle.ino: setup() + loop() jadi app_main().

#include "Stelle.h"

#include "Communication.h"
#include "ForwardKinematicsComputer.h"
#include "Mutexes.h"
#include "SensorReader.h"
#include "nvs_flash.h"

namespace stelle {

// ============================================================
// Definisi objek global (deklarasi extern-nya di Stelle.h)
// ============================================================
HardwareSerial SerialComA(1);
HardwareSerial Serial(0);

ADS1115 ads1;
ADS1115 ads2;

KinematicChain Kinematics;
InputChannels channels;

Vector3 offsetPos;
Quaternion offsetRot;
Quaternion offsetRotIMU;

void setup() {
    // NVS harus siap sebelum config dipakai
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Urutan sama dengan Stelle.ino versi Arduino.
    initMutexes();

    log1.begin();
    currentPowerState.begin();

    initTaskPowerKeeper();

    // Wire1(SDA=25, SCL=26)
    Wire1.begin(25, 26);

    config.begin();

    initTaskSerialComA();
    initTaskSerialUSB();
    initTaskForwardKinematicsComputer();

    const bool adsOk = ads1.begin(Wire1, ADS1115::ADDR_GND) &&
                       ads2.begin(Wire1, ADS1115::ADDR_SDA);
    initTaskSensorReader(adsOk);
}

void loop() {
    // Kosong, sama seperti versi Arduino: semua kerja di task.
}

} // namespace stelle

extern "C" void app_main(void) {
    stelle::setup();
    for (;;) {
        stelle::loop();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
