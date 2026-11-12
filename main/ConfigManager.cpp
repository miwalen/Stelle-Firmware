#include "ConfigManager.h"

#include <cstring>

#include "nvs.h"

namespace stelle {

void ConfigManager::copyField(char* dst, size_t len, const char* src) {
    std::memset(dst, 0, len);
    if (src == nullptr) return;
    std::strncpy(dst, src, len - 1);
    dst[len - 1] = 0;
}

void ConfigManager::getString(const char* key, char* out, size_t outLen,
                              const char* defaultValue) {
    nvs_handle_t h = 0;
    size_t len = outLen;

    if (nvs_open(NAMESPACE, NVS_READONLY, &h) == ESP_OK) {
        if (nvs_get_str(h, key, out, &len) != ESP_OK) {
            copyField(out, outLen, defaultValue);
        }
        nvs_close(h);
    } else {
        copyField(out, outLen, defaultValue);
    }
}

bool ConfigManager::begin() {
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
    nvs_handle_t h = 0;
    if (nvs_open(NAMESPACE, NVS_READWRITE, &h) == ESP_OK) {
        nvs_erase_all(h);
        nvs_commit(h);
        nvs_close(h);
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
// SAVE / LOAD (baca-tulis flash / NVS)
// ==========================================

// Wi-Fi & BT
void ConfigManager::saveWifiSSID(const char* ssid) {
    copyField(wifiSSID, sizeof(wifiSSID), ssid);
    nvs_handle_t h = 0;
    if (nvs_open(NAMESPACE, NVS_READWRITE, &h) == ESP_OK) {
        if (nvs_set_str(h, "wifi_ssid", wifiSSID) == ESP_OK) nvs_commit(h);
        nvs_close(h);
    }
}

void ConfigManager::loadWifiSSID(char* out, size_t outLen, const char* defaultValue) {
    getString("wifi_ssid", out, outLen, defaultValue);
}

void ConfigManager::saveWifiPassword(const char* password) {
    copyField(wifiPassword, sizeof(wifiPassword), password);
    nvs_handle_t h = 0;
    if (nvs_open(NAMESPACE, NVS_READWRITE, &h) == ESP_OK) {
        if (nvs_set_str(h, "wifi_pass", wifiPassword) == ESP_OK) nvs_commit(h);
        nvs_close(h);
    }
}

void ConfigManager::loadWifiPassword(char* out, size_t outLen, const char* defaultValue) {
    getString("wifi_pass", out, outLen, defaultValue);
}

void ConfigManager::saveBtName(const char* name) {
    copyField(btName, sizeof(btName), name);
    nvs_handle_t h = 0;
    if (nvs_open(NAMESPACE, NVS_READWRITE, &h) == ESP_OK) {
        if (nvs_set_str(h, "bt_name", btName) == ESP_OK) nvs_commit(h);
        nvs_close(h);
    }
}

void ConfigManager::loadBtName(char* out, size_t outLen, const char* defaultValue) {
    getString("bt_name", out, outLen, defaultValue);
}

void ConfigManager::saveBtPassword(const char* password) {
    copyField(btPassword, sizeof(btPassword), password);
    nvs_handle_t h = 0;
    if (nvs_open(NAMESPACE, NVS_READWRITE, &h) == ESP_OK) {
        if (nvs_set_str(h, "bt_pass", btPassword) == ESP_OK) nvs_commit(h);
        nvs_close(h);
    }
}

void ConfigManager::loadBtPassword(char* out, size_t outLen, const char* defaultValue) {
    getString("bt_pass", out, outLen, defaultValue);
}

// IMU State
void ConfigManager::saveUseIMU(bool enable) {
    useIMU = enable;
    nvs_handle_t h = 0;
    if (nvs_open(NAMESPACE, NVS_READWRITE, &h) == ESP_OK) {
        if (nvs_set_u8(h, "use_imu", enable ? 1 : 0) == ESP_OK) nvs_commit(h);
        nvs_close(h);
    }
}

bool ConfigManager::loadUseIMU(bool defaultValue) {
    nvs_handle_t h = 0;
    uint8_t flag = defaultValue ? 1 : 0;

    if (nvs_open(NAMESPACE, NVS_READONLY, &h) == ESP_OK) {
        nvs_get_u8(h, "use_imu", &flag);
        nvs_close(h);
    }
    return flag != 0;
}

// Quaternion / Vector3
void ConfigManager::saveIMUQuaternionOffset(QuaternionPacket packet) {
    uint8_t buffer[16];
    packet.getPacket(buffer);

    nvs_handle_t h = 0;
    if (nvs_open(NAMESPACE, NVS_READWRITE, &h) == ESP_OK) {
        if (nvs_set_blob(h, "imu_quat_off", buffer, packet.size()) == ESP_OK) nvs_commit(h);
        nvs_close(h);
    }
}

QuaternionPacket ConfigManager::loadIMUQuaternionOffset() {
    QuaternionPacket packet;
    uint8_t buffer[16];

    nvs_handle_t h = 0;
    if (nvs_open(NAMESPACE, NVS_READONLY, &h) == ESP_OK) {
        size_t len = sizeof(buffer);
        if (nvs_get_blob(h, "imu_quat_off", buffer, &len) == ESP_OK && len == packet.size()) {
            packet.setPacket(buffer);
        }
        nvs_close(h);
    }
    return packet;
}

void ConfigManager::saveQuaternionOffset(QuaternionPacket packet) {
    uint8_t buffer[16];
    packet.getPacket(buffer);

    nvs_handle_t h = 0;
    if (nvs_open(NAMESPACE, NVS_READWRITE, &h) == ESP_OK) {
        if (nvs_set_blob(h, "quat_off", buffer, packet.size()) == ESP_OK) nvs_commit(h);
        nvs_close(h);
    }
}

QuaternionPacket ConfigManager::loadQuaternionOffset() {
    QuaternionPacket packet;
    uint8_t buffer[16];

    nvs_handle_t h = 0;
    if (nvs_open(NAMESPACE, NVS_READONLY, &h) == ESP_OK) {
        size_t len = sizeof(buffer);
        if (nvs_get_blob(h, "quat_off", buffer, &len) == ESP_OK && len == packet.size()) {
            packet.setPacket(buffer);
        }
        nvs_close(h);
    }
    return packet;
}

void ConfigManager::saveVector3Offset(Vector3Packet packet) {
    uint8_t buffer[12];
    packet.getPacket(buffer);

    nvs_handle_t h = 0;
    if (nvs_open(NAMESPACE, NVS_READWRITE, &h) == ESP_OK) {
        if (nvs_set_blob(h, "vec3_off", buffer, packet.size()) == ESP_OK) nvs_commit(h);
        nvs_close(h);
    }
}

Vector3Packet ConfigManager::loadVector3Offset() {
    Vector3Packet packet;
    uint8_t buffer[12];

    nvs_handle_t h = 0;
    if (nvs_open(NAMESPACE, NVS_READONLY, &h) == ESP_OK) {
        size_t len = sizeof(buffer);
        if (nvs_get_blob(h, "vec3_off", buffer, &len) == ESP_OK && len == packet.size()) {
            packet.setPacket(buffer);
        }
        nvs_close(h);
    }
    return packet;
}

} // namespace stelle