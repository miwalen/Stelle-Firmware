#include "ConfigManager.h"

#include <cstring>
#include "Logger.h"
#include "Preferences.h"
#include "setup.h"

namespace stelle {

void ConfigManager::copyField(char* dst, size_t len, const char* src) {
    std::memset(dst, 0, len);
    if (src == nullptr) return;
    std::strncpy(dst, src, len - 1);
    dst[len - 1] = 0;
}

void ConfigManager::getString(const char* key, char* out, size_t outLen,
                              const char* defaultValue) {
    if (_prefs != nullptr) {
        if (_prefs->begin(NAMESPACE, true)) {
            _prefs->getString(key, out, outLen, defaultValue);
            _prefs->end();
            return;
        }
    }
    copyField(out, outLen, defaultValue);
}

bool ConfigManager::begin(Preferences* preferences) {
    _prefs = preferences;
    applyConfig();
    logI(log1, "config: configuration loaded");
    return true;
}

void ConfigManager::applyConfig() {
    loadWifiSSID(wifiSSID, sizeof(wifiSSID), "MPDA");
    loadWifiPassword(wifiPassword, sizeof(wifiPassword), "niubiniubi");
    loadBtName(btName, sizeof(btName), "Stelle");
    loadBtPassword(btPassword, sizeof(btPassword), "1234");
    useIMU = loadUseIMU();

    imuQuaternionOffset = loadIMUQuaternionOffset().unpack();
    quaternionOffset = loadQuaternionOffset().unpack();
    vector3Offset = loadVector3Offset().unpack();
}

void ConfigManager::clearAll() {
    if (_prefs != nullptr) {
        if (_prefs->begin(NAMESPACE, false)) {
            _prefs->clear();
            _prefs->end();
        }
    }
    applyConfig();
}

// ==========================================
// SET (apply langsung ke RAM, TANPA nulis ke flash)
// ==========================================
void ConfigManager::setWifiSSID(const char* ssid) { copyField(wifiSSID, sizeof(wifiSSID), ssid); }
void ConfigManager::setWifiPassword(const char* p) { copyField(wifiPassword, sizeof(wifiPassword), p); }
void ConfigManager::setBtName(const char* n) { copyField(btName, sizeof(btName), n); }
void ConfigManager::setBtPassword(const char* p) { copyField(btPassword, sizeof(btPassword), p); }
void ConfigManager::setUseIMU(bool enable) { useIMU = enable; }

void ConfigManager::setIMUQuaternionOffset(QuaternionPacket packet) {
    imuQuaternionOffset = packet.unpack();
}

void ConfigManager::setQuaternionOffset(QuaternionPacket packet) {
    quaternionOffset = packet.unpack();
}

void ConfigManager::setVector3Offset(Vector3Packet packet) {
    vector3Offset = packet.unpack();
}

// ==========================================
// SAVE / LOAD (baca-tulis flash / NVS via Preferences)
// ==========================================

// Wi-Fi & BT
void ConfigManager::saveWifiSSID(const char* ssid) {
    copyField(wifiSSID, sizeof(wifiSSID), ssid);
    if (_prefs != nullptr && _prefs->begin(NAMESPACE, false)) {
        _prefs->putString("wifi_ssid", wifiSSID);
        _prefs->end();
    }
}

void ConfigManager::loadWifiSSID(char* out, size_t outLen, const char* defaultValue) {
    getString("wifi_ssid", out, outLen, defaultValue);
}

void ConfigManager::saveWifiPassword(const char* password) {
    copyField(wifiPassword, sizeof(wifiPassword), password);
    if (_prefs != nullptr && _prefs->begin(NAMESPACE, false)) {
        _prefs->putString("wifi_pass", wifiPassword);
        _prefs->end();
    }
}

void ConfigManager::loadWifiPassword(char* out, size_t outLen, const char* defaultValue) {
    getString("wifi_pass", out, outLen, defaultValue);
}

void ConfigManager::saveBtName(const char* name) {
    copyField(btName, sizeof(btName), name);
    if (_prefs != nullptr && _prefs->begin(NAMESPACE, false)) {
        _prefs->putString("bt_name", btName);
        _prefs->end();
    }
}

void ConfigManager::loadBtName(char* out, size_t outLen, const char* defaultValue) {
    getString("bt_name", out, outLen, defaultValue);
}

void ConfigManager::saveBtPassword(const char* password) {
    copyField(btPassword, sizeof(btPassword), password);
    if (_prefs != nullptr && _prefs->begin(NAMESPACE, false)) {
        _prefs->putString("bt_pass", btPassword);
        _prefs->end();
    }
}

void ConfigManager::loadBtPassword(char* out, size_t outLen, const char* defaultValue) {
    getString("bt_pass", out, outLen, defaultValue);
}

// IMU State
void ConfigManager::saveUseIMU(bool enable) {
    useIMU = enable;
    if (_prefs != nullptr && _prefs->begin(NAMESPACE, false)) {
        _prefs->putBool("use_imu", enable);
        _prefs->end();
    }
}

bool ConfigManager::loadUseIMU(bool defaultValue) {
    if (_prefs != nullptr && _prefs->begin(NAMESPACE, true)) {
        bool res = _prefs->getBool("use_imu", defaultValue);
        _prefs->end();
        return res;
    }
    return defaultValue;
}

// Quaternion / Vector3
void ConfigManager::saveIMUQuaternionOffset(QuaternionPacket packet) {
    uint8_t buffer[16];
    packet.getPacket(buffer);

    if (_prefs != nullptr && _prefs->begin(NAMESPACE, false)) {
        _prefs->putBytes("imu_quat_off", buffer, packet.size());
        _prefs->end();
    }
}

QuaternionPacket ConfigManager::loadIMUQuaternionOffset() {
    QuaternionPacket packet;
    uint8_t buffer[16];

    if (_prefs != nullptr && _prefs->begin(NAMESPACE, true)) {
        size_t len = _prefs->getBytes("imu_quat_off", buffer, sizeof(buffer));
        if (len == packet.size()) {
            packet.setPacket(buffer);
        }
        _prefs->end();
    }
    return packet;
}

void ConfigManager::saveQuaternionOffset(QuaternionPacket packet) {
    uint8_t buffer[16];
    packet.getPacket(buffer);

    if (_prefs != nullptr && _prefs->begin(NAMESPACE, false)) {
        _prefs->putBytes("quat_off", buffer, packet.size());
        _prefs->end();
    }
}

QuaternionPacket ConfigManager::loadQuaternionOffset() {
    QuaternionPacket packet;
    uint8_t buffer[16];

    if (_prefs != nullptr && _prefs->begin(NAMESPACE, true)) {
        size_t len = _prefs->getBytes("quat_off", buffer, sizeof(buffer));
        if (len == packet.size()) {
            packet.setPacket(buffer);
        }
        _prefs->end();
    }
    return packet;
}

void ConfigManager::saveVector3Offset(Vector3Packet packet) {
    uint8_t buffer[12];
    packet.getPacket(buffer);

    if (_prefs != nullptr && _prefs->begin(NAMESPACE, false)) {
        _prefs->putBytes("vec3_off", buffer, packet.size());
        _prefs->end();
    }
}

Vector3Packet ConfigManager::loadVector3Offset() {
    Vector3Packet packet;
    uint8_t buffer[12];

    if (_prefs != nullptr && _prefs->begin(NAMESPACE, true)) {
        size_t len = _prefs->getBytes("vec3_off", buffer, sizeof(buffer));
        if (len == packet.size()) {
            packet.setPacket(buffer);
        }
        _prefs->end();
    }
    return packet;
}

} // namespace stelle
