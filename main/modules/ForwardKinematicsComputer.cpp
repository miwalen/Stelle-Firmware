#include "ForwardKinematicsComputer.h"

#include "MechLib.h"
#include "freertos/task.h"
#include "setup.h"

namespace stelle {

TaskHandle_t TaskFKComputerHandle = NULL;

void initTaskForwardKinematicsComputer() {
    // =========================================================================
    // Konfigurasi 6 Joint Rantai Kinematika (KinematicChain)
    // Parameter addJointConfig:
    //   1. Axis (Vector3): Vektor sumbu putar joint
    //   2. OffsetPosition (Vector3): Jarak/posisi relatif dari joint sebelumnya (meter)
    //   3. OffsetDegree (float): Kalibrasi offset rotasi awal dalam derajat
    //   4. MinRad (float): Batas sudut minimum dalam radian
    //   5. MaxRad (float): Batas sudut maksimum dalam radian
    //   6. AnalogMin (float): Batas nilai ADC minimum (ADS1115 single-ended raw)
    //   7. AnalogMax (float): Batas nilai ADC maksimum (ADS1115 single-ended raw)
    // =========================================================================

    // Joint 0: Base Joint (Rotasi Yaw pada sumbu Z)
    Kinematics.addJointConfig(
        Vector3(0.0f, 0.0f, 1.0f),  // Sumbu putar: Z (Yaw)
        Vector3(0.0f, 0.0f, 0.0f),  // Posisi relatif dari pangkal (base)
        0.0f,                       // Kalibrasi sudut offset: 0 derajat
        -3.14159265f, 3.14159265f,  // Rentang pergerakan: -180 hingga +180 derajat
        30, 26170                   // Rentang pembacaan ADS1115 raw
    );

    // Joint 1: Shoulder Joint (Bahu - rotasi Pitch pada sumbu Y)
    Kinematics.addJointConfig(
        Vector3(0.0f, 1.0f, 0.0f),  // Sumbu putar: Y (Pitch)
        Vector3(0.0f, 0.0f, 0.25f), // Link offset: tinggi bahu 25 cm (0.25 m)
        0.0f,                       // Kalibrasi sudut offset: 0 derajat
        -1.5707963f, 1.5707963f,    // Rentang pergerakan: -90 hingga +90 derajat
        43, 13010                   // Rentang pembacaan ADS1115 raw
    );

    // Joint 2: Elbow Joint (Siku - rotasi Pitch pada sumbu Y)
    Kinematics.addJointConfig(
        Vector3(0.0f, 1.0f, 0.0f),  // Sumbu putar: Y (Pitch)
        Vector3(0.0f, 0.0f, 0.20f), // Link offset: panjang lengan atas 20 cm (0.20 m)
        0.0f,                       // Kalibrasi sudut offset: 0 derajat
        -2.0943951f, 2.0943951f,    // Rentang pergerakan: -120 hingga +120 derajat
        0, 32767                    // Rentang pembacaan ADS1115 raw
    );

    // Joint 3: Wrist Pitch Joint (Pergelangan tangan - rotasi Pitch pada sumbu Y)
    Kinematics.addJointConfig(
        Vector3(0.0f, 1.0f, 0.0f),  // Sumbu putar: Y (Pitch)
        Vector3(0.15f, 0.0f, 0.0f), // Link offset: panjang lengan bawah 15 cm (0.15 m)
        0.0f,                       // Kalibrasi sudut offset: 0 derajat
        -1.5707963f, 1.5707963f,    // Rentang pergerakan: -90 hingga +90 derajat
        0, 32767                    // Rentang pembacaan ADS1115 raw
    );

    // Joint 4: Wrist Roll Joint (Pergelangan tangan - rotasi Roll pada sumbu X)
    Kinematics.addJointConfig(
        Vector3(1.0f, 0.0f, 0.0f),  // Sumbu putar: X (Roll)
        Vector3(0.05f, 0.0f, 0.0f), // Link offset: panjang pergelangan 5 cm (0.05 m)
        0.0f,                       // Kalibrasi sudut offset: 0 derajat
        -3.14159265f, 3.14159265f,  // Rentang pergerakan: -180 hingga +180 derajat
        0, 32767                    // Rentang pembacaan ADS1115 raw
    );

    // Joint 5: End-Effector / Tool Joint (Ujung tool / gripper pada sumbu Z)
    Kinematics.addJointConfig(
        Vector3(0.0f, 0.0f, 1.0f),  // Sumbu putar: Z (Yaw/Twist)
        Vector3(0.02f, 0.0f, 0.0f), // Link offset: panjang tool 2 cm (0.02 m)
        0.0f,                       // Kalibrasi sudut offset: 0 derajat
        -1.5707963f, 1.5707963f,    // Rentang pergerakan: -90 hingga +90 derajat
        0, 32767                    // Rentang pembacaan ADS1115 raw
    );

    xTaskCreatePinnedToCore(TaskForwardKinematicsComputer, "FKComputer_Task", 4096, NULL, 1,
                            &TaskFKComputerHandle, 1);
}

void TaskForwardKinematicsComputer(void* parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(20);  // 50 Hz

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        // Komputasi forward kinematics rantai sendi dengan perlindungan mutex
        if (xSemaphoreTake(mutexJoints, portMAX_DELAY)) {
            Kinematics.compute();
            xSemaphoreGive(mutexJoints);
        }
    }
}

} // namespace stelle
