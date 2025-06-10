add_executable(m24c08_test
    test.cpp
    ../m24c08.cpp
)

target_link_libraries(m24c08_test pico_stdlib hardware_i2c)

include_directories(..)

pico_enable_stdio_usb(m24c08_test 1)
pico_enable_stdio_uart(m24c08_test 0)

pico_add_extra_outputs(m24c08_test)