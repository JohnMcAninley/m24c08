#ifndef __M24C08_HPP__
#define __M24C08_HPP__

#include <cstddef>
#include <stdint.h>

#include "hardware/i2c.h"

#define M24C08_BASE_ADDR 0x50  // Base I2C address for block 0
#define M24C08_PAGE_SIZE 16   // bytes per block
#define M24C08_TOTAL_SIZE 1024  // total EEPROM size in bytes
#define EEPROM_WRITE_CYCLE_DELAY_MS 5


void m24c08_init(i2c_inst_t *i2c);

bool m24c08_write(i2c_inst_t *i2c, uint16_t addr, const uint8_t *data, size_t len);

bool m24c08_update(i2c_inst_t *i2c, uint16_t addr, const uint8_t *data, size_t len);

bool m24c08_read(i2c_inst_t *i2c, uint16_t addr, uint8_t *dst, size_t len);


#endif /* __M24C08_HPP__ */