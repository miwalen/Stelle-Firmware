#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <cstddef>
#include <cstdint>
#include "nvs.h"

namespace stelle {

/**
 * Preferences - Wrapper NVS ESP-IDF yang kompatibel dengan API Preferences Arduino ESP32.
 * 
 * Penggunaan:
 *   Preferences prefs;
 *   prefs.begin("my_ns", false);
 *   prefs.putString("ssid", "MyWiFi");
 *   char buf[32];
 *   prefs.getString("ssid", buf, sizeof(buf), "defaultSSID");
 *   prefs.end();
 */
class Preferences {
public:
    Preferences();
    ~Preferences();

    Preferences(const Preferences&) = delete;
    Preferences& operator=(const Preferences&) = delete;

    bool begin(const char* name, bool readOnly = false, const char* partition_label = nullptr);
    void end();

    bool clear();
    bool remove(const char* key);
    bool isKey(const char* key);

    // Putters
    size_t putChar(const char* key, int8_t value);
    size_t putUChar(const char* key, uint8_t value);
    size_t putShort(const char* key, int16_t value);
    size_t putUShort(const char* key, uint16_t value);
    size_t putInt(const char* key, int32_t value);
    size_t putUInt(const char* key, uint32_t value);
    size_t putLong(const char* key, int32_t value);
    size_t putULong(const char* key, uint32_t value);
    size_t putLong64(const char* key, int64_t value);
    size_t putULong64(const char* key, uint64_t value);
    size_t putFloat(const char* key, float value);
    size_t putDouble(const char* key, double value);
    size_t putBool(const char* key, bool value);
    size_t putString(const char* key, const char* value);
    size_t putBytes(const char* key, const void* value, size_t len);

    // Getters
    int8_t   getChar(const char* key, int8_t defaultValue = 0);
    uint8_t  getUChar(const char* key, uint8_t defaultValue = 0);
    int16_t  getShort(const char* key, int16_t defaultValue = 0);
    uint16_t getUShort(const char* key, uint16_t defaultValue = 0);
    int32_t  getInt(const char* key, int32_t defaultValue = 0);
    uint32_t getUInt(const char* key, uint32_t defaultValue = 0);
    int32_t  getLong(const char* key, int32_t defaultValue = 0);
    uint32_t getULong(const char* key, uint32_t defaultValue = 0);
    int64_t  getLong64(const char* key, int64_t defaultValue = 0);
    uint64_t getULong64(const char* key, uint64_t defaultValue = 0);
    float    getFloat(const char* key, float defaultValue = NAN);
    double   getDouble(const char* key, double defaultValue = NAN);
    bool     getBool(const char* key, bool defaultValue = false);
    
    // getString mengisi buffer C-string
    size_t   getString(const char* key, char* value, size_t maxLen, const char* defaultValue = "");
    size_t   getBytes(const char* key, void* buf, size_t maxLen);
    size_t   getBytesLength(const char* key);

private:
    nvs_handle_t _handle = 0;
    bool _readOnly = false;
    bool _started = false;
};

} // namespace stelle

#endif // PREFERENCES_H
