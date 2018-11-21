
bool longPress()
{
  static int lastPress = 0;
  if (millis() - lastPress > 3000 && digitalRead(PIN_BUTTON) == HIGH) {
    return true;
  } else if (digitalRead(PIN_BUTTON) == LOW) {
    lastPress = millis();
  }
  return false;
}

void tick()
{
  //toggle state
  int state = digitalRead(LED_BUILTIN);  // get the current state of GPIO1 pin
  digitalWrite(LED_BUILTIN, !state);     // set pin to the opposite state
}


void enter_smartconfig()
{
  Serial.println("Enter smartconfig");
  if (WiFi.status() == WL_CONNECTED)
  {
    WiFi.disconnect();
    delay(100);
  }
  if (in_smartconfig == false) {
    in_smartconfig = true;
    ticker.attach(0.1, tick);
    WiFi.beginSmartConfig();
  }
}

void exit_smart()
{
  Serial.println("Exit smartconfig");
  ticker.detach();
  LED_OFF();
  in_smartconfig = false;
}
