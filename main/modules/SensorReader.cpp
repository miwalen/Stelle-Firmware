#include "SensorReader.h"

#include "ADS1115.h"
#include "ForwardKinematicsComputer.h"
#include "Logger.h"
#include "MechLib.h"
#include "freertos/task.h"
#include "setup.h"

namespace stelle {

TaskHandle_t TaskSensorReaderHandle = NULL;

ADS1115 ads1;
ADS1115 ads2;

// Hasil pembacaan terakhir untuk 2 ADS x 3 channel
static float raw[2][3] = {0};

void initTaskSensorReader() {
    const bool adsOk = ads1.begin(Wire1, ADS1115::ADDR_GND) && ads2.begin(Wire1, ADS1115::ADDR_SDA);

    if (!adsOk) {
        logE(log1, "ADS1115: one or all failed, check the wiring");
    } else {
        logI(log1, "ADS1115: all present");
    }

    xTaskCreatePinnedToCore(TaskSensorReader, "SensorReader_Task", 4096, NULL, 1,
                            &TaskSensorReaderHandle, 1);
}

void TaskSensorReader(void* parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(10);  // 100 Hz

    for (;;) {
        // Periodic task dengan fixed-rate scheduling (delay-until)
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        // Baca semua channel ADS1115 #1 dan #2
        for (uint8_t ch = 0; ch < 3; ch++) {
            raw[0][ch] = (float)ads1.readRaw(ch);
            raw[1][ch] = (float)ads2.readRaw(ch);
        }

        if (!ads1.isOk()) {
            logW(log1, "ADS1115 1 read error (I2C/timeout)");
        }
        if (!ads2.isOk()) {
            logW(log1, "ADS1115 2 read error (I2C/timeout)");
        }

        if (xSemaphoreTake(mutexJoints, portMAX_DELAY)) {
            for (uint8_t ch = 0; ch < 3; ch++) {
                Kinematics.setAnalogInput(ch, raw[0][ch]);
                Kinematics.setAnalogInput(ch + 3, raw[1][ch]);
            }
            xSemaphoreGive(mutexJoints);
        }
    }
}

} // namespace stelle
