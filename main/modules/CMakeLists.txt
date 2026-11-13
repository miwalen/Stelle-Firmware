# Component: modules
# Application logic & FreeRTOS tasks

idf_component_register(
    SRCS
        "Communication.cpp"
        "ConfigManager.cpp"
        "ForwardKinematicsComputer.cpp"
        "PowerManager.cpp"
        "SensorReader.cpp"
    INCLUDE_DIRS
        "."
    REQUIRES
        drivers
        core
        driver
        nvs_flash
        log
        freertos
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Wno-unused-parameter)
