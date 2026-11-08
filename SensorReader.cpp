#include "SensorReader.h"
#include "Mutexes.h"
#include "Logger.h"
#include "MechLib.h"
#include "ConfigManager.h"

TaskHandle_t TaskSensorReaderHandle = NULL;
static bool adsFailFlag = false;

// Hasil pembacaan terakhir (tegangan, Volt) untuk 2 ADS x 4 channel
static float raw[2][4] = {0};

void initTaskSensorReader() {
    // Inisialisasi ADS1115 #1
    if (!ads1.begin(I2C_1,0x48) || !ads1.isConnected()) {
        adsFailFlag = true;
        if (xSemaphoreTake(mutexLog, portMAX_DELAY)) {log1.log("ADS1115 1 is not connected\n");xSemaphoreGive(mutexLog);}
    } else {
        ads1.setPGA(ADS1115::PGA_4_096V);
        ads1.setDataRate(ADS1115::DR_860SPS);
    }

    // Inisialisasi ADS1115 #2
    if (!ads2.begin(I2C_1,0x4A) || !ads2.isConnected()) {
        adsFailFlag = true;
        if (xSemaphoreTake(mutexLog, portMAX_DELAY)) {log1.log("ADS1115 2 is not connected\n");xSemaphoreGive(mutexLog);}
    } else {
        ads2.setPGA(ADS1115::PGA_4_096V);
        ads2.setDataRate(ADS1115::DR_860SPS);
    }

    if (adsFailFlag) {
        if (xSemaphoreTake(mutexLog, portMAX_DELAY)) {log1.log("One or all ADS1115 is fucking failed :(, Have fun fixing it :)\n");xSemaphoreGive(mutexLog);}
    } else {
        if (xSemaphoreTake(mutexLog, portMAX_DELAY)) {log1.log("All ADS1115 is absolutely fine :)\n");xSemaphoreGive(mutexLog);}
    }

    xTaskCreatePinnedToCore(
        TaskSensorReader,
        "SensorReader_Task",
        4096,
        NULL,
        1,
        &TaskSensorReaderHandle,
        1
    );
}

void TaskSensorReader(void *parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(10); // 100 Hz (10 ms period)

    for (;;) {
        // Periodic task dengan fixed-rate scheduling (delay-until)
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        if (adsFailFlag) {
            // Task pembacaan tidak jalan dan mulai spam log
            if (xSemaphoreTake(mutexLog, portMAX_DELAY)) {log1.log("your ADS1115 is a fucking failure\n");xSemaphoreGive(mutexLog);}
            vTaskDelay(pdMS_TO_TICKS(3000));
            xLastWakeTime = xTaskGetTickCount();
            continue;
        }

        // Baca semua channel ADS1115 #1 dan #2
        for (uint8_t ch = 0; ch < 3; ch++) {
            raw[0][ch] = ads1.readRaw(ch);
            raw[1][ch] = ads2.readRaw(ch);
        }

            // Cek kesehatan runtime: isOk() jadi false kalau I2C error / timeout
        if (!ads1.isOk()) {
            if (xSemaphoreTake(mutexLog, portMAX_DELAY)) {log1.log("ADS1115 1 read error (I2C/timeout)\n");xSemaphoreGive(mutexLog);}
        }
        if (!ads2.isOk()) {
            if (xSemaphoreTake(mutexLog, portMAX_DELAY)) {log1.log("ADS1115 2 read error (I2C/timeout)\n");xSemaphoreGive(mutexLog);}
        }
        if (!ads1.isConnected() || !ads2.isConnected()) {
            adsFailFlag = true;
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