# Component: core
# Math, packets serialization, & logging utilities

idf_component_register(
    SRCS
        "Logger.cpp"
        "MechLib.cpp"
    INCLUDE_DIRS
        "."
    REQUIRES
        log
        freertos
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Wno-unused-parameter)
