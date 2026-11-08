#ifndef ConfigManager_H
#define ConfigManager_H

#include <Arduino.h>
#include <Preferences.h>
#include "Packets.h"

class ConfigManager {
private:
    Preferences prefs;
    const char* NAMESPACE = "Configurations";

public:
    // ==========================================
    // VARIABEL KONFIGURASI AKTIF (PACKET REAL DI RAM)
    // ==========================================
    String wifiSSID;
    String wifiPassword;
    String btName;
    String btPassword;
    bool useIMU;
    
    Quaternion imuQuaternionOffset;
    Quaternion quaternionOffset;
    Vector3 vector3Offset;

    ConfigManager();
    ~ConfigManager();

    bool begin();
    void clearAll();
    void applyConfig();

    // Wi-Fi & BT
    void setWifiSSID(String& ssid);
    void saveWifiSSID(String& ssid);
    String loadWifiSSID(const String& defaultValue = "");

    void setWifiPassword(String& password);
    void saveWifiPassword(String& password);
    String loadWifiPassword(const String& defaultValue = "");

    void setBtName(String& name);
    void saveBtName(String& name);
    String loadBtName(const String& defaultValue = "StelleBT");

    void setBtPassword(String& password);
    void saveBtPassword(String& password);
    String loadBtPassword(const String& defaultValue = "1234");

    // IMU State
    void setUseIMU(bool enable);
    void saveUseIMU(bool enable);
    bool loadUseIMU(bool defaultValue = true);

    void setIMUQuaternionOffset(QuaternionPacket packet);
    void saveIMUQuaternionOffset(QuaternionPacket packet);
    QuaternionPacket loadIMUQuaternionOffset();
    
    void setQuaternionOffset(QuaternionPacket packet);
    void saveQuaternionOffset(QuaternionPacket packet);
    QuaternionPacket loadQuaternionOffset();

    void setVector3Offset(Vector3Packet packet);
    void saveVector3Offset(Vector3Packet packet);
    Vector3Packet loadVector3Offset();
};

#endif