int ConfigChanged(float configConfiged, float configPresent)
{
  int alert = 0;
  if (configPresent - configConfiged <= 0)
  {
    Serial.println("configPresent - configConfiged <= 0");
  }
  else if ((configPresent - configConfiged >= 0.0) && (configPresent - configConfiged < 0.5))
  {
    Serial.println("(configPresent - configConfiged >= 0) && (configPresent - configConfiged < 0.5)");
  }
  else if ((configPresent - configConfiged >= 0.5) && (configPresent - configConfiged < 1.0))
  {
    Serial.println("(configPresent - configConfiged >= 0.5) && (configPresent - configConfiged < 1.0)");
    alert = 1;
  }
  else if ((configPresent - configConfiged >= 1.0) && (configPresent - configConfiged < 2.5))
  {
    Serial.println("(configPresent - configConfiged >= 1.0) && (configPresent - configConfiged < 2.5)");
    alert = 2;
  }
  else
  {
    Serial.println("configPresent - configConfiged >= 2.5");
    alert = 3;
  }
  return alert;
}
