
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


#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include "FastLED.h"

#include "wifi_credentials.h" // .h contains ssid, wpakey
//const char* ssid = "the_ssid";
//const char* wpakey = "the_password";

//For more logging to Serial output
//#define DEBUG 

#define PIEZO_PIN  D2 // attach piezo element to D2 and GND to hear good/bad tones.
#define PIEZO_TONE_GOOD  800
#define PIEZO_TONE_BAD   200


#define USE_SERIAL Serial
//#define USE_SERIAL ''

#define AUTH_URL  "http://172.31.0.1:8081/device/check?id="

// vars....
ESP8266WiFiMulti WiFiMulti;
char cardBuff[60]; // buffer for RFID value (string of 4 hex bytes, plus spaces)


// #############################################################################
void setup() {

  // on startup, reconnect wifi.
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  USE_SERIAL.begin(115200);
  while (!Serial) ;
  // USE_SERIAL.setDebugOutput(true);
  USE_SERIAL.println();
  //USE_SERIAL.print(F("system_get_chip_id(): 0x"));
  //USE_SERIAL.println(system_get_chip_id(), HEX);

  // just use cardBuff once to show MAC H/W address
  sprintf(cardBuff, "MAC %06X.", ESP.getChipId());
  USE_SERIAL.println(cardBuff);

  SetupLED();

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf( "[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

    // set piezo tone output, play bad,good tones for speaker test.
    pinMode(PIEZO_PIN, OUTPUT);
    tone(PIEZO_PIN,PIEZO_TONE_BAD,500);
    delay(500);
    tone(PIEZO_PIN,PIEZO_TONE_GOOD,500);
    delay(500);

  SetupRFID();

  WiFiMulti.addAP(ssid, wpakey);

  TryWifi();

}

// #############################################################################
// connect to access point on Raspi.
// flash RED if cannot connect.
void TryWifi() {

  while ((WiFiMulti.run() != WL_CONNECTED)) {
    ShowLED(CRGB::Red);
    delay(500);
    ShowLED(CRGB::Black);
    delay(500);
  }
  // turn of LED, output IP address to Serial tty.
  ShowLED(CRGB::Black);
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());
}


// #############################################################################
// try a card id with the Auth Server.
// Handle result: LED indicator, buzzer.
bool TryCard(String cid) {
  bool accepted = false;

  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    HTTPClient http;

    USE_SERIAL.println(F("[HTTP] begin..."));
    // configure traged server and url
    //http.begin("https://192.168.1.12/test.html", "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS

    String url = AUTH_URL + cid;
    http.begin(url); //HTTP

    //USE_SERIAL.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        USE_SERIAL.print("Payload=");
        USE_SERIAL.println(payload);
        
        if (payload.startsWith("ACCEPT")) {
          accepted = true;
          USE_SERIAL.println("Card Accepted");
        } else if (payload.startsWith("DENY")) {
          USE_SERIAL.println("Card Denied");          
        }

      } else {
#ifdef DEBUG        
        USE_SERIAL.println("NOT OK");
#endif        
      }

    } else {
#ifdef DEBUG        
      USE_SERIAL.printf( "[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
#endif        
    }

    http.end();
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
    Serial.print("cardString=");
    Serial.println(cardString);

    if ( TryCard(cardString) ) {
      ShowLED(CRGB::Green);
      yield();
      tone(PIEZO_PIN,PIEZO_TONE_GOOD,800);
      delay(1000);
    } else {
      ShowLED(CRGB::Red);
      yield();
      tone(PIEZO_PIN,PIEZO_TONE_BAD,800);
      delay(1000);
    }
    ShowLED(CRGB::Black);
    delay(1000);

  } // if CardAvailable()
  yield();

}


