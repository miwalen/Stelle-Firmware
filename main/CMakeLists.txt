idf_component_register(
    SRCS
        "setup.cpp"
    INCLUDE_DIRS
        "."
    REQUIRES
        drivers
        core
        modules
        driver
        nvs_flash
        log
        freertos
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Wno-unused-parameter)
