

// Reader Indicator uses one WS2812 (NeoPixel)

// Yellow when starting up, flash red when no WiFi
// Green = accepted ; Red = Denied


#define NUM_LEDS   1
#define DATA_PIN   D1 // WS2812B led data output on D1/gpio5

// vars....
CRGB leds[NUM_LEDS];


// set LED color
void ShowLED(CRGB color) {
  fill_solid(leds, NUM_LEDS, color);
  FastLED.show();
  //delay(2000);
}


// call from Setup()
void SetupLED() {
  Serial.println("lights start");
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  //LEDS.setBrightness(10); // turn down brightness for indoor testing.
  LEDS.setBrightness(30);
  ShowLED(CRGB::Yellow);

}

