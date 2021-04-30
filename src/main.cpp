#include <Arduino.h>
#include <ArduinoLog.h>
#include "Button2.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>
  #include <WiFiUdp.h>
#endif
#include <ArduinoOTA.h>


// Code from https://wiki.dfrobot.com/DFPlayer_Mini_SKU_DFR0299

#define BUTTON_PIN D4

const char *HOSTNAME = "mario_player";
const char *WIFI_SSID = "<your ssid>";
const char *WIFI_PWD = "<your pwd>";

SoftwareSerial mySoftwareSerial(D1, D2);  // RX, TX
Button2 button = Button2(BUTTON_PIN);
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

void OTA_setup() {
  ArduinoOTA.onStart([]() {
    Log.notice(F("ArduinoOTA start" CR));
  });
  ArduinoOTA.onEnd([]() {
    Log.notice(F("ArduinoOTA end" CR));
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Log.notice(F("ArduinoOTA progress: %u%" CR), (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Log.error(F("ArduinoOTA error[%u]: " CR), error);

    if (error == OTA_AUTH_ERROR) Log.error(F("ArduinoOTA Auth Failed" CR));
    else if (error == OTA_BEGIN_ERROR) Log.error(F("ArduinoOTA Begin Failed" CR));
    else if (error == OTA_CONNECT_ERROR) Log.error(F("ArduinoOTA Connect Failed" CR));
    else if (error == OTA_RECEIVE_ERROR) Log.error(F("ArduinoOTA Receive Failed" CR));
    else if (error == OTA_END_ERROR) Log.error(F("ArduinoOTA End Failed" CR));
  });

  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.setPassword((const char *)"<your OTA pwd>");
  ArduinoOTA.begin();
}

void button_handler(Button2& btn) {
    if (btn.isPressed()) {
      Log.notice(F("Buttton pressed, playing MP3 after a initial delay." CR));
      myDFPlayer.volume(20);
      delay(5000);
      // play the first MP3, then the second, and...
      myDFPlayer.play();
    }
}

void setup()
{
  Serial.begin(115200);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);

  Log.notice(F("DFRobot DFPlayer Mini" CR));
  Log.notice(F("ESP Board MAC Address: %s" CR), WiFi.macAddress().c_str());
  mySoftwareSerial.begin(9600);

  Log.notice(F("WiFi connecting to accesspoint..." CR));
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Log.error(F("WiFi Connection Failed! Rebooting..." CR));
    delay(5000);
    ESP.restart();
  }
  
  Log.notice(F("WiFi connected, setting up OTA-listener" CR));
  OTA_setup();

  Log.notice(F("Initializing DFPlayer... (may take 3-5 seconds)" CR));

  if (!myDFPlayer.begin(mySoftwareSerial)) {
    Log.error(F("Unable to begin:" CR));
    Log.error(F("1.Please recheck the connection!" CR));
    Log.error(F("2.Please insert the SD card!" CR));
  } else {
    Log.notice(F("DFPlayer Mini online." CR));
    Log.notice(F("Found: %D files" CR), myDFPlayer.readFileCounts());
    
    myDFPlayer.volume(0);      // set volume value. From 0 to 30
    //----Set different EQ----
    myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  //  myDFPlayer.EQ(DFPLAYER_EQ_POP);
  //  myDFPlayer.EQ(DFPLAYER_EQ_ROCK);
  //  myDFPlayer.EQ(DFPLAYER_EQ_JAZZ);
  //  myDFPlayer.EQ(DFPLAYER_EQ_CLASSIC);
  //  myDFPlayer.EQ(DFPLAYER_EQ_BASS);

    button.setPressedHandler(button_handler);
  }
}

void loop()
{
  ArduinoOTA.handle();
  button.loop();  

  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read()); // print the detail message from DFPlayer to handle different errors and states.
  }
}

void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Log.warning(F("Time Out!" CR));
      break;
    case WrongStack:
      Log.warning(F("Stack Wrong!" CR));
      break;
    case DFPlayerCardInserted:
      Log.notice(F("Card Inserted!" CR));
      break;
    case DFPlayerCardRemoved:
      Log.notice(F("Card Removed!" CR));
      break;
    case DFPlayerCardOnline:
      Log.notice(F("Card Online!" CR));
      break;
    case DFPlayerPlayFinished:
      Log.notice(F("Number: %D play Finished!" CR), value);
      break;
    case DFPlayerError:
      Log.warning(F("DFPlayerError:" CR));
      switch (value) {
        case Busy:
          Log.warning(F("Card not found" CR));
          break;
        case Sleeping:
          Log.warning(F("Sleeping" CR));
          break;
        case SerialWrongStack:
          Log.warning(F("Get Wrong Stack" CR));
          break;
        case CheckSumNotMatch:
          Log.warning(F("Check Sum Not Match" CR));
          break;
        case FileIndexOut:
          Log.warning(F("File Index Out of Bound" CR));
          break;
        case FileMismatch:
          Log.warning(F("Cannot Find File" CR));
          break;
        case Advertise:
          Log.warning(F("In Advertise" CR));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}
