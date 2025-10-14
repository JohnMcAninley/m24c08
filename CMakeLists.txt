cmake_minimum_required(VERSION 3.13)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(m24c08)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_library(m24c08 m24c08.cpp m24c08.h)

target_include_directories(m24c08 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(m24c08 hardware_i2c)

add_subdirectory(test)

#pico_enable_stdio_usb(m24c08 1)
#pico_enable_stdio_uart(m24c08 0)

#pico_add_extra_outputs(m24c08)