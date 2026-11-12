#include "TwoWire.h"

#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"

namespace stelle {

namespace {
constexpr int I2C_TIMEOUT_MS = 50;
constexpr size_t WIRE_BUFFER = 64;
}  // namespace

struct TwoWire::BusHandle {
    i2c_master_bus_handle_t bus = nullptr;
    i2c_master_dev_handle_t dev = nullptr;
    uint8_t devAddress = 0;
    uint32_t frequency = 100000;
};

TwoWire Wire1;

TwoWire::~TwoWire() { end(); }

bool TwoWire::begin(int pinSda, int pinScl, uint32_t frequency) {
    if (_bus != nullptr) return true;

    _bus = new BusHandle();
    _bus->frequency = frequency;

    i2c_master_bus_config_t cfg = {};
    cfg.i2c_port = I2C_NUM_0;
    cfg.sda_io_num = static_cast<gpio_num_t>(pinSda);
    cfg.scl_io_num = static_cast<gpio_num_t>(pinScl);
    cfg.clk_source = I2C_CLK_SRC_DEFAULT;
    cfg.glitch_ignore_cnt = 7;
    cfg.flags.enable_internal_pullup = false;

    if (i2c_new_master_bus(&cfg, &_bus->bus) != ESP_OK) {
        delete _bus;
        _bus = nullptr;
        return false;
    }
    return true;
}

void TwoWire::end() {
    if (_bus == nullptr) return;

    if (_bus->dev != nullptr) {
        i2c_master_bus_rm_device(_bus->dev);
        _bus->dev = nullptr;
    }
    if (_bus->bus != nullptr) {
        i2c_del_master_bus(_bus->bus);
        _bus->bus = nullptr;
    }
    delete _bus;
    _bus = nullptr;
    _dev = nullptr;
}

bool TwoWire::ensureDevice(uint8_t address) {
    if (_bus == nullptr || _bus->bus == nullptr) return false;
    if (_bus->dev != nullptr && _bus->devAddress == address) return true;

    if (_bus->dev != nullptr) {
        i2c_master_bus_rm_device(_bus->dev);
        _bus->dev = nullptr;
    }

    i2c_device_config_t devCfg = {};
    devCfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    devCfg.device_address = address;
    devCfg.scl_speed_hz = _bus->frequency;

    if (i2c_master_bus_add_device(_bus->bus, &devCfg, &_bus->dev) != ESP_OK) {
        _bus->dev = nullptr;
        return false;
    }

    _bus->devAddress = address;
    _dev = _bus->dev;
    _address = address;
    return true;
}

// ==========================================
// Transmit
// ==========================================
void TwoWire::beginTransmission(uint8_t address) {
    _address = address;
    _txLength = 0;
}

size_t TwoWire::write(uint8_t value) {
    if (_txLength >= WIRE_BUFFER) return 0;
    _txBuffer[_txLength++] = value;
    return 1;
}

size_t TwoWire::write(const uint8_t* data, size_t len) {
    size_t written = 0;
    while (written < len && _txLength < WIRE_BUFFER) {
        _txBuffer[_txLength++] = data[written++];
    }
    return written;
}

// Kode balik sama artinya dengan Wire Arduino:
//   0 = sukses, 1 = data terlalu panjang, 2 = NACK alamat, 3 = NACK data
uint8_t TwoWire::endTransmission(bool sendStop) {
    (void)sendStop;  // i2c_master selalu mengirim STOP untuk transfer penuh

    if (_txLength == 0) return 4;  // tidak ada yang dikirim
    if (!ensureDevice(_address)) return 2;

    const esp_err_t err = i2c_master_transmit(_bus->dev, _txBuffer, _txLength, I2C_TIMEOUT_MS);
    _txLength = 0;

    if (err == ESP_OK) return 0;
    if (err == ESP_ERR_TIMEOUT) return 5;  // timeout
    return 2;                              // NACK / perangkat tidak menjawab
}

// ==========================================
// Receive
// ==========================================
uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity, bool sendStop) {
    (void)sendStop;

    if (quantity > WIRE_BUFFER) quantity = WIRE_BUFFER;
    _rxLength = 0;
    _rxIndex = 0;

    if (!ensureDevice(address)) return 0;

    esp_err_t err;
    if (_txLength > 0) {
        // Register-address style read: tulis pointer register lalu baca.
        err = i2c_master_transmit_receive(_bus->dev, _txBuffer, _txLength, _rxBuffer,
                                          quantity, I2C_TIMEOUT_MS);
        _txLength = 0;
    } else {
        err = i2c_master_receive(_bus->dev, _rxBuffer, quantity, I2C_TIMEOUT_MS);
    }

    if (err != ESP_OK) return 0;
    _rxLength = quantity;
    return quantity;
}

int TwoWire::read() {
    if (_rxIndex >= _rxLength) return -1;
    return _rxBuffer[_rxIndex++];
}

int TwoWire::peek() const {
    if (_rxIndex >= _rxLength) return -1;
    return _rxBuffer[_rxIndex];
}

} // namespace stelle
