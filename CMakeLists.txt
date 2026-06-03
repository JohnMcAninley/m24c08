cmake_minimum_required(VERSION 3.13)

if(NOT TARGET pico_stdlib)
    include(pico_sdk_import.cmake)
    project(m24c08)
    pico_sdk_init()
endif()

add_library(m24c08 m24c08.cpp m24c08.h)

target_include_directories(m24c08 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(m24c08 hardware_i2c)

add_subdirectory(test)