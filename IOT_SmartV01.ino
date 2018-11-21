#include "DHT.h"
#include <ESP8266HTTPClient.h>
#include "ArduinoJson.h"
#include <Arduino.h>
#include <Ticker.h>
#include <time.h>
#include <ESP8266WiFi.h>
#include "PubSubClient.h"
#include <EEPROM.h>


//
#define samplingInterval 500
#define printInterval 10000

//TDSSEnsor---------
#define TdsSensorPin A0 //tds meter analog output to arduino analog input 0
#define SCOUNT  30           // sum of sample point
///----------

//pHsensor
#define SensorPin A0   //pH meter Analog output to Arduino Analog Input 0
#define Offset 0.00 //deviation compensate
#define ArrayLenth  40    //times of collection
//

//relay int
#define relayIn1 15 //D8 //pump
#define relayIn2 12 //D6 //
#define relayIn3 0 //D3 //
#define relayIn4 2 //D4 //led on/off
//

#define  DEBUG  0
#define CONTROL_POSITON  1
#define SIZE_OF_EEPROM  512
#define PIN_BUTTON 04 //smartconfig //d2
#define LED_OFF() digitalWrite(LED_BUILTIN, HIGH)
//#define LED_TOGGLE() digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN) ^ 0x01)

#define timeToBreak 60000 //nghi trong vong 1p = 60*1000milis
// Update these with values suitable for your network.

const char* mqtt_server = "vnsupplychain.xyz"; //server cua broke
const String ipServer = "api.vnsupplychain.xyz:8443";
const char* ThumpPrint = "8E:80:4A:CC:30:BC:50:95:97:51:3A:C0:80:E0:85:71:81:38:29:EC";



Ticker ticker;
WiFiClient espClient;
PubSubClient client(espClient); //dung esp wifi de gui nhan messenge len broker

int toMinute = 1000 * 60;
long time_now = 0;
bool on_motor = false;
int ConfigChanged();
int TYPE;
int TYPEtemp;
//EEPROM, vi tri bien duoc luu trong EEPROM
byte e_autoMode = 254; //vi tri luu autoMode
int e_TIME_OFF_AUTOMATIC_PUMP = 240;
int e_TIME_ON_MOTOR = 230;
int temp_automode;
int temp_Manual;
int myInt = 999999999, myNewInt = 0;
int e_Manual = 253;
//

//TDS_Sensor
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, tdsValue = 0, temperature = 25;
int getMedianNum();
//

//phSensor
int pHArray[ArrayLenth];   //Store the average value of the sensor feedback
int pHArrayIndex = 0;
double avergearray();
float voltage, pHValue;
//


//var
const char* ID;
const char* Name;
double pH, Temperature, Light, Soluble;
unsigned long timeBreak; //pumTimeOff
unsigned long timeRun; //pumpTimeOn
const char* Config;
int autoMode;
int manual_Pump;
int manual_Led;
unsigned long timeBreakEE; //pumTimeOff
unsigned long timeRunEE;


int tempModePump; //luu che do Pump
int tempModeLed; //luu che do Led
void OutputConfigs();
//

//returnConfig
String returnConfigStr;
//

//smartconfig
bool in_smartconfig = false;
bool longPress();
void tick();
void enter_smartconfig();
void exit_smart();
//smartconfig


