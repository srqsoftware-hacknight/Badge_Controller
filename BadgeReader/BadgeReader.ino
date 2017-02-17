// vERsion 0.1.11
#include <Arduino.h>

// ########################### Global Variables ################################
// If you're new to microcontroller programming, they are little state machines
// and so often have many ugly global variables. Don't be alarmed.

// Initial wiring used "Wemos D1 mini" esp8266 controller from Amazon.

//Indicator lights
#include "FastLED.h"
#define NUM_LEDS 1
#define DATA_PIN D1 // ws2812b led data output on D1/gpio5
unsigned long strobeTime = millis();
unsigned long strobePeriod = 500;
CRGB leds[NUM_LEDS];
bool blinking = false;

//Wifi system
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti WiFiMulti;
#include <ESP8266HTTPClient.h>
WiFiClient client;
bool connected = false;

//const char* ssid   ="TheDoormouse";
//const char* wpakey ="alicechasingrabbits";
#include "wifi_credentials.h" // set ssid,wpakey in file NOT exposed to public git repository

String nodename = "";
IPAddress gateway;

//RFID Reader
#include <SPI.h>
#include <MFRC522.h>  // v 1.2.0
// See wemos pinout diagram for SS, MOSI, MISO, SCK (wemos mini: D8, D13, D12, D14)
#define SS_PIN D8 // D8/gpio15 for SS
#define RST_PIN D4 // any pin, we use D4/gpio2
MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
// byte nuidPICC[3];
String CardID = "";
String CardState = "waiting"; //waiting,submitted,approved,denied (internal state of the machine reading the card, not the card's status in the DB)
String CardStateBuffer = "";
unsigned long CardTimeout = millis();
unsigned long CardTimeoutPeriod = 5000;
unsigned long HeartBeatTimeout = millis();
unsigned long HeartBeatPeriod = 5000;

//http
HTTPClient http;
const char* host     = "172.31.0.1";
const uint16_t port = 8080;
const char* endpoint = "/device/check";
#define SERVER_ACCEPT "accept" //what does the server return when card is valid

// ################################ Main #######################################
void setup() {
  delay(3000);
  Serial.begin(115200);
  delay(500);
  Serial.println("reader start");
  
  //lights
  Serial.println("lights start");
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  LEDS.setBrightness(10); // turn down brightness for indoor testing.
  
  //Wifi
  Serial.println("wifi disconnect");
  // delete old config
  WiFi.disconnect(true);  
  delay(1000);
  Serial.println("wifi begin");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, wpakey);

  //while (WiFi.status() != WL_CONNECTED) {
  while (WiFi.run() != WL_CONNECTED) {
    delay(1000);
    Serial.println("x");
  }
  
  nodename = get_nodename();
  Serial.print("nodename=");
  Serial.println(nodename);
  WiFi.hostname(nodename);
  Serial.println("Joining network");
//  WiFiMulti.addAP(ssid, wpakey);
//  while(WiFiMulti.run() != WL_CONNECTED) {
//      delay(100);
//      Serial.print(".");
//      yield();
//  }

  //RFID
  Serial.println("spi start");
  SPI.begin();
  rfid.PCD_Init();
  
  Serial.println("lamp test");
  lampTest(3);
  Serial.println("end setup");
}

void loop() {
  updateCardState();
//  updateHttpConnection();
  updateLED();
  updateWifi();
  updateHeartBeat(); // only for testing
}
// #############################################################################


// ######################## Support Functions ##################################

// #############################################################################
String ScanCard() {
  String temp = "";
  //No card detected, return empty
  if ( ! rfid.PICC_IsNewCardPresent()) {
    return temp;
  }
  if ( ! rfid.PICC_ReadCardSerial()) {
    return temp;
  }
  Serial.println("det card");
  //Card detected, return the uid of the card
  temp = printHex(rfid.uid.uidByte, rfid.uid.size);
  Serial.print("temp=");
  Serial.println(temp);
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  return temp;
}

