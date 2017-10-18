
/**
 * Using Wemos D1 Mini.
 * 
 * RFID reader with http client and color LED indicator.
 * Read RFID if available, GET response from Badge Server on Pi. 
 * Set LED/buzzer if access is granted/denied.
 * Service CALL button by hitting some http endpoint. lockout button for 20 secs or so.
 *
 * NOTE: gpio2(D4), gpio0(D3) have external 10k pull-up resistors (used in programming)
 * We can pull down with 'call' button and check for LOW condition.
 * This can be useful to implement a 'Guest Call' button on the reader box.
 *
 * these are not good pins to use for serial bus I/O, since they have the pullup resistor.
 * but they are fine for general signalling, or taking advantage of the pullup (like SPI reset line)
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

#include "private_credentials.h"
//const char* ssid = "the_ssid";
//const char* wpakey = "the_password";

//For more logging to Serial output
//#define DEBUG

#define PIEZO_PIN  D2 // attach piezo element to D2 and GND to hear good/bad tones.

#define PIEZO_TONE_GOOD  800
#define PIEZO_TONE_CALL  400
#define PIEZO_TONE_BAD   200


#define BADGE_URL  "http://172.31.0.1:8081/device/check?badge_id="
#define CALL_URL  "http://172.31.0.1:8081/call"
//#define BADGE_URL  "http://192.168.1.2:8081/device/check?badge_id="
//#define CALL_URL  "http://192.168.1.2:8081/call"

// vars....
ESP8266WiFiMulti WiFiMulti;
char cardBuff[60]; // buffer for RFID value (string of 4 hex bytes, plus spaces)

// #############################################################################
void setup() {

  // on startup, reconnect wifi.
  delay(500);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(500);

  Serial.begin(115200);
  while (!Serial) ;
  // USE_SERIAL.setDebugOutput(true);
  DBG_PRINTLN();
  //USE_SERIAL.print(F("system_get_chip_id(): 0x"));
  //USE_SERIAL.println(system_get_chip_id(), HEX);

  // just use cardBuff once to show MAC H/W address
  sprintf(cardBuff, "MAC %06X.", ESP.getChipId());
  DBG_PRINTLN(cardBuff);

  // 'call' button for doorbell, use internal pull-up resistor (active low)
  SetupButton();

  SetupLED();

  for (uint8_t t = 4; t > 0; t--) {
    DBG_PRINT( "[SETUP] WAIT %d..." );
    DBG_PRINTLN( t);
    Serial.flush();
    delay(1000);
  }

  // set piezo tone output, play bad,good tones for speaker test.
  pinMode(PIEZO_PIN, OUTPUT);
  tone(PIEZO_PIN, PIEZO_TONE_BAD, 500);
  delay(500);
  tone(PIEZO_PIN, PIEZO_TONE_CALL, 500);
  delay(500);
  tone(PIEZO_PIN, PIEZO_TONE_GOOD, 500);
  delay(500);
  digitalWrite(PIEZO_PIN, LOW);


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
  // play 'good' 3 times on success wifi connect.
  tone(PIEZO_PIN, PIEZO_TONE_GOOD, 100);
  delay(200);
  tone(PIEZO_PIN, PIEZO_TONE_GOOD, 100);
  delay(200);
  tone(PIEZO_PIN, PIEZO_TONE_GOOD, 100);
  delay(200);
  // delay a little longer after 'good' tones, then go black.
  delay(500);
  ShowLED(CRGB::Black);
}


bool sendURL(String url) {

  HTTPClient http;
  bool ret = false;

  if ((WiFiMulti.run() != WL_CONNECTED)) {
    DBG_PRINT( F("[sendURL] no connection") );
    return false;
  }


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
    DBG_PRINT("[HTTP] GET... code: ");
    DBG_PRINTLN(httpCode);

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
      } else {
        DBG_PRINTLN(F("no card status"));
      }

    } else {
#ifdef DEBUG
      DBG_PRINTLN("NOT OK");
#endif
    }
  } else {
#ifdef DEBUG
    DBG_PRINT( "[HTTP] GET... failed, error: ");
    DBG_PRINTLN( HTTPClient::errorToString(httpCode).c_str());
#endif
  }
  http.end();
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
    //ShowLED(CRGB::Green);
    ShowLED(CRGB::Purple);
    yield();
    tone(PIEZO_PIN, PIEZO_TONE_CALL, 800);
    SendButton();
    delay(1000);
    yield();
    digitalWrite(PIEZO_PIN, LOW);
    ShowLED(CRGB::Black);
    //delay(1000);
  }


}


