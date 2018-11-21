void save_EEPROM_int(int addr, int data) {
  EEPROM.write(addr, data);
  // EEPROM.put(addr, myFloat);
  EEPROM.commit();
}

int read_EEPROM_int(int addr) {
  int result = 0;
  result = EEPROM.read(addr);
  // EEPROM.get(10, myNewFloat);
  // Serial.println(myNewFloat);
  //  Serial.println(myNewInt);
  EEPROM.commit();
    Serial.println(myNewInt);
  // EEPROM.commit();
  return result;
}
