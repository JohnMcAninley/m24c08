#include <cstring>

#include "hardware/i2c.h"
#include "pico/time.h"

#include "drivers/m24c08.h"


void m24c08_init(i2c_inst_t *i2c) {
  // Nothing to initialize for this EEPROM,
  // but this function is here for future expansion or if needed.
  (void)i2c;
}

static inline uint8_t get_page_offset(uint16_t mem_addr)
{
  return mem_addr % M24C08_PAGE_SIZE;
}

uint8_t deviceSelect(uint16_t addr, uint8_t device_id=0)
{
  return M24C08_BASE_ADDR | ((device_id & 1) << 2) | ((addr >> 8) & 0b11);
}

uint8_t address(uint16_t addr)
{
  return static_cast<uint8_t>(addr & 0xFF);
}

bool write_address(i2c_inst_t *i2c, uint16_t addr)
{
  uint8_t device_select = deviceSelect(addr);
  uint8_t addr_l = address(addr);
  int ret = i2c_write_blocking(i2c, device_select, &addr_l, 1, true);
  return ret == 1;
}

bool m24c08_write(i2c_inst_t *i2c, uint16_t addr, const uint8_t *data, size_t len)
{
  if ((addr + len) > M24C08_TOTAL_SIZE) return false;

  while (len > 0)
  {
    // Write device select and address to begin page write
    uint8_t device_select = deviceSelect(addr);
    uint8_t addr_l = address(addr);
    int ret = i2c_write_blocking(i2c, device_select, &addr_l, 1, true);
    if (ret != 1) return false;

    // Calculate how many bytes we can write in current block
    uint8_t offset = get_page_offset(addr);
    size_t chunk = M24C08_PAGE_SIZE - offset;
    if (chunk > len) chunk = len;

    ret = i2c_write_blocking(i2c, device_select, data, chunk, false);
    if (ret != chunk) return false; // Write failed

    // Wait for the EEPROM internal write cycle to complete (~5 ms typical, 10 ms safe)
    busy_wait_ms(EEPROM_WRITE_CYCLE_DELAY_MS);

    addr += chunk;
    data += chunk;
    len -= chunk;
  }

  return true;
}

bool m24c08_update(i2c_inst_t *i2c, uint16_t addr, const uint8_t *data, size_t len)
{
  if ((addr + len) > M24C08_TOTAL_SIZE) return false;

  while (len > 0)
  {
    // Write device select and address to begin page write
    uint8_t device_select = deviceSelect(addr);
    uint8_t addr_l = address(addr);
    int ret = i2c_write_blocking(i2c, device_select, &addr_l, 1, true);
    if (ret != 1) return false;

    // Calculate how many bytes we can write in current block
    uint8_t offset = get_page_offset(addr);
    size_t chunk = M24C08_PAGE_SIZE - offset;
    if (chunk > len) chunk = len;

    uint8_t buf[chunk];
    if (!m24c08_read(i2c, addr, buf, len)) return false;

    if (memcmp(data, buf, chunk))
    {
      ret = i2c_write_blocking(i2c, device_select, data, chunk, false);
      if (ret != chunk) return false; // Write failed

      // Wait for the EEPROM internal write cycle to complete (~5 ms typical, 10 ms safe)
      busy_wait_ms(EEPROM_WRITE_CYCLE_DELAY_MS);
    }

    addr += chunk;
    data += chunk;
    len -= chunk;
  }

  return true;
}

bool m24c08_read(i2c_inst_t *i2c, uint16_t addr, uint8_t *dst, size_t len)
{
  // Bounds check on the address, DOES NOT allow A10 to be used to select device
  // NOTE addr + len is allowed to exceed 0x3FF because the read will wrap around
  if (addr > 0x3FF) return false;

  uint8_t device_select = deviceSelect(addr);
  uint8_t addr_l = address(addr);
  int ret = i2c_write_blocking(i2c, device_select, &addr_l, 1, true);
  if (ret != 1) return false;

  // sequential read
  ret = i2c_read_blocking(i2c, device_select, dst, len, false);
  if (ret != len) return false;

  return true;
}
