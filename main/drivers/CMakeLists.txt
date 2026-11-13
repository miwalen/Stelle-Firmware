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
        nvs_flash
        log
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Wno-unused-parameter)
