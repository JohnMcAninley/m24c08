cmake_minimum_required(VERSION 3.13)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)

message(${PICO_SDK_PATH})

set(PICO_SDK_PATH "C:/Program Files/Raspberry Pi/Pico SDK v1.5.1/pico-sdk")

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(m24c08)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
#add_compile_definitions(RP2040)

#set(CMOTION_SOURCE_DIR ${CMOTION_DIR}/source/)

add_subdirectory(test)


#file (STRINGS "MYT_Works_key.auth" MYT_AUTH_KEY)
#target_compile_definitions(og_master PUBLIC MYT_AUTH_KEY="\""${MYT_AUTH_KEY}"\"")

#pico_enable_stdio_usb(m24c08 1)
#pico_enable_stdio_uart(m24c08 0)

#pico_add_extra_outputs(m24c08)