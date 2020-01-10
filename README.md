# 24LC256
Arduino library for the 24LC256 external I2C EEPROM.

It is designed to mimic the built-in EEPROM class as much as possible, offering the familiar put, get, read, write and update functions. However there are small differences, particularly in the initialisation of the EEPROM.

This library has been tested on Arduino/ATmega328P and ESP8266. It will likely work on other Arduino compatible microcontrollers offering I2C interface.

This library has been designed for and is tested on the 24LC256 EEPROM chip, it is expected to work also with other 24AA/24LC series I2C EEPROM chips, provided these chips use the same interface and I2C commands.

## Installation
Download the zip from this site, and uncompress it in your libraries folder. 

## Functions

### E24LC256 (I2C_address = 0x50)
The constructor. If no I2C address is given, the default 0x50 is used.

### init ()
Call this once in setup() to initialise the EEPROM.

### byte read (address)
Read a single byte from the given address and returns it.

No range check is done, if address is out of range of the EEPROM size the behaviour is undefined.

### write (address, data)
Write a single byte to the given address.

No range check is done, if address is out of range of the EEPROM size the behaviour is undefined; other data may be overwritten.

### update (address, data)
Writes a byte to the given address but only if data is different from what is on that position already.

### put (address, data)
Writes any type of variable to the EEPROM, starting at given address. Only writes if the data actually differes from what is already stored in the EEPROM. 

The user must ensure the data does not extend past the size of the EEPROM, otherwise behaviour is undefined; other data may be overwritten.

### get (address, data)
Read any type of variable from the EEPROM, starting at given address. 

If the data structure is larger than the space between address and the size of the EEPROM, the data returned for the out of range part is undefined.

### getStatus ()
Returns an EEPROMStatus, which gets set after init() is done. This can be used to check whether the EEPROM is present and responding. Possible results:
* UNKNOWN: no search done yet.
* EEPROM_NOT_FOUND: no EEPROM chip found on this I2C address.
* EEPROM_FOUND: an EEPROM chip was found.
