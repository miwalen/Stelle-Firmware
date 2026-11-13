#include "Communication.h"

#include <cstring>

#include "ConfigManager.h"
#include "ForwardKinematicsComputer.h"
#include "Logger.h"
#include "PowerManager.h"
#include "setup.h"

namespace stelle {

TaskHandle_t TaskSerialHandle = NULL;
TaskHandle_t TaskSerialUSBHandle = NULL; // tambahan untuk USB (Serial)

// ==========================================
// sendFrame: bungkus 0xAA 0x55 | cmd | payload | xor
// ==========================================
bool sendFrame(HardwareSerial& port, uint8_t cmd, const uint8_t* payload, uint8_t len) {
    port.write(HEADER_1);
    port.write(HEADER_2);
    port.write(cmd);

    uint8_t chk = cmd;
    for (uint8_t i = 0; i < len; i++) {
        port.write(payload[i]);
        chk ^= payload[i];
    }
    port.write(chk);
    return true;
}

bool sendFrame(HardwareSerial& port, Cmd cmd, const uint8_t* payload, uint8_t len) {
    return sendFrame(port, static_cast<uint8_t>(cmd), payload, len);
}

bool sendFrame(HardwareSerial& port, Cmd cmd) {
    return sendFrame(port, static_cast<uint8_t>(cmd), nullptr, 0);
}

// ==========================================
// FUNGSI EKSEKUSI COMMAND
// ==========================================
void executeCommand(HardwareSerial& port, Cmd cmd, uint8_t* payload, uint8_t len) {
    char strBuffer[33];

    switch (cmd) {
        case Cmd::Ping:
            logI(log1, "Ping OK");
            break;

        case Cmd::SetQuatOffset:
            if (len == sizeof(QuaternionPacket)) {
                QuaternionPacket qp;
                qp.setPacket(payload);

                if (xSemaphoreTake(mutexQuaternionOffset, portMAX_DELAY)) {
                    config.setQuaternionOffset(qp);
                    xSemaphoreGive(mutexQuaternionOffset);
                }

                if (xSemaphoreTake(mutexSaveConfig, portMAX_DELAY)) {
                    config.saveQuaternionOffset(qp);
                    xSemaphoreGive(mutexSaveConfig);
                }

                logI(log1, "Quat Saved & Applied");
            }
            break;

        case Cmd::SetVec3Offset:
            if (len == sizeof(Vector3Packet)) {
                Vector3Packet vp;
                vp.setPacket(payload);

                if (xSemaphoreTake(mutexVector3Offset, portMAX_DELAY)) {
                    config.setVector3Offset(vp);
                    xSemaphoreGive(mutexVector3Offset);
                }

                if (xSemaphoreTake(mutexSaveConfig, portMAX_DELAY)) {
                    config.saveVector3Offset(vp);
                    xSemaphoreGive(mutexSaveConfig);
                }

                logI(log1, "Vec3 Saved & Applied");
            }
            break;

        case Cmd::SetUseImu:
            if (len == 1) {
                bool useIMU = (payload[0] != 0);

                if (xSemaphoreTake(mutexUseIMU, portMAX_DELAY)) {
                    config.setUseIMU(useIMU);
                    xSemaphoreGive(mutexUseIMU);
                }

                if (xSemaphoreTake(mutexSaveConfig, portMAX_DELAY)) {
                    config.saveUseIMU(useIMU);
                    xSemaphoreGive(mutexSaveConfig);
                }

                logI(log1, "IMU State Saved & Applied");
            }
            break;

        case Cmd::SetWifiSsid:
            if (len == 32) {
                memcpy(strBuffer, payload, 32);
                strBuffer[32] = '\0';

                if (xSemaphoreTake(mutexWifiSSID, portMAX_DELAY)) {
                    config.setWifiSSID(strBuffer);
                    xSemaphoreGive(mutexWifiSSID);
                }

                if (xSemaphoreTake(mutexSaveConfig, portMAX_DELAY)) {
                    config.saveWifiSSID(strBuffer);
                    xSemaphoreGive(mutexSaveConfig);
                }

                logI(log1, "SSID Saved & Applied");
            }
            break;

        case Cmd::SetWifiPassword:
            if (len == 32) {
                memcpy(strBuffer, payload, 32);
                strBuffer[32] = '\0';

                if (xSemaphoreTake(mutexWifiPassword, portMAX_DELAY)) {
                    config.setWifiPassword(strBuffer);
                    xSemaphoreGive(mutexWifiPassword);
                }

                if (xSemaphoreTake(mutexSaveConfig, portMAX_DELAY)) {
                    config.saveWifiPassword(strBuffer);
                    xSemaphoreGive(mutexSaveConfig);
                }

                logI(log1, "WiFi Pass Saved & Applied");
            }
            break;

        case Cmd::SetBtName:
            if (len == 32) {
                memcpy(strBuffer, payload, 32);
                strBuffer[32] = '\0';

                if (xSemaphoreTake(mutexBtName, portMAX_DELAY)) {
                    config.setBtName(strBuffer);
                    xSemaphoreGive(mutexBtName);
                }

                if (xSemaphoreTake(mutexSaveConfig, portMAX_DELAY)) {
                    config.saveBtName(strBuffer);
                    xSemaphoreGive(mutexSaveConfig);
                }

                logI(log1, "BT Name Saved & Applied");
            }
            break;

        case Cmd::SetBtPassword:
            if (len == 32) {
                memcpy(strBuffer, payload, 32);
                strBuffer[32] = '\0';

                if (xSemaphoreTake(mutexBtPassword, portMAX_DELAY)) {
                    config.setBtPassword(strBuffer);
                    xSemaphoreGive(mutexBtPassword);
                }

                if (xSemaphoreTake(mutexSaveConfig, portMAX_DELAY)) {
                    config.saveBtPassword(strBuffer);
                    xSemaphoreGive(mutexSaveConfig);
                }

                logI(log1, "BT Pass Saved & Applied");
            }
            break;

        case Cmd::SetImuQuatOffset:
            if (len == sizeof(QuaternionPacket)) {
                QuaternionPacket qp;
                qp.setPacket(payload);

                if (xSemaphoreTake(mutexIMUQuaternionOffset, portMAX_DELAY)) {
                    config.setIMUQuaternionOffset(qp);
                    xSemaphoreGive(mutexIMUQuaternionOffset);
                }

                if (xSemaphoreTake(mutexSaveConfig, portMAX_DELAY)) {
                    config.saveIMUQuaternionOffset(qp);
                    xSemaphoreGive(mutexSaveConfig);
                }

                logI(log1, "IMU Quat Saved & Applied");
            }
            break;

        // ==========================================
        // COMMAND YANG MENGIRIM BALASAN (DATA / LOG)
        // ==========================================
        case Cmd::GetJointPos:
            if (len == sizeof(int)) {
                int idx;
                memcpy(&idx, payload, sizeof(int));

                if (idx >= 0 && idx < Kinematics.getJointCount()) {
                    Vector3Packet vp;
                    if (xSemaphoreTake(mutexJoints, portMAX_DELAY)) {
                        vp.pack(Kinematics.getJointPos(idx));
                        xSemaphoreGive(mutexJoints);
                    }

                    uint8_t sendBuffer[12];
                    vp.getPacket(sendBuffer);

                    sendFrame(port, cmd, sendBuffer, static_cast<uint8_t>(vp.size()));
                }
            }
            break;

        case Cmd::GetJointQuat:
            if (len == sizeof(int)) {
                int idx;
                memcpy(&idx, payload, sizeof(int));

                if (idx >= 0 && idx < Kinematics.getJointCount()) {
                    QuaternionPacket qp;
                    if (xSemaphoreTake(mutexJoints, portMAX_DELAY)) {
                        qp.pack(Kinematics.getJointQuat(idx));
                        xSemaphoreGive(mutexJoints);
                    }

                    uint8_t sendBuffer[16];
                    qp.getPacket(sendBuffer);

                    sendFrame(port, cmd, sendBuffer, static_cast<uint8_t>(qp.size()));
                }
            }
            break;

        // COMMAND BARU: Get Joint Angle Degree
        case Cmd::GetJointAngleDeg:
            if (len == sizeof(int)) {
                int idx;
                memcpy(&idx, payload, sizeof(int));

                if (idx >= 0 && idx < Kinematics.getJointCount()) {
                    float angleDeg = 0.0f;
                    if (xSemaphoreTake(mutexJoints, portMAX_DELAY)) {
                        angleDeg = Kinematics.getJointAngleDegree(idx);
                        xSemaphoreGive(mutexJoints);
                    }

                    uint8_t sendBuffer[sizeof(float)];
                    memcpy(sendBuffer, &angleDeg, sizeof(float));

                    sendFrame(port, cmd, sendBuffer, sizeof(sendBuffer));
                }
            }
            break;

        // COMMAND BARU: Get Joint Angle Radian
        case Cmd::GetJointAngleRad:
            if (len == sizeof(int)) {
                int idx;
                memcpy(&idx, payload, sizeof(int));

                if (idx >= 0 && idx < Kinematics.getJointCount()) {
                    float angleRad = 0.0f;
                    if (xSemaphoreTake(mutexJoints, portMAX_DELAY)) {
                        angleRad = Kinematics.getJointAngleRad(idx);
                        xSemaphoreGive(mutexJoints);
                    }

                    uint8_t sendBuffer[sizeof(float)];
                    memcpy(sendBuffer, &angleRad, sizeof(float));

                    sendFrame(port, cmd, sendBuffer, sizeof(sendBuffer));
                }
            }
            break;

        // COMMAND BARU: Get Joint Count
        case Cmd::GetJointCount:
            if (len == 0) {
                uint8_t jointCount = 0;
                if (xSemaphoreTake(mutexJoints, portMAX_DELAY)) {
                    jointCount = (uint8_t)Kinematics.getJointCount();
                    xSemaphoreGive(mutexJoints);
                }

                sendFrame(port, cmd, &jointCount, 1);
            }
            break;

        case Cmd::SetChannels:
            if (len == sizeof(InputChannelsPacket)) {
                InputChannelsPacket icp;
                icp.setPacket(payload);

                if (xSemaphoreTake(mutexChannels, portMAX_DELAY)) {
                    channels = icp.unpack();
                    xSemaphoreGive(mutexChannels);
                }
            }
            break;

        case Cmd::GetChannels:
            if (len == 0) {
                InputChannelsPacket icp;
                if (xSemaphoreTake(mutexChannels, portMAX_DELAY)) {
                    icp.pack(channels);
                    xSemaphoreGive(mutexChannels);
                }

                uint8_t sendBuffer[sizeof(InputChannelsPacket)];
                icp.getPacket(sendBuffer);

                sendFrame(port, cmd, sendBuffer, static_cast<uint8_t>(icp.size()));
            }
            break;

        case Cmd::GetLogUnread:
            if (len == 0) {
                char logBuffer[257];

                if (xSemaphoreTake(mutexLog, portMAX_DELAY)) {
                    log1.readUnread(logBuffer);
                    xSemaphoreGive(mutexLog);
                }

                uint16_t logLen = strlen(logBuffer);
                if (logLen > 255) logLen = 255;

                sendFrame(port, Cmd::ReplyLog,
                          reinterpret_cast<const uint8_t*>(logBuffer),
                          static_cast<uint8_t>(logLen));
            }
            break;

        case Cmd::GetLogAllHistory:
            if (len == 0) {
                char logBuffer[257];

                if (xSemaphoreTake(mutexLog, portMAX_DELAY)) {
                    log1.readAllHistory(logBuffer);
                    xSemaphoreGive(mutexLog);
                }

                uint16_t logLen = strlen(logBuffer);
                if (logLen > 255) logLen = 255;

                sendFrame(port, Cmd::ReplyLog,
                          reinterpret_cast<const uint8_t*>(logBuffer),
                          static_cast<uint8_t>(logLen));
            }
            break;

        // ==========================================
        // COMMAND POWER OFF
        // ==========================================
        case Cmd::PowerOff:
            if (len == 0) {
                logW(log1, "GoodBye");

                if (xSemaphoreTake(mutexSaveConfig, portMAX_DELAY)) {
                    if (xSemaphoreTake(mutexPowerState, portMAX_DELAY)) {
                        currentPowerState.setPowerState(false);
                        xSemaphoreGive(mutexPowerState);
                    }
                    xSemaphoreGive(mutexSaveConfig);
                }
                // PowerKeeper akan menulis pin LOW dalam <= CONFIG_STELLE_POWER_POLL_MS
            }
            break;

        default:
            break;
    }
}

// Port dikirim lewat pvParameters (SerialComA atau Serial)
void TaskSerialParse(void* pvParameters) {
    HardwareSerial* port = (HardwareSerial*)pvParameters;

    uint8_t rxState = 0;
    Cmd cmd = static_cast<Cmd>(0);
    uint8_t payloadLen = 0, payloadIdx = 0, checksumCalc = 0;
    uint8_t payloadBuffer[64];

    for (;;) {
        while (port->available() > 0) {
            uint8_t b = (uint8_t)port->read();

            switch (rxState) {
                case 0:
                    if (b == HEADER_1) rxState = 1;
                    break;
                case 1:
                    if (b == HEADER_2) rxState = 2;
                    else rxState = 0;
                    break;
                case 2: {
                    cmd = static_cast<Cmd>(b);
                    checksumCalc = b;

                    // Panjang payload dari tabel di Packets.h
                    payloadLen = PayloadLenForCommand(cmd);

                    payloadIdx = 0;
                    rxState = (payloadLen > 0) ? 3 : 4;
                    break;
                }
                case 3:
                    payloadBuffer[payloadIdx++] = b;
                    checksumCalc ^= b;
                    if (payloadIdx >= payloadLen) rxState = 4;
                    break;
                case 4:
                    if (b == checksumCalc) {
                        executeCommand(*port, cmd, payloadBuffer, payloadLen);
                    } else {
                        logW(log1, "Checksum error");
                    }
                    memset(payloadBuffer, 0, sizeof(payloadBuffer));
                    rxState = 0;
                    break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Port via pin (UART1)
void initTaskSerialComA() {
    SerialComA.setRxBufferSize(256);
    SerialComA.begin(115200, SERIAL_8N1, 16, 17);

    xTaskCreatePinnedToCore(
        TaskSerialParse,
        "SerialComA_Task",
        4096,
        (void*)&SerialComA,
        1,
        &TaskSerialHandle,
        1);
    logI(log1, "SerialComA is ready");
}

// Port via USB (Serial bawaan), untuk testing
void initTaskSerialUSB() {
    Serial.setRxBufferSize(256); // harus sebelum begin()
    Serial.begin(115200);

    xTaskCreatePinnedToCore(
        TaskSerialParse,
        "SerialUSB_Task",
        4096,
        (void*)&Serial,
        1,
        &TaskSerialUSBHandle,
        1);
    logI(log1, "SerialUSB is ready");
}

} // namespace stelle