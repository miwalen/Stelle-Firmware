#include "Communication.h"

HardwareSerial SerialComA(1); 

TaskHandle_t TaskSerialHandle = NULL;
TaskHandle_t TaskSerialUSBHandle = NULL; // tambahan untuk USB (Serial)

// Deklarasi extern untuk mutex save yang baru ditambahkan
extern SemaphoreHandle_t mutexSaveConfig;

// ==========================================
// FUNGSI EKSEKUSI COMMAND
// ==========================================
void executeCommand(Stream& port, uint8_t cmd, uint8_t* payload, uint8_t len) {
    char strBuffer[33]; 

    switch (cmd) {
        case 0x01: 
            if (xSemaphoreTake(mutexLog, portMAX_DELAY)){log1.log("Ping OK\n"); xSemaphoreGive(mutexLog);}
            break;
            
        case 0x10: 
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

                if (xSemaphoreTake(mutexLog, portMAX_DELAY)){log1.log("Quat Saved & Applied\n"); xSemaphoreGive(mutexLog);}
            }
            break;

        case 0x11: 
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

                if (xSemaphoreTake(mutexLog, portMAX_DELAY)){log1.log("Vec3 Saved & Applied\n"); xSemaphoreGive(mutexLog);}
            }
            break;

        case 0x12: 
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

                if (xSemaphoreTake(mutexLog, portMAX_DELAY)){log1.log("IMU State Saved & Applied\n"); xSemaphoreGive(mutexLog);}
            }
            break;

        case 0x13: 
            if (len == 32) {
                memcpy(strBuffer, payload, 32);
                strBuffer[32] = '\0';
                String ssid = String(strBuffer);
                ssid.trim();

                if (xSemaphoreTake(mutexWifiSSID, portMAX_DELAY)) {
                    config.setWifiSSID(ssid);
                    xSemaphoreGive(mutexWifiSSID);
                }
                
                if (xSemaphoreTake(mutexSaveConfig, portMAX_DELAY)) {
                    config.saveWifiSSID(ssid);
                    xSemaphoreGive(mutexSaveConfig);
                }

                if (xSemaphoreTake(mutexLog, portMAX_DELAY)){log1.log("SSID Saved & Applied\n"); xSemaphoreGive(mutexLog);}
            }
            break;

        case 0x14: 
            if (len == 32) {
                memcpy(strBuffer, payload, 32);
                strBuffer[32] = '\0';
                String pass = String(strBuffer);
                pass.trim();

                if (xSemaphoreTake(mutexWifiPassword, portMAX_DELAY)) {
                    config.setWifiPassword(pass);
                    xSemaphoreGive(mutexWifiPassword);
                }
                
                if (xSemaphoreTake(mutexSaveConfig, portMAX_DELAY)) {
                    config.saveWifiPassword(pass);
                    xSemaphoreGive(mutexSaveConfig);
                }

                if (xSemaphoreTake(mutexLog, portMAX_DELAY)){log1.log("WiFi Pass Saved & Applied\n"); xSemaphoreGive(mutexLog);}
            }
            break;

        case 0x15: 
            if (len == 32) {
                memcpy(strBuffer, payload, 32);
                strBuffer[32] = '\0';
                String btName = String(strBuffer);
                btName.trim();

                if (xSemaphoreTake(mutexBtName, portMAX_DELAY)) {
                    config.setBtName(btName);
                    xSemaphoreGive(mutexBtName);
                }
                
                if (xSemaphoreTake(mutexSaveConfig, portMAX_DELAY)) {
                    config.saveBtName(btName);
                    xSemaphoreGive(mutexSaveConfig);
                }

                if (xSemaphoreTake(mutexLog, portMAX_DELAY)){log1.log("BT Name Saved & Applied\n"); xSemaphoreGive(mutexLog);}
            }
            break;

        case 0x16: 
            if (len == 32) {
                memcpy(strBuffer, payload, 32);
                strBuffer[32] = '\0';
                String btPass = String(strBuffer);
                btPass.trim();

                if (xSemaphoreTake(mutexBtPassword, portMAX_DELAY)) {
                    config.setBtPassword(btPass);
                    xSemaphoreGive(mutexBtPassword);
                }
                
                if (xSemaphoreTake(mutexSaveConfig, portMAX_DELAY)) {
                    config.saveBtPassword(btPass);
                    xSemaphoreGive(mutexSaveConfig);
                }

                if (xSemaphoreTake(mutexLog, portMAX_DELAY)){log1.log("BT Pass Saved & Applied\n"); xSemaphoreGive(mutexLog);}
            }
            break;
            
        case 0x17: 
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

                if (xSemaphoreTake(mutexLog, portMAX_DELAY)){log1.log("IMU Quat Saved & Applied\n"); xSemaphoreGive(mutexLog);}
            }
            break;

        // ==========================================
        // COMMAND YANG MENGIRIM BALASAN (DATA / LOG)
        // ==========================================
        case 0x20: 
            if (len == sizeof(int)) {
                int idx;
                memcpy(&idx, payload, sizeof(int));
                
                if (idx >= 0 && idx < 6) { // Batas statis 6 joint
                    Vector3Packet vp;
                    if (xSemaphoreTake(mutexJoints, portMAX_DELAY)) {
                        vp.pack(Kinematics.getJointPos(idx));
                        xSemaphoreGive(mutexJoints);
                    }
                    
                    uint8_t sendBuffer[12];
                    vp.getPacket(sendBuffer);

                    port.write(HEADER_1);
                    port.write(HEADER_2);
                    port.write(0x20); 
                    uint8_t chk = 0x20;
                    for (size_t i = 0; i < vp.size(); i++) {
                        port.write(sendBuffer[i]);
                        chk ^= sendBuffer[i];
                    }
                    port.write(chk);
                }
            }
            break;

        case 0x21: 
            if (len == sizeof(int)) {
                int idx;
                memcpy(&idx, payload, sizeof(int));
                
                if (idx >= 0 && idx < 6) { // Batas statis 6 joint
                    QuaternionPacket qp;
                    if (xSemaphoreTake(mutexJoints, portMAX_DELAY)) {
                        qp.pack(Kinematics.getJointQuat(idx));
                        xSemaphoreGive(mutexJoints);
                    }
                    
                    uint8_t sendBuffer[16];
                    qp.getPacket(sendBuffer);

                    port.write(HEADER_1);
                    port.write(HEADER_2);
                    port.write(0x21); 
                    uint8_t chk = 0x21;
                    for (size_t i = 0; i < qp.size(); i++) {
                        port.write(sendBuffer[i]);
                        chk ^= sendBuffer[i];
                    }
                    port.write(chk);
                }
            }
            break;

        // COMMAND BARU: Get Joint Angle Degree
        case 0x22:
            if (len == sizeof(int)) {
                int idx;
                memcpy(&idx, payload, sizeof(int));
                
                if (idx >= 0 && idx < 6) {
                    float angleDeg = 0.0f;
                    if (xSemaphoreTake(mutexJoints, portMAX_DELAY)) {
                        angleDeg = Kinematics.getJointAngleDegree(idx);
                        xSemaphoreGive(mutexJoints);
                    }
                    
                    uint8_t sendBuffer[sizeof(float)];
                    memcpy(sendBuffer, &angleDeg, sizeof(float));

                    port.write(HEADER_1);
                    port.write(HEADER_2);
                    port.write(0x22); 
                    uint8_t chk = 0x22;
                    for (size_t i = 0; i < sizeof(float); i++) {
                        port.write(sendBuffer[i]);
                        chk ^= sendBuffer[i];
                    }
                    port.write(chk);
                }
            }
            break;

        // COMMAND BARU: Get Joint Angle Radian
        case 0x23:
            if (len == sizeof(int)) {
                int idx;
                memcpy(&idx, payload, sizeof(int));
                
                if (idx >= 0 && idx < 6) {
                    float angleRad = 0.0f;
                    if (xSemaphoreTake(mutexJoints, portMAX_DELAY)) {
                        angleRad = Kinematics.getJointAngleRad(idx);
                        xSemaphoreGive(mutexJoints);
                    }
                    
                    uint8_t sendBuffer[sizeof(float)];
                    memcpy(sendBuffer, &angleRad, sizeof(float));

                    port.write(HEADER_1);
                    port.write(HEADER_2);
                    port.write(0x23); 
                    uint8_t chk = 0x23;
                    for (size_t i = 0; i < sizeof(float); i++) {
                        port.write(sendBuffer[i]);
                        chk ^= sendBuffer[i];
                    }
                    port.write(chk);
                }
            }
            break;

        case 0x30: 
            if (len == sizeof(InputChannelsPacket)) {
                InputChannelsPacket icp;
                icp.setPacket(payload);
                
                if (xSemaphoreTake(mutexChannels, portMAX_DELAY)) {
                    channels = icp.unpack();
                    xSemaphoreGive(mutexChannels);
                }
            }
            break;

        case 0x31: 
            if (len == 0) {
                InputChannelsPacket icp;
                if (xSemaphoreTake(mutexChannels, portMAX_DELAY)) {
                    icp.pack(channels);
                    xSemaphoreGive(mutexChannels);
                }
                
                uint8_t sendBuffer[sizeof(InputChannelsPacket)];
                icp.getPacket(sendBuffer);

                port.write(HEADER_1);
                port.write(HEADER_2);
                port.write(0x31); 
                uint8_t chk = 0x31;
                for (size_t i = 0; i < icp.size(); i++) {
                    port.write(sendBuffer[i]);
                    chk ^= sendBuffer[i];
                }
                port.write(chk);
            }
            break;

        case 0x40: 
            if (len == 0) {            
                char logBuffer[257]; 
                
                if (xSemaphoreTake(mutexLog, portMAX_DELAY)) {
                    log1.readUnread(logBuffer);
                    xSemaphoreGive(mutexLog);
                }
                
                uint16_t logLen = strlen(logBuffer);
                if (logLen > 255) logLen = 255; 
                
                port.write(HEADER_1);
                port.write(HEADER_2);
                port.write(0x99); 
                uint8_t chk = 0x99;
                for (size_t i = 0; i < (uint8_t)logLen; i++) {
                    port.write((uint8_t)logBuffer[i]);
                    chk ^= (uint8_t)logBuffer[i];
                }
                port.write(chk);
            }
            break;

        case 0x41: 
            if (len == 0) {
                char logBuffer[257];
                
                if (xSemaphoreTake(mutexLog, portMAX_DELAY)) {
                    log1.readAllHistory(logBuffer);
                    xSemaphoreGive(mutexLog);
                }
                
                uint16_t logLen = strlen(logBuffer);
                if (logLen > 255) logLen = 255;
                
                port.write(HEADER_1);
                port.write(HEADER_2);
                port.write(0x99); 
                uint8_t chk = 0x99;
                for (size_t i = 0; i < (uint8_t)logLen; i++) {
                    port.write((uint8_t)logBuffer[i]);
                    chk ^= (uint8_t)logBuffer[i];
                }
                port.write(chk);
            }
            break;

        // ==========================================
        // COMMAND POWER OFF (payload 1 int, isi bebas / tidak dipakai)
        // ==========================================
        case 0x50:
            if (len == 0) {
                if (xSemaphoreTake(mutexLog, portMAX_DELAY)){log1.log("GoodBye\n"); xSemaphoreGive(mutexLog);}

                if (xSemaphoreTake(mutexSaveConfig, portMAX_DELAY)) {
                    if (xSemaphoreTake(mutexPowerState, portMAX_DELAY)) {
                        currentPowerState.setPowerState(false);
                        xSemaphoreGive(mutexPowerState);
                    }
                    xSemaphoreGive(mutexSaveConfig);
                }
                // PowerKeeper akan menulis pin LOW dalam <= 50 ms
            }
            break;

        default:
            break;
    }
}

