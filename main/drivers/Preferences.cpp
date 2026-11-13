#include "Preferences.h"

#include <cmath>
#include <cstring>
#include "esp_err.h"
#include "nvs_flash.h"

namespace stelle {

namespace {
bool s_nvsInitialized = false;
}

bool Preferences::initStorage() {
    if (s_nvsInitialized) return true;

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    if (err == ESP_OK) {
        s_nvsInitialized = true;
        return true;
    }
    return false;
}

Preferences::Preferences() : _handle(0), _readOnly(false), _started(false) {}

Preferences::~Preferences() {
    end();
}

bool Preferences::begin(const char* name, bool readOnly, const char* partition_label) {
    if (_started) {
        return false;
    }

    // Pastikan storage NVS sudah terinisialisasi secara transparan
    if (!s_nvsInitialized && !initStorage()) {
        return false;
    }

    _readOnly = readOnly;
    esp_err_t err;

    if (partition_label != nullptr) {
        err = nvs_open_from_partition(partition_label, name, readOnly ? NVS_READONLY : NVS_READWRITE, &_handle);
    } else {
        err = nvs_open(name, readOnly ? NVS_READONLY : NVS_READWRITE, &_handle);
    }

    if (err != ESP_OK) {
        _handle = 0;
        _started = false;
        return false;
    }

    _started = true;
    return true;
}

void Preferences::end() {
    if (!_started) return;
    if (!_readOnly) {
        nvs_commit(_handle);
    }
    nvs_close(_handle);
    _handle = 0;
    _started = false;
}

bool Preferences::clear() {
    if (!_started || _readOnly) return false;
    esp_err_t err = nvs_erase_all(_handle);
    if (err == ESP_OK) {
        nvs_commit(_handle);
        return true;
    }
    return false;
}

bool Preferences::remove(const char* key) {
    if (!_started || _readOnly || key == nullptr) return false;
    esp_err_t err = nvs_erase_key(_handle, key);
    if (err == ESP_OK) {
        nvs_commit(_handle);
        return true;
    }
    return false;
}

bool Preferences::isKey(const char* key) {
    if (!_started || key == nullptr) return false;
    uint8_t dummy;
    size_t len = sizeof(dummy);
    // Mencoba membaca data sebagai blob 1 byte atau type check
    nvs_type_t type;
    esp_err_t err = nvs_find_key(_handle, key, &type);
    return (err == ESP_OK);
}

// ============================================================
// Putters
// ============================================================
size_t Preferences::putChar(const char* key, int8_t value) {
    if (!_started || _readOnly) return 0;
    esp_err_t err = nvs_set_i8(_handle, key, value);
    if (err == ESP_OK) { nvs_commit(_handle); return sizeof(value); }
    return 0;
}

size_t Preferences::putUChar(const char* key, uint8_t value) {
    if (!_started || _readOnly) return 0;
    esp_err_t err = nvs_set_u8(_handle, key, value);
    if (err == ESP_OK) { nvs_commit(_handle); return sizeof(value); }
    return 0;
}

size_t Preferences::putShort(const char* key, int16_t value) {
    if (!_started || _readOnly) return 0;
    esp_err_t err = nvs_set_i16(_handle, key, value);
    if (err == ESP_OK) { nvs_commit(_handle); return sizeof(value); }
    return 0;
}

size_t Preferences::putUShort(const char* key, uint16_t value) {
    if (!_started || _readOnly) return 0;
    esp_err_t err = nvs_set_u16(_handle, key, value);
    if (err == ESP_OK) { nvs_commit(_handle); return sizeof(value); }
    return 0;
}

size_t Preferences::putInt(const char* key, int32_t value) {
    if (!_started || _readOnly) return 0;
    esp_err_t err = nvs_set_i32(_handle, key, value);
    if (err == ESP_OK) { nvs_commit(_handle); return sizeof(value); }
    return 0;
}

size_t Preferences::putUInt(const char* key, uint32_t value) {
    if (!_started || _readOnly) return 0;
    esp_err_t err = nvs_set_u32(_handle, key, value);
    if (err == ESP_OK) { nvs_commit(_handle); return sizeof(value); }
    return 0;
}

size_t Preferences::putLong(const char* key, int32_t value) {
    return putInt(key, value);
}

size_t Preferences::putULong(const char* key, uint32_t value) {
    return putUInt(key, value);
}

size_t Preferences::putLong64(const char* key, int64_t value) {
    if (!_started || _readOnly) return 0;
    esp_err_t err = nvs_set_i64(_handle, key, value);
    if (err == ESP_OK) { nvs_commit(_handle); return sizeof(value); }
    return 0;
}

size_t Preferences::putULong64(const char* key, uint64_t value) {
    if (!_started || _readOnly) return 0;
    esp_err_t err = nvs_set_u64(_handle, key, value);
    if (err == ESP_OK) { nvs_commit(_handle); return sizeof(value); }
    return 0;
}

size_t Preferences::putFloat(const char* key, float value) {
    return putBytes(key, &value, sizeof(value));
}

size_t Preferences::putDouble(const char* key, double value) {
    return putBytes(key, &value, sizeof(value));
}

size_t Preferences::putBool(const char* key, bool value) {
    return putUChar(key, value ? 1 : 0);
}

size_t Preferences::putString(const char* key, const char* value) {
    if (!_started || _readOnly || key == nullptr || value == nullptr) return 0;
    esp_err_t err = nvs_set_str(_handle, key, value);
    if (err == ESP_OK) {
        nvs_commit(_handle);
        return std::strlen(value);
    }
    return 0;
}

size_t Preferences::putBytes(const char* key, const void* value, size_t len) {
    if (!_started || _readOnly || key == nullptr || value == nullptr || len == 0) return 0;
    esp_err_t err = nvs_set_blob(_handle, key, value, len);
    if (err == ESP_OK) {
        nvs_commit(_handle);
        return len;
    }
    return 0;
}

// ============================================================
// Getters
// ============================================================
int8_t Preferences::getChar(const char* key, int8_t defaultValue) {
    int8_t val = defaultValue;
    if (!_started || key == nullptr) return val;
    nvs_get_i8(_handle, key, &val);
    return val;
}

uint8_t Preferences::getUChar(const char* key, uint8_t defaultValue) {
    uint8_t val = defaultValue;
    if (!_started || key == nullptr) return val;
    nvs_get_u8(_handle, key, &val);
    return val;
}

int16_t Preferences::getShort(const char* key, int16_t defaultValue) {
    int16_t val = defaultValue;
    if (!_started || key == nullptr) return val;
    nvs_get_i16(_handle, key, &val);
    return val;
}

uint16_t Preferences::getUShort(const char* key, uint16_t defaultValue) {
    uint16_t val = defaultValue;
    if (!_started || key == nullptr) return val;
    nvs_get_u16(_handle, key, &val);
    return val;
}

int32_t Preferences::getInt(const char* key, int32_t defaultValue) {
    int32_t val = defaultValue;
    if (!_started || key == nullptr) return val;
    nvs_get_i32(_handle, key, &val);
    return val;
}

uint32_t Preferences::getUInt(const char* key, uint32_t defaultValue) {
    uint32_t val = defaultValue;
    if (!_started || key == nullptr) return val;
    nvs_get_u32(_handle, key, &val);
    return val;
}

int32_t Preferences::getLong(const char* key, int32_t defaultValue) {
    return getInt(key, defaultValue);
}

uint32_t Preferences::getULong(const char* key, uint32_t defaultValue) {
    return getUInt(key, defaultValue);
}

int64_t Preferences::getLong64(const char* key, int64_t defaultValue) {
    int64_t val = defaultValue;
    if (!_started || key == nullptr) return val;
    nvs_get_i64(_handle, key, &val);
    return val;
}

uint64_t Preferences::getULong64(const char* key, uint64_t defaultValue) {
    uint64_t val = defaultValue;
    if (!_started || key == nullptr) return val;
    nvs_get_u64(_handle, key, &val);
    return val;
}

float Preferences::getFloat(const char* key, float defaultValue) {
    float val = defaultValue;
    if (getBytes(key, &val, sizeof(val)) == sizeof(val)) {
        return val;
    }
    return defaultValue;
}

double Preferences::getDouble(const char* key, double defaultValue) {
    double val = defaultValue;
    if (getBytes(key, &val, sizeof(val)) == sizeof(val)) {
        return val;
    }
    return defaultValue;
}

bool Preferences::getBool(const char* key, bool defaultValue) {
    return getUChar(key, defaultValue ? 1 : 0) != 0;
}

size_t Preferences::getString(const char* key, char* value, size_t maxLen, const char* defaultValue) {
    if (value == nullptr || maxLen == 0) return 0;
    
    // Set default value dulu
    std::memset(value, 0, maxLen);
    if (defaultValue != nullptr) {
        std::strncpy(value, defaultValue, maxLen - 1);
        value[maxLen - 1] = '\0';
    }

    if (!_started || key == nullptr) {
        return std::strlen(value);
    }

    size_t reqLen = 0;
    esp_err_t err = nvs_get_str(_handle, key, nullptr, &reqLen);
    if (err == ESP_OK && reqLen > 0) {
        if (reqLen > maxLen) {
            reqLen = maxLen;
        }
        err = nvs_get_str(_handle, key, value, &reqLen);
        if (err == ESP_OK) {
            value[maxLen - 1] = '\0';
            return std::strlen(value);
        }
    }
    return std::strlen(value);
}

size_t Preferences::getBytesLength(const char* key) {
    if (!_started || key == nullptr) return 0;
    size_t len = 0;
    if (nvs_get_blob(_handle, key, nullptr, &len) == ESP_OK) {
        return len;
    }
    return 0;
}

size_t Preferences::getBytes(const char* key, void* buf, size_t maxLen) {
    if (!_started || key == nullptr || buf == nullptr || maxLen == 0) return 0;
    size_t len = maxLen;
    esp_err_t err = nvs_get_blob(_handle, key, buf, &len);
    if (err == ESP_OK) {
        return len;
    }
    return 0;
}

} // namespace stelle
