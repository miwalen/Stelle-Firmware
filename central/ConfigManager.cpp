#include "ConfigManager.h"

ConfigManager::ConfigManager() {}

ConfigManager::~ConfigManager() {
    prefs.end();
}

bool ConfigManager::begin() {
    bool status = prefs.begin(NAMESPACE, false);
    if (status) {
        applyConfig();
    }
    return status;
}

void ConfigManager::applyConfig() {
    wifiSSID            = loadWifiSSID();
    wifiPassword        = loadWifiPassword();
    btName              = loadBtName();
    btPassword          = loadBtPassword();
    useIMU              = loadUseIMU();
    imuQuaternionOffset = loadIMUQuaternionOffset().unpack();
    quaternionOffset    = loadQuaternionOffset().unpack();
    vector3Offset       = loadVector3Offset().unpack();
}

void ConfigManager::clearAll() {
    prefs.clear();
    applyConfig();
}

// ==========================================
// SET (apply langsung ke RAM, TANPA nulis ke flash)
// ==========================================
void ConfigManager::setWifiSSID(String& ssid)         { wifiSSID = ssid; }
void ConfigManager::setWifiPassword(String& password) { wifiPassword = password; }
void ConfigManager::setBtName(String& name)             { btName = name; }
void ConfigManager::setBtPassword(String& password)     { btPassword = password; }
void ConfigManager::setUseIMU(bool enable)                     { useIMU = enable; }

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
// SAVE / LOAD (baca-tulis Preferences/flash)
// ==========================================

// Wi-Fi & BT
void ConfigManager::saveWifiSSID(String& ssid) { wifiSSID = ssid; prefs.putString("wifi_ssid", ssid); }
String ConfigManager::loadWifiSSID(const String& defaultValue) { return prefs.getString("wifi_ssid", "MPDA"); }

void ConfigManager::saveWifiPassword(String& password) { wifiPassword = password; prefs.putString("wifi_pass", password); }
String ConfigManager::loadWifiPassword(const String& defaultValue) { return prefs.getString("wifi_pass", "niubiniubi"); }

void ConfigManager::saveBtName(String& name) { btName = name; prefs.putString("bt_name", name); }
String ConfigManager::loadBtName(const String& defaultValue) { return prefs.getString("bt_name", "Stelle"); }

void ConfigManager::saveBtPassword(String& password) { btPassword = password; prefs.putString("bt_pass", password); }
String ConfigManager::loadBtPassword(const String& defaultValue) { return prefs.getString("bt_pass", "1234"); }

void ConfigManager::saveUseIMU(bool enable) { useIMU = enable; prefs.putBool("use_imu", enable); }
bool ConfigManager::loadUseIMU(bool defaultValue) { return prefs.getBool("use_imu", defaultValue); }

// ==========================================
// Menerima Packet (wire format), simpan tipe data asli ke RAM
// ==========================================

void ConfigManager::saveIMUQuaternionOffset(QuaternionPacket packet) {
    uint8_t buffer[16];
    packet.getPacket(buffer);
    prefs.putBytes("IMU_quat_off", buffer, packet.size());
}

QuaternionPacket ConfigManager::loadIMUQuaternionOffset() {
    QuaternionPacket packet;
    if (prefs.getBytesLength("IMU_quat_off") == packet.size()) {
        uint8_t buffer[16];
        prefs.getBytes("IMU_quat_off", buffer, sizeof(buffer));
        packet.setPacket(buffer);
    }
    return packet;
}

void ConfigManager::saveQuaternionOffset(QuaternionPacket packet) {
    uint8_t buffer[16];
    packet.getPacket(buffer);
    prefs.putBytes("quat_off", buffer, packet.size());
}

QuaternionPacket ConfigManager::loadQuaternionOffset() {
    QuaternionPacket packet;
    if (prefs.getBytesLength("quat_off") == packet.size()) {
        uint8_t buffer[16];
        prefs.getBytes("quat_off", buffer, sizeof(buffer));
        packet.setPacket(buffer);
    }
    return packet;
}

void ConfigManager::saveVector3Offset(Vector3Packet packet) {
    uint8_t buffer[12];
    packet.getPacket(buffer);
    prefs.putBytes("vec3_off", buffer, packet.size());
}

Vector3Packet ConfigManager::loadVector3Offset() {
    Vector3Packet packet;
    if (prefs.getBytesLength("vec3_off") == packet.size()) {
        uint8_t buffer[12];
        prefs.getBytes("vec3_off", buffer, sizeof(buffer));
        packet.setPacket(buffer);
    }
    return packet;
}