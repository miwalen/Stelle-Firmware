cmake_minimum_required(VERSION 3.16)

# Daftarkan folder komponen tambahan di bawah main/
set(EXTRA_COMPONENT_DIRS 
    "${CMAKE_CURRENT_LIST_DIR}/main/drivers"
    "${CMAKE_CURRENT_LIST_DIR}/main/core"
    "${CMAKE_CURRENT_LIST_DIR}/main/modules"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(stelle)
