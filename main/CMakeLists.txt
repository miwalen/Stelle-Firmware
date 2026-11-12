# Satu file per komponen Arduino, tidak ada penggabungan.
idf_component_register(
    SRCS
        "setup.cpp"
        "TwoWire.cpp"
        "HardwareSerial.cpp"
        "ADS1115.cpp"
        "Communication.cpp"
        "ConfigManager.cpp"
        "ForwardKinematicsComputer.cpp"
        "Logger.cpp"
        "MechLib.cpp"
        "Mutexes.cpp"
        "PowerManager.cpp"
        "SensorReader.cpp"
    INCLUDE_DIRS
        "."
    REQUIRES
        driver
        nvs_flash
        log
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Wno-unused-parameter)