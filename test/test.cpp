#include <stdio.h>
#include <cstring>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/bootrom.h"

#include "m24c08.h"

#define TEST_ADDR       0x010
#define TEST_LEN        16
#define EEPROM_I2C_PORT i2c1
#define EEPROM_SDA_PIN  2
#define EEPROM_SCL_PIN  3

static void i2c_init_eeprom()
{
    i2c_init(EEPROM_I2C_PORT, 100 * 1000);
    gpio_set_function(EEPROM_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(EEPROM_SCL_PIN, GPIO_FUNC_I2C);
    // gpio_pull_up(EEPROM_SDA_PIN);
    // gpio_pull_up(EEPROM_SCL_PIN);
    m24c08_init(EEPROM_I2C_PORT);
}

bool test_read()
{
    uint8_t read_data[TEST_LEN];

    if (!m24c08_read(EEPROM_I2C_PORT, 0, read_data, TEST_LEN))
    {
        printf("Read failed!\n");
        return false;
    }

    printf("test_read PASSED\n");
    return true;
}

bool test_byte_write_read()
{
    uint8_t write_data = 42;
    uint8_t read_data = 10;

    //if (!m24c08_write(EEPROM_I2C_PORT, 0x00, &write_data, 1))
    if (!m24c08_write(EEPROM_I2C_PORT, 0, write_data))
    {
        printf("Write byte failed!\n");
        return false;
    }

    busy_wait_ms(50);

    if (!m24c08_read(EEPROM_I2C_PORT, 0, &read_data, 1))
    {
        printf("Read byte failed!\n");
        return false;
    }

    if (memcmp(&write_data, &read_data, 1) != 0)
    {
        printf("Data mismatch!\n");
        printf("Wrote: %u\n", write_data);
        printf("Read: %u\n", read_data);
        return false;
    }

    printf("test_byte_write_read PASSED\n");
    return true;
}

bool test_write_read()
{
    uint8_t write_data[TEST_LEN];
    uint8_t read_data[TEST_LEN];

    for (int i = 0; i < TEST_LEN; ++i)
        write_data[i] = i;

    printf("test_write_read: writing...");
    if (!m24c08_write(EEPROM_I2C_PORT, 0, write_data, TEST_LEN))
    {
        printf("failed!\n");
        return false;
    }
    else
    {
        printf("SUCCESS!\n");
    }
    
    printf("test_write_read: reading...");
    if (!m24c08_read(EEPROM_I2C_PORT, 0, read_data, TEST_LEN))
    {
        printf("FAILED!\n");
        return false;
    }
    else
    {
        printf("SUCCESS!\n");
    }

    if (memcmp(write_data, read_data, TEST_LEN) != 0)
    {
        printf("Data mismatch!\n");
        return false;
    }

    printf("test_write_read PASSED\n");
    return true;
}

bool test_update()
{
    uint8_t write_data[TEST_LEN];
    uint8_t new_data[TEST_LEN];
    uint8_t read_data[TEST_LEN];

    for (int i = 0; i < TEST_LEN; ++i)
    {
        write_data[i] = 0xAA;
        new_data[i] = (i % 2) ? 0x55 : 0xAA;
    }

    // First write
    if (!m24c08_write(EEPROM_I2C_PORT, TEST_ADDR, write_data, TEST_LEN))
    {
        printf("Initial write failed\n");
        return false;
    }

    // Update with the same data (should skip write)
    if (!m24c08_update(EEPROM_I2C_PORT, TEST_ADDR, write_data, TEST_LEN))
    {
        printf("Update (no change) failed\n");
        return false;
    }

    // Update with new data (should write)
    if (!m24c08_update(EEPROM_I2C_PORT, TEST_ADDR, new_data, TEST_LEN))
    {
        printf("Update (with change) failed\n");
        return false;
    }

    if (!m24c08_read(EEPROM_I2C_PORT, TEST_ADDR, read_data, TEST_LEN))
    {
        printf("Read after update failed\n");
        return false;
    }

    if (memcmp(new_data, read_data, TEST_LEN) != 0)
    {
        printf("Update data mismatch\n");
        return false;
    }

    printf("test_update PASSED\n");
    return true;
}

bool test_wraparound_read()
{
    const uint16_t addr = 0x3F0;
    const uint8_t len = 32;
    uint8_t write_data[len];
    uint8_t read_data[len];

    for (int i = 0; i < len; ++i) write_data[i] = i + 0x10;

    if (!m24c08_write(EEPROM_I2C_PORT, addr, write_data, len))
    {
        printf("Wraparound write failed\n");
        return false;
    }

    if (!m24c08_read(EEPROM_I2C_PORT, addr, read_data, len))
    {
        printf("Wraparound read failed\n");
        return false;
    }

    if (memcmp(write_data, read_data, len) != 0)
    {
        printf("Wraparound data mismatch\n");
        return false;
    }

    printf("test_wraparound_read PASSED\n");
    return true;
}


int main()
{
    stdio_init_all();
    sleep_ms(5000);  // allow time for USB CDC connect
    printf("Starting M24C08 EEPROM tests...\n");

    i2c_init_eeprom();

    bool ok = true;
    ok &= test_read();
    ok &= test_byte_write_read();
    busy_wait_ms(50);  // wait for EEPROM write cycle
    ok &= test_write_read();
    busy_wait_ms(50);  // wait for EEPROM write cycle
    ok &= test_update();
    //ok &= test_wraparound_read();

    if (ok) printf("ALL TESTS PASSED\n");
    else    printf("TESTS FAILED\n");

    while(1) tight_loop_contents;
}