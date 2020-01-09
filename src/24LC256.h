#ifndef E24LC256_H
#define E24LC256_H

#include "Wire.h"

struct E24LC256 {

  enum Status {
    EEPROM_NOT_FOUND,
    EEPROM_FOUND
  } EEPROMStatus = EEPROM_NOT_FOUND;

	E24LC256(uint8_t a = 0x50) {                              // Constructor - takes the I2C address of the EEPROM (default 0x50-0x57)
  	I2CAddress = a;
  	Wire.begin();
  	if (ackPolling()) {                                     // See whether the EEPROM responds. We're just starting up so it should be read
    	EEPROMStatus = EEPROM_FOUND;
    }
  }
  
  uint8_t read(uint16_t address) {                          // Read a single byte from the memory address given.
  	if (ackPolling()) {                                     // Make sure the EEPROM is ready to communicate.
	    Wire.beginTransmission(I2CAddress);
	    Wire.write(address >> 8);
	    Wire.write(address & 0xFF);
	    Wire.endTransmission();
	    Wire.requestFrom(I2CAddress, (uint8_t) 1);
	    return Wire.read();
	  }
	}

  void write(uint16_t address, uint8_t data) {              // Write a single byte to the memory address given.
  	if (ackPolling()) {                                     // Make sure the EEPROM is ready to communicate.
	    Wire.beginTransmission(I2CAddress);
	    Wire.write((byte) address >> 8);
	    Wire.write((byte) address & 0xFF);
	    Wire.write((byte) data);
	    Wire.endTransmission();
	  }
	}

  void update(uint16_t address, uint8_t data) {             // Write a single byte to the memory address given 
                                                            // if it's different from the current value.
    uint8_t a = read(address);
    if (a != data) write(address, data);
  }
  
  Status getStatus() {
    return EEPROMStatus;
  }

  // Put complete stuctures to EEPROM - but only if the data has changed (determined on a per-page basis).
  //
  // TODO: compare 64-byte pages on Arduino.
  //
  template <typename T> T &put(uint16_t address, T &t) {
    uint16_t size = sizeof(T);                              // Size of the object given: the number of bytes to write.
    uint8_t *ptr = (uint8_t*) &t;                           // Cast object to byte array for easier handling.
    uint8_t pageSize = 32;                                  // The size of the I2C buffer for AVR Arduinos, use that for page size.
    #ifdef ESP8266
    pageSize = 64;                                          // Page size of the EEPROM (ESP's I2C buffer is 128 bytes).
    #endif
    if (ackPolling()) {                                     // Make sure the EEPROM is ready to communicate.
      uint8_t firstPageSize = pageSize * (address / pageSize + 1) - address; // Bytes until the next page boundary.
      if (firstPageSize > size) {                           // Make sure it's not greater than the total we have to check.
        firstPageSize = size;
      }
      readBytes(address, readBuffer, firstPageSize);        // Read the first page, and compare it.
      if (compareBytes(readBuffer, ptr, firstPageSize) == false) {
        writeBytes(address, ptr, firstPageSize);            // If page different: write the new data to the EEPROM.
        ackPolling();                                       // Wait for EEPROM to finish writing before continuing with the next block.
      }
      ptr += firstPageSize;                                 // Update the data pointer.
      if (size > firstPageSize) {                           // Check whether we have more bytes to write.
        uint8_t nextPageSize;
        for (uint16_t i = firstPageSize; i < size; i += pageSize) { // We have to write data pageSize bytes (or less) at a time.
          nextPageSize = pageSize;
          if (i + pageSize > size) {                        // Next page to check: pageSize bytes or less.
            nextPageSize = size - i;
          }
          readBytes(address + i, readBuffer, nextPageSize); // Read current data; compare to new data; write if different.
          if (compareBytes(readBuffer, ptr, nextPageSize) == false) {
            writeBytes(address + i, ptr, nextPageSize);
            if (i + pageSize < size) {                      // We're not done yet!
              ackPolling();                                 // Wait for EEPROM to finish writing before continuing with the next block.
            }
          }
          ptr += nextPageSize;                              // Increase the data pointer.
	      }
      }
    }
    return t;
  }
  
  template <typename T> T &get(uint16_t address, T &t) {    // Get any type of data from the EEPROM.
    uint16_t size = sizeof(T);                              // The size of the type: amount of bytes to read.
    uint8_t *ptr = (uint8_t*) &t;                           // Cast object to byte array for easier handling.
    uint8_t bufferSize = 32;                                // Arduino's default I2C buffer size - don't read more than that in one go.
    #ifdef ESP8266
    bufferSize = 128;                                       // ESP8266's default I2C buffer size - don't read more than that in one go.
    #endif

    if (ackPolling()) {                                     // Make sure the EEPROM is ready to communicate.    
      for (uint16_t i = 0; i < size; i += bufferSize) {     // We have to read data bufferSize bytes (or less) at a time.
        uint8_t block = bufferSize;
        if (size - i < bufferSize) {                        // Calculate remainder, if less than bufferSize bytes left to read.
          block = size - i;
        }
        readBytes(address + i, ptr, block);
        ptr += block;
      }
    }
    return t;
  }
  
private:
  uint8_t I2CAddress;
  uint8_t readBuffer[64];
  
  void writeBytes (uint16_t address, uint8_t *ptr, uint8_t nBytes) {
    Wire.beginTransmission(I2CAddress);
    Wire.write((uint8_t) (address >> 8));
    Wire.write((uint8_t) (address & 0xFF));
    for (uint16_t i = 0; i < nBytes; i++) {
      Wire.write((uint8_t) *ptr);
      ptr++;
    }
    Wire.endTransmission();
  }

  void readBytes (uint16_t address, uint8_t *ptr, uint8_t nBytes) {
    Wire.beginTransmission(I2CAddress);
    Wire.write((address) >> 8);
    Wire.write((address) & 0xFF);
    Wire.endTransmission();
    Wire.requestFrom(I2CAddress, nBytes);
    for (uint8_t j = 0; j < nBytes; j++) {
      *ptr = Wire.read();                                   // Read the bytes one by one, copy them to the data object.
      ptr++;                                                // Increment the pointer.
    }
  }

  bool compareBytes(uint8_t* a, uint8_t* b, uint8_t n) {
    for (uint8_t i = 0; i < n; i++) {
      if (*a != *b) {
        return false;
      }
      a++;
      b++;
    }
    return true;
  }

	bool ackPolling() {                                       // Poll the IC to make sure it's ready for communication.
	  uint32_t startPolling = micros();
	  uint8_t code = 1;
	  while (code != 0                                        // Continue until we have a successful ack, or
	         && micros() - startPolling < 6000) {             // timeout: writing should not take more than 5 ms, normal is ~4.5 ms.
	    Wire.beginTransmission(I2CAddress);
		  Wire.write((uint8_t) 0);
      code = Wire.endTransmission();
    }
    return (code == 0);
	}
};
#endif
