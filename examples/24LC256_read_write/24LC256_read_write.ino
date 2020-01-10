// 24LC256 I2C EEPROM test code.

#include <24LC256.h>

// The constructor.
//E24LC256 MyEEPROM;                                          // Initialise EEPROM at default I2C address: 0x50
E24LC256 MyEEPROM(0x50);                                    // Initialise EEPROM at the given address (0x50-0x57).

void setup() {
  Serial.begin(115200);
  Serial.println(F("Looking for EEPROM chip..."));
  MyEEPROM.init();
  switch (MyEEPROM.getStatus()) {
    case  MyEEPROM.EEPROM_FOUND:
      Serial.println(F("EEPROM found."));
      break;

    case MyEEPROM.EEPROM_NOT_FOUND:
      Serial.println(F("EEPROM not found. Halting operations."));
      while (true) {}
      break;

    default:
      Serial.println(F("Unkown status. Halting operations."));
      while (true) {}
      break;
  }

  Serial.print(F("Current data in the first 10 bytes: "));
  uint8_t data[10];
  for (uint8_t i = 0; i < 10; i++) {
    data[i] = MyEEPROM.read(i);
    Serial.print(data[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
  Serial.println();

  Serial.println(F("Writing new data to EEPROM: "));
  for (uint8_t i = 0; i < 10; i++) {
    MyEEPROM.write(i, data[i] + 1);
    Serial.print(data[i] + 1, HEX);
    Serial.print(' ');    
  }

  Serial.println();
  Serial.print(F("Reading back data from the first 10 bytes: "));
  for (uint8_t i = 0; i < 10; i++) {
    data[i] = MyEEPROM.read(i);
    Serial.print(data[i], HEX);
    Serial.print(' ');    
  }
  Serial.println();
}

void loop() {}
