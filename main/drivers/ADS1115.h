#ifndef ADS1115_H
#define ADS1115_H

#include <cstdint>

#include "TwoWire.h"

namespace stelle {

class ADS1115 {
public:
    // Default I2C Addresses
    enum Address : uint8_t {
        ADDR_GND = 0x48,
        ADDR_VDD = 0x49,
        ADDR_SDA = 0x4A,
        ADDR_SCL = 0x4B
    };

    // Programmable Gain Amplifier (PGA)
    // ESP32 bekerja pada 3.3V, PGA_4_096V (FSR +/-4.096V) adalah opsi standar teraman
    enum PGA : uint16_t {
        PGA_6_144V = 0x0000, // +/-6.144V (1 bit = 0.1875mV)
        PGA_4_096V = 0x0200, // +/-4.096V (1 bit = 0.125mV) - Default ideal untuk 3.3V
        PGA_2_048V = 0x0400, // +/-2.048V (1 bit = 0.0625mV)
        PGA_1_024V = 0x0600, // +/-1.024V (1 bit = 0.03125mV)
        PGA_0_512V = 0x0800, // +/-0.512V (1 bit = 0.015625mV)
        PGA_0_256V = 0x0A00  // +/-0.256V (1 bit = 0.0078125mV)
    };

    // Data Rate (Samples Per Second)
    enum DataRate : uint16_t {
        DR_128SPS = 0x0080,
        DR_250SPS = 0x00A0,
        DR_475SPS = 0x00C0,
        DR_860SPS = 0x00E0  // Pembacaan cepat
    };

    // Multiplexer Selection (Single-ended channel vs GND)
    enum Channel : uint16_t {
        CH_0 = 0x4000, // AIN0 vs GND
        CH_1 = 0x5000, // AIN1 vs GND
        CH_2 = 0x6000, // AIN2 vs GND
        CH_3 = 0x7000  // AIN3 vs GND
    };

    ADS1115(uint8_t address = ADDR_GND);

    // Inisialisasi Wire & Address
    bool begin(TwoWire& wirePort = Wire1, uint8_t address = 0);

    // Status checks
    bool isConnected();
    bool isOk() const { return _isOk; }

    // Konfigurasi
    void setPGA(PGA pga);
    void setDataRate(DataRate rate);
    PGA getPGA() const { return _pga; }
    DataRate getDataRate() const { return _dataRate; }

    // Pembacaan Data
    int16_t readRaw(uint8_t channel);
    float readVoltage(uint8_t channel);

private:
    uint8_t _address;
    TwoWire* _wire;
    PGA _pga;
    DataRate _dataRate;
    bool _isOk;

    // Register Pointers
    static const uint8_t REG_CONVERSION = 0x00;
    static const uint8_t REG_CONFIG     = 0x01;

    bool writeRegister(uint8_t reg, uint16_t value);
    bool readRegister(uint8_t reg, uint16_t& value);
    float getMultiplier() const;
};

} // namespace stelle

#endif // ADS1115_H