void setup() {

  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  EEPROM.begin(SIZE_OF_EEPROM);
  pinMode(relayIn1, OUTPUT);
  pinMode(relayIn2, OUTPUT);
  pinMode(relayIn3, OUTPUT);
  pinMode(relayIn4, OUTPUT);
  digitalWrite(relayIn1, LOW);
  digitalWrite(relayIn2, LOW);
  digitalWrite(relayIn3, LOW);
  digitalWrite(relayIn4, LOW);
  digitalWrite(LED_BUILTIN, HIGH);//led onboard
  pinMode(TdsSensorPin, INPUT);
  pinMode(SensorPin, INPUT);
  setup_wifi();
  Serial.println("DOC CONFIG MODE");
  autoMode = read_EEPROM_int(e_autoMode);
  Serial.println(autoMode);
  Serial.println("KET THUC DOC MODE");
  if (autoMode == 0)
  {
    manual_Pump = read_EEPROM_int(e_Manual);
    if (manual_Pump == 1)
    {
      Serial.print("TRANG THAI TRUOC LA ON MOTOR MANUAL: ");
      Serial.println(manual_Pump);
      actionOnMotor();
    }
    if (manual_Pump == 0)
    {
      Serial.println("TRANG THAI TRUOC LA OFF MOTOR MANUAL: ");
      Serial.println(manual_Pump);
      actionOffMotor();
    }
  }
  else if (autoMode == 1)
  {
    Serial.println("TRANG THAI TRUOC LA AUTO PUMP");
  }
  else
  {
    Serial.println("KHONG DOC DUOC TRANG THAI");
  }


  client.setServer(mqtt_server, 1883);//port 1883 la port ket noi den broker
  client.setCallback(callback); //nhan cac gia tri tham so khi client nhan 1 message tu broker.
  Serial.println("pH meter experiment!");   //Test the serial monitor
}

void setup_wifi() {
  if (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to Wifi.....");
  }
  else
  {
    Serial.setDebugOutput(true);
    WiFi.mode(WIFI_STA); //chuyen wifi sang che do cho
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(PIN_BUTTON, INPUT);
    ticker.attach(1, tick);
    Serial.println("Setup done - Wifi connected");
  }
}

