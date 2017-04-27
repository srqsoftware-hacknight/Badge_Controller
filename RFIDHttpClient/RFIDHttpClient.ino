
/**
   RFID reader with http client and color LED indicator
   read RFID if available, GET response from Badge Server
   set LED/buzzer if access/denied
*/

#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif


// Set DEBUG to 1 for lots of lovely debug output
#define DEBUG  0

// Debug directives
#if DEBUG
#   define DBG_PRINT(...)    Serial.print(__VA_ARGS__)
#   define DBG_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
#   define DBG_PRINT(...)
#   define DBG_PRINTLN(...)
#endif



#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include "FastLED.h"

#include "cred.h"
//#include "wifi_credentials.h" // .h contains ssid, wpakey
//const char* ssid = "the_ssid";
//const char* wpakey = "the_password";

//For more logging to Serial output
//#define DEBUG

#define BTN_PIN    D3  // FIXME put real button pin here
#define PIEZO_PIN  D2 // attach piezo element to D2 and GND to hear good/bad tones.

#define PIEZO_TONE_GOOD  800
#define PIEZO_TONE_CALL  600
#define PIEZO_TONE_BAD   200


#define BADGE_URL  "http://172.31.0.1:8081/device/check?id="
#define CALL_URL  "http://172.31.0.1:8081/call"

// vars....
ESP8266WiFiMulti WiFiMulti;
char cardBuff[60]; // buffer for RFID value (string of 4 hex bytes, plus spaces)

// #############################################################################
void setup() {

  // on startup, reconnect wifi.
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.begin(115200);
  while (!Serial) ;
  // USE_SERIAL.setDebugOutput(true);
  DBG_PRINTLN();
  //USE_SERIAL.print(F("system_get_chip_id(): 0x"));
  //USE_SERIAL.println(system_get_chip_id(), HEX);

  // just use cardBuff once to show MAC H/W address
  sprintf(cardBuff, "MAC %06X.", ESP.getChipId());
  DBG_PRINTLN(cardBuff);

  SetupButton();
  
  SetupLED();

  for (uint8_t t = 4; t > 0; t--) {
    DBG_PRINT( "[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  // set piezo tone output, play bad,good tones for speaker test.
  pinMode(PIEZO_PIN, OUTPUT);
  tone(PIEZO_PIN, PIEZO_TONE_BAD, 500);
  delay(500);
  tone(PIEZO_PIN, PIEZO_TONE_GOOD, 500);
  delay(500);
  digitalWrite(PIEZO_PIN, LOW);

  // 'call' button for doorbell, use internal pull-up resistor (active low)
  pinMode(BTN_PIN, INPUT_PULLUP);


  SetupRFID();

  WiFiMulti.addAP(ssid, wpakey);

  TryWifi();

}

// #############################################################################
// connect to access point on Raspi.
// flash RED if cannot connect.
void TryWifi() {

  while ((WiFiMulti.run() != WL_CONNECTED)) {
    ShowLED(CRGB::Red); // flashing RED while trying to connect to wifi
    delay(500);
    ShowLED(CRGB::Black);
    delay(500);
  }
  // turn of LED, output IP address to Serial tty.
  ShowLED(CRGB::Black);
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());
  ShowLED(CRGB::Green); // flash green after getting IP address.
  delay(1000);
  ShowLED(CRGB::Black);
}


bool sendURL(String url) {

  HTTPClient http;
  bool ret = false;

  DBG_PRINTLN(F("[HTTP] begin..."));
  // configure traged server and url
  //http.begin("https://192.168.1.12/test.html", "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS

  http.begin(url); //HTTP

  //USE_SERIAL.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();

  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    DBG_PRINT("[HTTP] GET... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      DBG_PRINT(F("Payload="));
      DBG_PRINTLN(payload);

      if (payload.startsWith("ACCEPT")) {
        ret = true;
        DBG_PRINTLN(F("Card Accepted"));
      } else if (payload.startsWith("DENY")) {
        DBG_PRINTLN(F("Card Denied"));
      }

    } else {
#ifdef DEBUG
      DBG_PRINTLN("NOT OK");
#endif
    }
  } else {
#ifdef DEBUG
    DBG_PRINT( "[HTTP] GET... failed, error: %s\n", HTTPClient::errorToString(httpCode).c_str());
#endif
  }
  http.end();
  return ret;
}


// #############################################################################
// try read 'call' button, if presed then send to CALL_URL.
//
bool TryButton() {
  bool ret = false;
  // active low button, LOW when pressed.
  if ( digitalRead(BTN_PIN) == HIGH) {
    ret = false;
  }

  if ((WiFiMulti.run() == WL_CONNECTED)) {
    String url = CALL_URL ;
    ret =  sendURL(url);
  } else {
    DBG_PRINT( F("[HTTP] call error") );
  }

  return ret;
}

// #############################################################################
// try a card id with the Auth Server.
// Handle result: LED indicator, buzzer.
bool TryCard(String cid) {
  bool accepted =  false;

  if ((WiFiMulti.run() == WL_CONNECTED)) {
    String url = BADGE_URL + cid ;
    accepted =  sendURL(url);
  } else {
    DBG_PRINT( F("[HTTP] auth error") );
  }

  return accepted;
}


// #############################################################################
// Main event loop
void loop() {

  if ( CardAvailable() ) {
    ShowLED(CRGB::Blue);
    yield();

    // if RFID card present, read it. Else, keep looping.
    ScanCard(cardBuff);
    yield();
    String cardString(cardBuff);
    DBG_PRINT(F("cardString="));
    DBG_PRINTLN(cardString);

    if ( TryCard(cardString) ) {
      ShowLED(CRGB::Green);
      yield();
      tone(PIEZO_PIN, PIEZO_TONE_GOOD, 800);
      delay(1000);
      digitalWrite(PIEZO_PIN, LOW);
    } else {
      ShowLED(CRGB::Red);
      yield();
      tone(PIEZO_PIN, PIEZO_TONE_BAD, 800);
      delay(1000);
      digitalWrite(PIEZO_PIN, LOW);
    }
    ShowLED(CRGB::Black);
    delay(500);

  } // if CardAvailable()
  yield();

  if ( TryButton() ) {
    ShowLED(CRGB::Green);
    yield();
    tone(PIEZO_PIN, PIEZO_TONE_CALL, 800);
    delay(1000);
    yield();
    digitalWrite(PIEZO_PIN, LOW);
    ShowLED(CRGB::Black);
    delay(1000);
  }


}


