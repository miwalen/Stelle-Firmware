# Component: drivers
# Reusable hardware drivers & peripherals wrapper

idf_component_register(
    SRCS
        "TwoWire.cpp"
        "HardwareSerial.cpp"
        "Preferences.cpp"
        "ADS1115.cpp"
    INCLUDE_DIRS
        "."
    REQUIRES
        driver
        esp_driver_i2c
        esp_driver_uart
        core
        nvs_flash
        log
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Wno-unused-parameter)