void callback(char* topic, byte* payload, unsigned int length)
{
  String json;
  const char* IDconfig;
  const char* type;

  //payload: message
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("]");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    json += (char)payload[i];
  }
  Serial.println();
  Serial.print("Json nhan duoc: ");
  Serial.println(json);

  DynamicJsonBuffer  jsonBuffer(200);
  JsonObject& root = jsonBuffer.parseObject(json);

  if (!root.success()) //kt neu parseJson ko thanh cong thi tra ve.
  {
    Serial.println("parseObjectJson failed");
    json = "";
    return;
  }
  TYPE = root["type"];
  //String str(type);
  Serial.print("Type: ");
  Serial.println(TYPE);

  switch (TYPE)
  {
    case 0: //refesh mode Pump - led
      {
        int refeshMode = TYPE + 100;
        String returnStr = String(refeshMode);
        returnStr =  "{\"type\":" + returnStr + ",\"data\":[" + tempModePump + "," + tempModeLed + "]}"; //data la mang gia tri.
        char returnConfig[100];
        returnStr.toCharArray(returnConfig, 100);
        //Serial.println(returnConfig);
        client.publish("ESP8266IOT", returnConfig);
        returnStr = "";
        json = "";
      }
      break;
    case 1: //xet config chay tu dong
      {
        IDconfig = root["data"];
        String idCon(IDconfig);
        Serial.print("Data: ");
        Serial.println(idCon);

        HTTPClient http;

        String server = "https://" + ipServer + "/config/" + idCon + "";
        //https - certificate
        http.begin(server, ThumpPrint); //firewall
        Serial.print("Connect HTTP: ");
        Serial.println(server);
        int httpCode = http.GET();
        Serial.print("HttpCode: ");
        Serial.println(httpCode);
        if (httpCode > 0)
        {
          String stringReturn = http.getString();
          Serial.println(stringReturn);

          JsonObject& rootConfigs = jsonBuffer.parseObject(stringReturn);
          if (!rootConfigs.success()) //kt neu parseJson ko thanh cong thi tra ve.
          {
            Serial.println("parseObjectJson failed");
            stringReturn = "";
            return;
          }
          else
          {
            //in config
            Serial.println("----------------------------");
            OutputConfigs(rootConfigs); //truyen vao JsonObject

            //return
            int returnC = TYPE + 100;
            String returnStr = String(returnC);
            returnStr =  "{\"type\":" + returnStr + ",\"data\":1}";
            char returnConfig[100];
            returnStr.toCharArray(returnConfig, 100);
            //Serial.println(returnConfig);
            client.publish("ESP8266IOT", returnConfig);
            //str = "";
            returnStr = "";
            json = "";
            //
          }
        }

        http.end();
        Serial.println(""); Serial.println("");
      }
      break;


    case 2: //chay bom bang tay
      {
        autoMode = 0;
        temp_automode = autoMode;
        save_EEPROM_int(e_autoMode, temp_automode);
        on_motor = true;
        manual_Pump = root["data"];
        temp_Manual = manual_Pump;
        if (manual_Pump == 1)
        {
          Serial.print("ON-MOTOR");
          save_EEPROM_int(e_Manual, temp_Manual);

        }
        if (manual_Pump == 0)
        {
          Serial.print("OFF-MOTOR");
          save_EEPROM_int(e_Manual, temp_Manual);
        }
        json = "";
      }
      break;
    case 3:
      {
        manual_Led = root["data"];
        if (manual_Led == 1)
        {
          Serial.println("ON-LED");
        }
        if (manual_Led == 0)
        {
          Serial.println("OFF-LED");
        }
        json = "";
      }
      break;
    case 4: //refesh thong so hien tai
      {
        int refeshMode = TYPE + 100;
        String returnStr = String(refeshMode);
        returnStr =  "{\"type\":" + returnStr + ",\"data\":[" + pHValue + "," + temperature + "," + tdsValue + "]}"; //data la mang gia tri.
        char returnConfig[100];
        returnStr.toCharArray(returnConfig, 100);
        //Serial.println(returnConfig);
        client.publish("ESP8266IOT", returnConfig);
        returnStr = "";
        json = "";
      }
      break;
    case 101:
      {
        Serial.println("CONFIG SUCESSFUL! ");
        TYPEtemp = 101; //type dung de tu dong refesh gia tri thong so config thay doi
        return;
        json = "";
      }
      break;
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("IOT_ESP")) { //co 3 tham so: clientID, username, password.
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("ESP8266IOT", "Reconected"); //publish message ken broker
      // ... and resubscribe
      client.subscribe("ESP8266IOT");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {

  static unsigned long preTime = millis();
  if (longPress())
  {
    enter_smartconfig();
  }
  if (WiFi.status() == WL_CONNECTED && in_smartconfig == true && WiFi.smartConfigDone()) {
    exit_smart();
    Serial.println("Connected, Exit smartconfig");
  }
  else if (millis() - preTime > 60000)
  {
    if (WiFi.status() != WL_CONNECTED && in_smartconfig == true)
    {
      exit_smart();
      Serial.println("NOT CONNECT, Exit smartconfig");
    }
    preTime = millis();
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    if (!client.connected()) {
      reconnect();
    }
    else //ket noi duoc voi broker
    {
      client.loop();

      if (autoMode == 1)//chay auto
      {
        actionAuto();
      }
      if (autoMode == 0) //chay tu chinh
      {
        actionManual();
      }
      actionManualLed();

      //--------TIME PRINT -- TIME VALUE
      static unsigned long samplingTime = millis();
      static unsigned long printTime = millis();

      if (millis() - samplingTime > samplingInterval) //moi 5s do 1 lan
      {
        //-------PH
        pHArray[pHArrayIndex++] = analogRead(SensorPin);
        if (pHArrayIndex == ArrayLenth)
          pHArrayIndex = 0;
        voltage = avergearray(pHArray, ArrayLenth) * 5.0 / 1024;
        pHValue = 7 - (voltage / 57.14) + Offset;
        //-----------------

        ///----TDS
        analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
        analogBufferIndex++;
        if (analogBufferIndex == SCOUNT)
          analogBufferIndex = 0;
        //----------------

        //---------------ppm
        for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
          analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
        averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * 5.0 / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
        float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0); //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
        float compensationVoltage = averageVoltage / compensationCoefficient; //temperature compensation
        tdsValue = ((133.42 * compensationVoltage * compensationVoltage * compensationVoltage - 255.86 * compensationVoltage * compensationVoltage + 857.39 * compensationVoltage) * 0.5) / 10; //convert voltage value to tds value
        //----------------

        samplingTime = millis();

      }
      if (millis() - printTime > printInterval)  //moi 10s in 1 lan.
      {

        //---------PH
        Serial.print("Voltage:");
        Serial.print(voltage, 2);
        Serial.print("    pH value: ");
        Serial.println(pHValue, 2);
        //------------------


        //--------TDS
        Serial.print("voltage:");
        Serial.print(averageVoltage, 2);
        Serial.print("V   ");
        Serial.print("TDS Value:");
        Serial.print(tdsValue, 0);
        Serial.println("ppm");
        //---------------


        //return Config to broker
        if (TYPEtemp == 101)
        {
          if (ConfigChanged(pH, pHValue) > 1)
          {
            String pHString = String(pHValue);
            returnConfigStr =  "{\"type\":105,\"data\":[" + pHString + "]}";
            char returnConfigMT[100];
            returnConfigStr.toCharArray(returnConfigMT, 100);
            //Serial.println(returnConfig);
            client.publish("ESP8266IOT", returnConfigMT);
            //str = "";
            returnConfigStr = "";
          }

          if (ConfigChanged(Temperature, temperature) > 1)
          {
            String tempString = String(temperature);
            returnConfigStr =  "{\"type\":106,\"data\":[" + tempString + "]}";
            char returnConfigMT[100];
            returnConfigStr.toCharArray(returnConfigMT, 100);
            //Serial.println(returnConfig);
            client.publish("ESP8266IOT", returnConfigMT);
            //str = "";
            returnConfigStr = "";
          }

          if (ConfigChanged(Soluble, tdsValue) > 1)
          {
            String tdsString = String(tdsValue);
            returnConfigStr =  "{\"type\":107,\"data\":[" + tdsString + "]}";
            char returnConfigMT[100];
            returnConfigStr.toCharArray(returnConfigMT, 100);
            //Serial.println(returnConfig);
            client.publish("ESP8266IOT", returnConfigMT);
            //str = "";
            returnConfigStr = "";
          }
        }
        printTime = millis();
      }
    }
  }
}