// #############################################################################
void updateCardState() {
  if ((millis() - CardTimeout) > CardTimeoutPeriod) {
    CardTimeout = millis();
    CardState = "waiting";
    return;
  }
  // If we're waiting for an answer or acting on one, exit before scanning for new card
  if (CardState == "approved")   {
    return;
  }
  if (CardState == "denied")     {
    return;
  }
  if (CardState == "submitted")  {
    return;
  }

  CardID = ScanCard();
//  Serial.print("CardID=");
//  Serial.println(CardID);
  if (CardID != "") {
    SubitCard();
  }
}

// #############################################################################
void SubitCard() {
  CardStateBuffer = "";
  if ( client.connect(host, port) ) {
    char request[80];
    sprintf(request, "GET %s?device_id=%s&badge_id=%s HTTP/1.1", endpoint, nodename.c_str(), CardID.c_str() );
    //  String request="GET "+endpoint+"?device_id="+nodename+"&badge_id="+CardID+" HTTP/1.1";
    client.println(request);
    client.print("Host: ");
    client.println(host);
    client.println("Connection: close");
    client.println();
  }
  CardState = "submitted";
  CardTimeout = millis();
}

// #############################################################################
void updateHttpConnection() {
  if (CardState != "submitted") {
    return;
  }
  if ( client.available() ) {
    char c = client.read();
    CardStateBuffer = CardStateBuffer + c;
  }
  if ( !client.connected() ) {
    if (CardStateBuffer.indexOf(SERVER_ACCEPT) >= 0) {
      CardState = "approved";
    }
    else {
      CardState = "denied";
    }
    CardTimeout = millis();
    CardStateBuffer = "";
  }
}

// #############################################################################
void updateWifi() {
  if (WiFiMulti.run() == WL_CONNECTED) {
    connected = true;
  } else {
    connected = false;
  }
}

// #############################################################################
String get_nodename() {
  String temp;
  byte mac[6];
  WiFi.macAddress(mac);
  temp = printHex(mac, 6);
  temp = "DU_" + temp;
  return temp;
}

// #############################################################################
void updateLED() {
  if (connected) {
    blinking = false;
  } else {
    blinking = true;
  }
  CRGB color = CRGB::Blue;
  if (CardState == "submitted") {
    color = CRGB::Yellow;
  }
  if (CardState == "approved")  {
    color = CRGB::Green;
  }
  if (CardState == "denied")    {
    color = CRGB::Red;
  }

  if ((millis() - strobeTime) < 0.5 * strobePeriod) {
    fill_solid(leds, NUM_LEDS, color);
  }
  else {
    if (blinking) {
      fill_solid(leds, NUM_LEDS, CRGB::Black);
    }
  }
  FastLED.show();
  if ((millis() - strobeTime) > strobePeriod) {
    strobeTime = millis();
  }
}

pwd


// #############################################################################
// heartbeat to show alive
void updateHeartBeat() {
  if ((millis() - HeartBeatTimeout) > HeartBeatPeriod) {
    Serial.println("HHH");
    HeartBeatTimeout = millis();
    //lampTest(1); // blocking call
  }
}

// #############################################################################
// cycle LED on setup, display colors for 3 rounds.
void lampTest(int num) {
  CRGB colors[] = { CRGB::Red, CRGB::Yellow, CRGB::Green, CRGB::Black};
#define numTestColors 4
  for (int i = 0; i < numTestColors * num; i++) {
    fill_solid(leds, NUM_LEDS, colors[i % numTestColors]);
    FastLED.show();
    delay(300);
  }
}


// #############################################################################
// Return a buffer of bytes as a String of 2 character hex values
String printHex(byte *buffer, byte bufferSize) {
  String output;
  for (byte i = 0; i < bufferSize; i++) {
    output = output + (buffer[i] < 0x10 ? "0" : "");
    output = output + String(buffer[i], HEX);
  }
  return output;
}
