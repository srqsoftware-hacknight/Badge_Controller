// vERsion 0.1.11
#include <Arduino.h>

// ########################### Global Variables ################################
// If you're new to microcontroller programming, they are little state machines
// and so often have many ugly global variables. Don't be alarmed.

//Indicator lights
#include "FastLED.h"
#define NUM_LEDS 1
#define DATA_PIN D1
unsigned long strobeTime=millis();
unsigned long strobePeriod=500;
CRGB leds[NUM_LEDS];
bool blinking=false;

//Wifi system
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti WiFiMulti;
#include <ESP8266HTTPClient.h>
WiFiClient client;
bool connected=false;
const char* ssid   ="TheDoormouse";
const char* wpakey ="alicechasingrabbits";
String nodename = "";
IPAddress gateway;

//RFID Reader
#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN 2
#define RST_PIN 4
MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
byte nuidPICC[3];
String CardID="";
String CardState="waiting"; //waiting,submitted,approved,denied (internal state of the machine reading the card, not the card's status in the DB)
String CardStateBuffer="";
unsigned long CardTimeout=millis();
unsigned long CardTimeoutPeriod=5000;

//http
HTTPClient http;
const char* host     = "172.31.0.1";
const uint16_t port = 8080;
const char* endpoint = "/rfid/check";
#define SERVER_ACCEPT "accept" //what does the server return when card is valid

// ################################ Main #######################################
void setup() {
     //lights
  	  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
     //Wifi
      WiFi.mode(WIFI_STA);
      nodename=get_nodename();
      WiFi.hostname(nodename);
      WiFiMulti.addAP(ssid, wpakey);
    //RFID
      SPI.begin();
      rfid.PCD_Init();
}

void loop() {
 updateCardState();
 updateHttpConnection();
 updateLED();
 updateWifi();
}
// #############################################################################


// ######################## Support Functions ##################################

// #############################################################################
String ScanCard() {
 String temp="";
 //No card detected, return empty
  if ( ! rfid.PICC_IsNewCardPresent()) {return temp;}
  if ( ! rfid.PICC_ReadCardSerial()) {return temp;}
 //Card detected, return the uid of the card
  temp = printHex(rfid.uid.uidByte, rfid.uid.size);
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  return temp;
}

// #############################################################################
void updateCardState(){
  if ((millis() - CardTimeout) > CardTimeoutPeriod) {
    CardTimeout=millis();
    CardState="waiting";
    return;
  }
  // If we're waiting for an answer or acting on one, exit before scanning for new card
   if (CardState=="approved")   {return;}
   if (CardState=="denied")     {return;}
   if (CardState=="submitted")  {return;}

  CardID=ScanCard();
  if (CardID!=""){SubitCard();}
}

// #############################################################################
void SubitCard(){
 CardStateBuffer="";
 if ( client.connect(host, port) ) {
  String request="GET "+endpoint+"?device_id="+nodename+"&badge_id="+CardID+" HTTP/1.1";
  client.println(request);
  client.print("Host: ");
  client.println(host);
  client.println("Connection: close");
  client.println();
 }
 CardState="submitted";
 CardTimeout=millis();
}

// #############################################################################
void updateHttpConnection(){
 if (CardState!="submitted"){return;}
 if ( client.available() ) {
  char c = client.read();
  CardStateBuffer=CardStateBuffer+c;
 }
 if ( !client.connected() ) {
  if (CardStateBuffer.indexOf(SERVER_ACCEPT) >=0) {CardState="approved";}
   else {CardState="denied";}
  CardTimeout=millis();
  CardStateBuffer="";
 }
}

// #############################################################################
void updateWifi(){
 if(WiFiMulti.run() == WL_CONNECTED) {
   connected=true;
 } else {
   connected=false;
 }
}

// #############################################################################
String get_nodename(){
  String temp;
  byte mac[6];
  WiFi.macAddress(mac);
  temp=printHex(mac,6);
  temp="DU_"+temp;
  return temp;
}

// #############################################################################
void updateLED() {
  if (connected){blinking=false;}else{blinking=true;}
  CRGB color=CRGB::Blue;
   if(CardState=="submitted") {color=CRGB::Yellow;}
   if(CardState=="approved")  {color=CRGB::Green;}
   if(CardState=="denied")    {color=CRGB::Red;}

  if ((millis() - strobeTime) < 0.5*strobePeriod) {fill_solid(leds, NUM_LEDS,color);}
   else { if(blinking){fill_solid(leds, NUM_LEDS,CRGB::Black);} }
  FastLED.show();
  if ((millis() - strobeTime) > strobePeriod) {strobeTime=millis();}
}

// #############################################################################
// Return a buffer of bytes as a String of 2 character hex values
String printHex(byte *buffer, byte bufferSize) {
  String output;
  for (byte i = 0; i < bufferSize; i++) {
    output=output+(buffer[i] < 0x10 ? "0" : "");
    output=output+String(buffer[i], HEX);
  }
  return output;
}