void actionAuto() {// chạy tự động sau khoảng toời gian

  if (on_motor == true) {
    if (millis() - time_now <= timeRun) {// chay bo trong khoang thời gian TIME_RUN_MOTOR
      actionOnMotor();
      //  Serial.println(millis() - time_now );
    } else {
      actionOffMotor();
      time_now = millis();
      on_motor = false;
      //   Serial.println(TIME_OFF_AUTOMATIC_PUMP);

      //check_temprature_for_run_pump();
      //   Serial.println(check_temprature_for_run_pump(TIME_OFF_AUTOMATIC_PUMP));
    }
  } else {
    if (millis() - time_now <= timeBreak) {// chay bo trong khoang thời gian TIME_RUN_MOTOR
      actionOffMotor();
    } else {
      actionOnMotor();
      time_now = millis();
      on_motor = true;
    }
  }
}

void actionManual() {// chạy tu chinh

  if (manual_Pump == -1)
  {
    Serial.println("Toi khong nhan duoc gi ca!");
  }
  else
  {
    if (on_motor == true) {
      if (manual_Pump == 1) {
        actionOnMotor();
        //Serial.print("ON-MOTOR");

      }
      if (manual_Pump == 0) {
        actionOffMotor();
        //Serial.print("OFF-MOTOR");
        on_motor = false;
      }
    }
  }
}

void actionManualLed()
{
  if (manual_Led == -1)
  {
    Serial.println("Toi khong nhan duoc gi ca!");
  }
  else
  {
    if (manual_Led == 1) {
      actionOnLed();
      //Serial.print("ON-MOTOR");

    }
    if (manual_Led == 0) {
      actionOffLed();
      //Serial.print("OFF-MOTOR");
    }
  }
}


void actionOnMotor() {// bật pump
  digitalWrite(relayIn1, HIGH);
  tempModePump = 1;
}

void actionOffMotor() {// tat pump
  digitalWrite(relayIn1, LOW);
  tempModePump = 0;
}

void actionOnLed() {
  digitalWrite(relayIn2, HIGH);
  digitalWrite(relayIn3, HIGH);
  digitalWrite(relayIn4, HIGH);
  tempModeLed = 1;
}

void actionOffLed() {
  digitalWrite(relayIn2, LOW);
  digitalWrite(relayIn3, LOW);
  digitalWrite(relayIn4, LOW);
  tempModeLed = 0;
}
