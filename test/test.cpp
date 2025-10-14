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

struct SettingMemory {
  bool     ResetAll         = false;
  bool     IsInverted       = false;
  uint8_t  UnitsMask        = 0x3F;
  bool     MotorEnabled     = false;
  bool     MotorIsHalf      = false;
  uint32_t LimitNear        = 0xFFFFFFFF;
  uint32_t LimitHalf        = 0;
  uint32_t LimitFar         = 0;
  uint32_t Preset1          = 2496930;
  uint32_t Preset2          = 7490792;
  uint32_t Preset3          = 11236188;
  uint32_t CalDistance      = 3745396;
  int32_t  Calibration      = 0;
  bool     QCIsAllowed      = false;
  bool     IsDebugging      = false;
  bool     SecEnabled       = false;
  int32_t  OffsetSec        = 0;
  bool     SecIsInverted    = false;
  bool     KeyEnabled       = false;
  uint32_t MotorPulleyCirc  = 573288;
  uint16_t MotorRPM         = 100;
  uint16_t MotorAccel       = 100;
  uint16_t MotorErrorTol    = 4096;
  uint8_t  MotorTimeTol     = 10;
  int16_t  MotorOvrshtPos   = 0;
  int16_t  MotorOvrshtNeg   = 0;
  int16_t  MotorPulCompPos  = 0;
  int16_t  MotorPulCompNeg  = 0;
  bool     MotorCompEnabled = false;
  uint32_t MotorPulses      = 25600;
  uint32_t LifetimeTravel   = 0;
  uint16_t PosThreshold     = 12;
} __attribute__ ((__packed__));

void eeprom_read_block(void *__dst, const void * __src, size_t __n)
{
  // AVR EEPROM pointres are always 16-bit, so casting directly to uint16_t
  // does not result in a loss of precision but is disallowed by compiler.
  printf("%u\n", m24c08_read(EEPROM_I2C_PORT, (uint16_t)(uintptr_t)__src, (uint8_t *)__dst, __n));
}

void eeprom_update_block(const void *__src, void *__dst, size_t	__n)
{
    printf("%u\n", m24c08_update(EEPROM_I2C_PORT, (uintptr_t)__dst, (const uint8_t *)__src, __n));
}

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
    uint8_t read_data[79];

    if (!m24c08_read(EEPROM_I2C_PORT, 0, read_data, 79))
    {
        printf("Read failed!\n");
        return false;
    }

    for (int i = 0; i < 79; ++i) printf("%02X ", read_data[i]);
    printf("\n");

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

bool test_write_read_settings()
{
    SettingMemory settings;
    settings.KeyEnabled = true;
    uint8_t read_data[sizeof(settings)];
    SettingMemory readSettings;

    printf("test_write_read_settings: writing...");
    if (!m24c08_write(EEPROM_I2C_PORT, 0, (uint8_t*)&settings, sizeof(settings)))
    {
        printf("failed!\n");
        return false;
    }
    else
    {
        printf("SUCCESS!\n");
    }
    
    printf("test_write_read_settings: reading...");
    if (!m24c08_read(EEPROM_I2C_PORT, 0, (uint8_t*)&readSettings, sizeof(settings)))
    {
        printf("FAILED!\n");
        return false;
    }
    else
    {
        printf("SUCCESS!\n");
    }

    if (memcmp(&settings, &readSettings, sizeof(settings)) != 0)
    {
        printf("Data mismatch!\n");
        return false;
    }

    printf("test_write_read_settings PASSED\n");
    return true;
}

bool test_settings()
{
    SettingMemory settings;
    settings.KeyEnabled = true;
    uint8_t read_data[sizeof(settings)];
    SettingMemory readSettings;

    printf("test_settings: writing...");
    eeprom_update_block(&settings, 0, sizeof(settings));
    /*
    if (!m24c08_write(EEPROM_I2C_PORT, 0, write_data, sizeof(settings)))
    {
        printf("failed!\n");
        return false;
    }
    else
    {
        printf("SUCCESS!\n");
    }*/
    
    printf("test_settings: reading...");
    eeprom_read_block(&readSettings, 0, sizeof(settings));
    /* 
    if (!m24c08_read(EEPROM_I2C_PORT, 0, read_data, sizeof(settings)))
    {
        printf("FAILED!\n");
        return false;
    }
    else
    {
        printf("SUCCESS!\n");
    }
    */

    if (memcmp(&settings, &readSettings, sizeof(settings)) != 0)
    {
        printf("Data mismatch!\n");
        return false;
    }

    printf("test_settings PASSED\n");
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
    ok &= test_write_read_settings();
    busy_wait_ms(50);
    ok &= test_settings();
    busy_wait_ms(50);  // wait for EEPROM write cycle
    //ok &= test_byte_write_read();
    //busy_wait_ms(50);  // wait for EEPROM write cycle
    //ok &= test_write_read();
    //busy_wait_ms(50);  // wait for EEPROM write cycle
    //ok &= test_update();
    ok &= test_wraparound_read();

    if (ok) printf("ALL TESTS PASSED\n");
    else    printf("TESTS FAILED\n");

    while(1) tight_loop_contents;
}