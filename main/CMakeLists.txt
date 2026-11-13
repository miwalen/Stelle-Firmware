idf_component_register(
    SRCS
        # Entrypoint & Setup
        "setup.cpp"

        # Drivers & Hardware Wrappers
        "drivers/TwoWire.cpp"
        "drivers/HardwareSerial.cpp"
        "drivers/Preferences.cpp"
        "drivers/ADS1115.cpp"

        # Core Utilities, Math, & Synchronization
        "core/Logger.cpp"
        "core/MechLib.cpp"

        # Application Tasks & Business Modules
        "modules/Communication.cpp"
        "modules/ConfigManager.cpp"
        "modules/ForwardKinematicsComputer.cpp"
        "modules/PowerManager.cpp"
        "modules/SensorReader.cpp"
    INCLUDE_DIRS
        "."
        "drivers"
        "core"
        "modules"
    REQUIRES
        driver
        nvs_flash
        log
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Wno-unused-parameter)
