_Update (6/7/25): This library should work on RP2350 as well but has not yet been tested._

The M24C08 is an 8 Kbit I2C EEPROM from STMicroelectronics. It is organized as 1024 bytes with a page size of 16 bytes.

Each transaction begins by sending a 1 byte device select code which with consists of the base address (0x50), device select bit, highest 2 address bits, and the read/write bit.

### Read
A read always begins at the EEPROM’s current internal address. In order to do a random read, a dummy write is first performed to set the address before performing the read. The EEPROM will continue to return data as long as it continues to receive ACK’s from the bus master.

### Write
A write operation begins with a device select and address byte followed by the data. If the write reaches the end of a page, it will wrap around and continue writing at the beginning of the page. 

A naive write implementation may write single bytes. However this increases wear by a factor equal to the page size as the entire page is rewritten even for a single byte write. Bus traffic is increased by a factor of PAGE_SIZE / 2 + 1 due to the 2 byte overhead of each transaction. The overall write time is significantly increased due to the extra EEPROM internal write cycles and bus traffic.

The most efficient write implementation does page writes. It must correctly handle the potential partial first and last pages as well as the potential offset in the first page. The write implementation must also allow time between page writes for the EEPROM’s internal write operation to complete.

### Update
The read and write functions are sufficient for a user to implement any more sophisticated operations. However, I also included an update function as it is a very common EEPROM use pattern. Update first performs a read and only performs a write if the data to be written differs from the data in memory. This reduces wear on the memory as the number of lifetime writes on an EEPROM is far lower than the number of lifetime reads. Again, this is most efficiently implemented on a page basis to match the internal behavior of the EEPROM.

The M24C08 offers sequential reads without additional addressing but it does not support the same for writes. If a write goes past the end of a page, it will wrap around and continue to write at the beginning of the page, so a new write operation must occur for each page.

The two possibilities to read and compare the data in memory are a sequential read of the entire range before any writes or a page read before each potential page write. The single sequential read is faster and requires less bus traffic, but it uses more memory (potentially up to the size of the EEPROM). Going page by page only requires a buffer size up to the page size. It does however, require a device select and address to be written for each read. 

My initial implementation utilizes the latter approach because of its simplicity, but I intend to make the behavior user configurable.

## [Next Steps](https://github.com/JohnMcAninley/m24c08/issues)

### [Timeout](https://github.com/JohnMcAninley/m24c08/issues/1)
The current implementation uses blocking reads and writes which could potentially block forever. Instead, use blocking methods that also include timeouts and expose a function to the user to adjust the timeout.

### [Read from Current Address](https://github.com/JohnMcAninley/m24c08/issues/2)
Perform a read without first executing a dummy write to set the address. Unclear if address bits A9 and A8 in the device select code must match the current address.

### [ACK Polling on Write](https://github.com/JohnMcAninley/m24c08/issues/3)
Per the datasheet, the maximum write cycle time (Tw) is 5ms, but it is often faster. Therefore, instead of waiting 5ms between writes as in the current implementation, the next device select can be issued and reissued until an ACK is received. This should still include a timeout.

### [Configurable Update Buffer Size](https://github.com/JohnMcAninley/m24c08/issues/4)
A generalized update implementation which allows the user to select between throughput and memory usage. This implementation performs a sequential read up to a configurable maximum buffer size, repeating as many times as necessary. Setting the maximum buffer size equal to the EEPROM size performs a single sequential read and setting it equal to the page size performs a new read for each page.

```MAX_UPDATE_BUFFER_SIZE = n * PAGE_SIZE, 1 <= n <= PAGES```

### [Generalize to Additional EEPROM Sizes](https://github.com/JohnMcAninley/m24c08/issues/5)
The same device family includes EEPROM’s of additional sizes. Adapting the library for use with the M24C01, M24C02, M24C04, and M24C16 should be straightforward. These all implement the same 2 KB address space across different numbers of devices. They also have the same page size.

Larger EEPROM’s in the same family (M24C32, M24C64, and M24128) require an additional address byte and therefore require slightly more changes to implement.

### [Multiple Device Address Ranges](https://github.com/JohnMcAninley/m24c08/issues/6)
There are 11 bits used to identify an address, the highest of which identifies 1 of 2 devices. A new device select code is necessary to continue the operation on the second device. This would actually occur automatically for writes as a new device select will be sent which will map to the device holding the higher memory, but requires the removal of a guard checking accesses that would be out of bounds on a single device. 

However, this would introduce ambiguous behavior for sequential reads as on a single device they roll over. This is behavior that is potentially desirable if the memory contains a circular buffer. Therefore, this behavior will need to be configurable.

### [Generic I2C Read/Write Functions](https://github.com/JohnMcAninley/m24c08/issues/7)
Implement a hardware abstraction layer that allows the library to be used in applications beyond the RP2040 with the Pico C SDK. Accept generic I2C read and write functions as arguments to allow user defined functions or those from other API’s (Arduino, STM, etc).

### [Write Verification](https://github.com/JohnMcAninley/m24c08/issues/8)
Verify the success of a write by performing a readback and comparing the data. Convenience function as this is already possible for the user to achieve by reading after writing.

### [Programmatic Write Control](https://github.com/JohnMcAninley/m24c08/issues/9)
The M24C08 includes a write control pin which can be tied high or low or controlled programmatically. A user can clear and set this pin before and after performing a write. This behavior can be included in the library and be performed on writes for convenience to the user.

## References
- [M24C08 Datasheet](https://www.st.com/resource/en/datasheet/m24c08-r.pdf)
- [Pico C SDK: I2C API](https://www.raspberrypi.com/documentation/pico-sdk/hardware.html#group_hardware_i2c)
