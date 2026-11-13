#pragma once

#include <cstddef>

#include "GeoLib.h"

namespace stelle {

class Preferences;

// NVS: wifi, BT, useIMU, offset.
//   set*  -> apply langsung ke RAM, tanpa nulis ke flash
//   save* -> tulis ke NVS
//   load* -> baca dari NVS
class ConfigManager {
public:
    static constexpr const char* NAMESPACE = "stelle_cfg";

    // ==========================================
    // VARIABEL KONFIGURASI AKTIF (PACKET REAL DI RAM)
    // ==========================================
    char wifiSSID[33] = {};
    char wifiPassword[33] = {};
    char btName[33] = {};
    char btPassword[33] = {};
    bool useIMU = true;

    Quaternion imuQuaternionOffset;
    Quaternion quaternionOffset;
    Vector3 vector3Offset;

    bool begin(Preferences* preferences = nullptr);
    void clearAll();
    void applyConfig();

    // ==========================================
    // SET (apply langsung ke RAM, TANPA nulis ke flash)
    // ==========================================
    void setWifiSSID(const char* ssid);
    void setWifiPassword(const char* password);
    void setBtName(const char* name);
    void setBtPassword(const char* password);
    void setUseIMU(bool enable);
    void setQuaternionOffset(QuaternionPacket packet);
    void setVector3Offset(Vector3Packet packet);
    void setIMUQuaternionOffset(QuaternionPacket packet);

    // ==========================================
    // SAVE / LOAD (baca-tulis flash / NVS)
    // ==========================================
    void saveWifiSSID(const char* ssid);
    void saveWifiPassword(const char* password);
    void saveBtName(const char* name);
    void saveBtPassword(const char* password);
    void saveUseIMU(bool enable);
    void saveQuaternionOffset(QuaternionPacket packet);
    void saveVector3Offset(Vector3Packet packet);
    void saveIMUQuaternionOffset(QuaternionPacket packet);

    void loadWifiSSID(char* out, size_t outLen, const char* defaultValue = "");
    void loadWifiPassword(char* out, size_t outLen, const char* defaultValue = "");
    void loadBtName(char* out, size_t outLen, const char* defaultValue = "StelleBT");
    void loadBtPassword(char* out, size_t outLen, const char* defaultValue = "1234");
    bool loadUseIMU(bool defaultValue = true);
    QuaternionPacket loadQuaternionOffset();
    Vector3Packet loadVector3Offset();
    QuaternionPacket loadIMUQuaternionOffset();

private:
    Preferences* _prefs = nullptr;

    static void copyField(char* dst, size_t len, const char* src);
    void getString(const char* key, char* out, size_t outLen, const char* defaultValue);
};

// Instance global, seperti deklarasi di Stelle.ino.
extern ConfigManager config;

} // namespace stelle
