#include "ADS1115.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace stelle {

ADS1115::ADS1115(uint8_t address)
    : _address(address), _pga(PGA_4_096V), _dataRate(DR_860SPS), _isOk(false) {}

bool ADS1115::begin(TwoWire& wirePort, uint8_t address) {
    _wire = &wirePort;
    if (address != 0) {
        _address = address;
    }

    // Pastikan koneksi device ada di bus I2C
    if (!isConnected()) {
        _isOk = false;
        return false;
    }

    _isOk = true;
    return true;
}

bool ADS1115::isConnected() {
    if (!_wire) return false;
    _wire->beginTransmission(_address);
    return (_wire->endTransmission() == 0 || _wire->endTransmission() == 4);
}

void ADS1115::setPGA(PGA pga) { _pga = pga; }

void ADS1115::setDataRate(DataRate rate) { _dataRate = rate; }

float ADS1115::getMultiplier() const {
    switch (_pga) {
        case PGA_6_144V: return 0.0001875f;
        case PGA_4_096V: return 0.0001250f;
        case PGA_2_048V: return 0.0000625f;
        case PGA_1_024V: return 0.00003125f;
        case PGA_0_512V: return 0.000015625f;
        case PGA_0_256V: return 0.0000078125f;
        default:         return 0.0001250f;
    }
}

bool ADS1115::writeRegister(uint8_t reg, uint16_t value) {
    if (!_wire) return false;
    _wire->beginTransmission(_address);
    _wire->write(reg);
    _wire->write((uint8_t)(value >> 8));
    _wire->write((uint8_t)(value & 0xFF));
    return (_wire->endTransmission() == 0);
}

bool ADS1115::readRegister(uint8_t reg, uint16_t& value) {
    if (!_wire) return false;
    _wire->beginTransmission(_address);
    _wire->write(reg);
    if (_wire->endTransmission() != 0) {
        return false;
    }

    if (_wire->requestFrom(_address, (uint8_t)2) != 2) {
        return false;
    }

    uint8_t msb = _wire->read();
    uint8_t lsb = _wire->read();
    value = ((uint16_t)msb << 8) | lsb;
    return true;
}

int16_t ADS1115::readRaw(uint8_t channel) {
    if (channel > 3) return 0;

    uint16_t mux;
    switch (channel) {
        case 0: mux = CH_0; break;
        case 1: mux = CH_1; break;
        case 2: mux = CH_2; break;
        case 3: mux = CH_3; break;
        default: mux = CH_0; break;
    }

    // Config format:
    // OS[15] = 1 (Start single-shot conversion)
    // MUX[14:12] = channel vs GND
    // PGA[11:9] = set PGA
    // MODE[8] = 1 (Single-shot mode / power-down)
    // DR[7:5] = data rate
    // COMP_MODE[4], COMP_POL[3], COMP_LAT[2], COMP_QUE[1:0] = 0x0003 (Disable comparator)
    uint16_t config = 0x8000 | mux | _pga | 0x0100 | _dataRate | 0x0003;

    if (!writeRegister(REG_CONFIG, config)) {
        _isOk = false;
        return 0;
    }

    // Polling bit OS (bit 15): akan bernilai 1 jika konversi selesai
    uint16_t regVal = 0;
    bool completed = false;

    for (int waited = 0; waited < 20; waited++) {
        if (readRegister(REG_CONFIG, regVal)) {
            if (regVal & 0x8000) {
                completed = true;
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    if (!completed) {
        _isOk = false;
        return 0;
    }

    // Baca hasil konversi
    uint16_t rawResult = 0;
    if (!readRegister(REG_CONVERSION, rawResult)) {
        _isOk = false;
        return 0;
    }

    _isOk = true;
    return (int16_t)rawResult;
}

float ADS1115::readVoltage(uint8_t channel) {
    int16_t raw = readRaw(channel);
    if (raw < 0) raw = 0; // Single-ended vs GND tidak bernilai negatif
    return (float)raw * getMultiplier();
}

} // namespace stelle
