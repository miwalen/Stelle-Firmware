#include "HardwareSerial.h"

#include "freertos/FreeRTOS.h"

namespace stelle {

namespace {
constexpr TickType_t READ_TIMEOUT = pdMS_TO_TICKS(1);
constexpr TickType_t WRITE_TIMEOUT = pdMS_TO_TICKS(100);

uart_word_length_t dataBitsFrom(uint32_t config) {
    switch ((config >> 0) & 0x3) {
        case 0:  return UART_DATA_5_BITS;
        case 1:  return UART_DATA_6_BITS;
        case 2:  return UART_DATA_7_BITS;
        default: return UART_DATA_8_BITS;
    }
}

uart_parity_t parityFrom(uint32_t config) {
    // bit 8-9: 0 = none, 1 = even, 2 = odd
    switch ((config >> 8) & 0x3) {
        case 1:  return UART_PARITY_EVEN;
        case 2:  return UART_PARITY_ODD;
        default: return UART_PARITY_DISABLE;
    }
}

uart_stop_bits_t stopBitsFrom(uint32_t config) {
    return ((config >> 4) & 0x1) ? UART_STOP_BITS_2 : UART_STOP_BITS_1;
}
}  // namespace

HardwareSerial::HardwareSerial(int uartNum) : _uartNum(uartNum) {}

HardwareSerial::~HardwareSerial() { end(); }

void HardwareSerial::setRxBufferSize(size_t size) { _rxBufferSize = size; }

bool HardwareSerial::begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin) {
    if (_begun) return true;

    const uart_port_t port = static_cast<uart_port_t>(_uartNum);

    if (uart_driver_install(port, _rxBufferSize, 1024, 0, nullptr, 0) != ESP_OK) {
        return false;
    }

    uart_config_t cfg = {};
    cfg.baud_rate = static_cast<int>(baud);
    cfg.data_bits = dataBitsFrom(config);
    cfg.parity = parityFrom(config);
    cfg.stop_bits = stopBitsFrom(config);
    cfg.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    cfg.source_clk = UART_SCLK_DEFAULT;

    if (uart_param_config(port, &cfg) != ESP_OK) return false;

    const int tx = (txPin >= 0) ? txPin : UART_PIN_NO_CHANGE;
    const int rx = (rxPin >= 0) ? rxPin : UART_PIN_NO_CHANGE;
    if (uart_set_pin(port, tx, rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK) {
        return false;
    }

    _begun = true;
    return true;
}

void HardwareSerial::end() {
    if (!_begun) return;
    uart_driver_delete(static_cast<uart_port_t>(_uartNum));
    _peekedByte = -1;
    _begun = false;
}

int HardwareSerial::available() {
    if (!_begun) return 0;
    size_t buffered = 0;
    if (uart_get_buffered_data_len(static_cast<uart_port_t>(_uartNum), &buffered) != ESP_OK) {
        return 0;
    }
    return static_cast<int>(buffered) + (_peekedByte >= 0 ? 1 : 0);
}

int HardwareSerial::read() {
    if (!_begun) return -1;
    if (_peekedByte >= 0) {
        const int byte = _peekedByte;
        _peekedByte = -1;
        return byte;
    }

    uint8_t byte = 0;
    const int got = uart_read_bytes(static_cast<uart_port_t>(_uartNum), &byte, 1, READ_TIMEOUT);
    return got == 1 ? static_cast<int>(byte) : -1;
}

int HardwareSerial::peek() {
    if (!_begun) return -1;
    if (_peekedByte >= 0) return _peekedByte;

    uint8_t byte = 0;
    const int got = uart_read_bytes(static_cast<uart_port_t>(_uartNum), &byte, 1, READ_TIMEOUT);
    if (got != 1) return -1;
    _peekedByte = byte;
    return _peekedByte;
}

size_t HardwareSerial::write(uint8_t value) {
    if (!_begun) return 0;
    return uart_write_bytes(static_cast<uart_port_t>(_uartNum), &value, 1) == 1 ? 1 : 0;
}

size_t HardwareSerial::write(const uint8_t* data, size_t len) {
    if (!_begun || data == nullptr || len == 0) return 0;
    const int sent = uart_write_bytes(static_cast<uart_port_t>(_uartNum), data, len);
    return sent > 0 ? static_cast<size_t>(sent) : 0;
}

void HardwareSerial::flush() {
    if (!_begun) return;
    uart_wait_tx_done(static_cast<uart_port_t>(_uartNum), WRITE_TIMEOUT);
}

} // namespace stelle
