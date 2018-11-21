
void OutputConfigs(JsonObject& rootConfigs)
{
  Serial.println("JSON CONFIGS: ");

  Serial.print("ID: ");
  ID = rootConfigs["id"].as<char*>();
  Serial.println(ID);

  Serial.print("Name: ");
  Name = rootConfigs["name"].as<char*>();
  Serial.println(Name);

  Serial.print("pH: ");
  pH = rootConfigs["config"]["pH"];
  Serial.println(pH);

  Serial.print("Temperature: ");
  Temperature = rootConfigs["config"]["temperature"];
  Serial.println(Temperature);

  Serial.print("Light: ");
  Light = rootConfigs["config"]["light"];
  Serial.println(Light);

  Serial.print("Soluble: ");
  Soluble = rootConfigs["config"]["soluble"];
  Serial.println(Soluble);

  Serial.print("timeRun: ");
  timeRun = rootConfigs["config"]["pumpTimeOn"];
  timeRun = timeRun * toMinute;
  //timeRunEE = timeRun;
  Serial.println(timeRun);

  Serial.print("timeBreak: ");
  timeBreak = rootConfigs["config"]["pumpTimeOff"];
  timeBreak = timeBreak * toMinute;
  //timeBreakEE = timeBreak;
  Serial.println(timeBreak);

  Serial.print("Auto mode: ");
  autoMode = 1;
  temp_automode = autoMode;
  save_EEPROM_int(e_autoMode,temp_automode);
  Serial.print(autoMode);
  on_motor = true;
}