// Port dikirim lewat pvParameters (SerialComA atau Serial)
void TaskSerialParse(void *pvParameters) {
    Stream* port = (Stream*)pvParameters;

    uint8_t rxState = 0;
    uint8_t cmd = 0, payloadLen = 0, payloadIdx = 0, checksumCalc = 0;
    uint8_t payloadBuffer[64];

    for (;;) {
        while (port->available() > 0) {
            uint8_t b = port->read();

            switch (rxState) {
                case 0:
                    if (b == HEADER_1) rxState = 1;
                    break;
                case 1:
                    if (b == HEADER_2) rxState = 2;
                    else rxState = 0;
                    break;
                case 2:
                    cmd = b;
                    checksumCalc = b;
                    
                    if (cmd == 0x01) payloadLen = 0;
                    else if (cmd == 0x10 || cmd == 0x17) payloadLen = 16;
                    else if (cmd == 0x11) payloadLen = 12;
                    else if (cmd == 0x12) payloadLen = 1; 
                    else if (cmd >= 0x13 && cmd <= 0x16) payloadLen = 32;
                    else if (cmd >= 0x20 && cmd <= 0x23) payloadLen = sizeof(int); 
                    else if (cmd == 0x30) payloadLen = InputChannels::SIZE * sizeof(float);
                    else if (cmd == 0x31) payloadLen = 0;
                    else if (cmd == 0x50) payloadLen = 0;   // power off
                    else payloadLen = 0; 
                    
                    payloadIdx = 0;
                    rxState = (payloadLen > 0) ? 3 : 4;
                    break;
                case 3:
                    payloadBuffer[payloadIdx++] = b;
                    checksumCalc ^= b;
                    if (payloadIdx >= payloadLen) rxState = 4;
                    break;
                case 4:
                    if (b == checksumCalc) {
                        executeCommand(*port, cmd, payloadBuffer, payloadLen);
                    } else {
                        if (xSemaphoreTake(mutexLog, portMAX_DELAY)){log1.log("SerialComA Error: Checksum\n"); xSemaphoreGive(mutexLog);}
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
        1
    );
    if (xSemaphoreTake(mutexLog, portMAX_DELAY)) {log1.log("SerialComA is ready bitch\n");xSemaphoreGive(mutexLog);}
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
        1
    );
    if (xSemaphoreTake(mutexLog, portMAX_DELAY)) {log1.log("SerialUSB is ready bitch\n");xSemaphoreGive(mutexLog);}
}