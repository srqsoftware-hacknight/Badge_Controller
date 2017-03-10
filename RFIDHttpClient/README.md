RFIDHttpClient Arduino Sketch

To compile and flash the RFIDHttpClient Arduino sketch requires these things:

  * Install recent Arduino software
  * Install ESP8266 board
  * Install FastLED library for WS2812 LED
  * Install MFRC522 library for RFID reader

== Set up Arduino

Install recent Arduino software from http://arduino.cc

Next, install support for ESP8266 (Google for latest procedure).

1) Add to Arduino preferences 'Additional Boards Manager URLs'
This URL: http://arduino.esp8266.com/stable/package_esp8266com_index.json

2) Restart Arduino

3) Install board:
    * click Tools -> Board -> Boards Manager
    * Filter for 'esp8266'
    * Click to install es8266

4) Install libraries:
    * click Sketch -> Include Library -> Manage Libraries
    * Filter for 'fastled'
    * Click to install FastLED v3.1.3 (or later)
    * Filter for '522'
    * Click to install MFRC522 v1.2.0 (or later)

The WeMos boards use the popular WCH341 Interface for USB to Serial.
Install the necessary driver.

== For MAC OS:
NOTE: The Winchiphead driver should work for wch340 and 341.

https://tzapu.com/ch340-ch341-serial-adapters-macos-sierra/

ZIP file: http://blog.sengotta.net/wp-content/uploads/2015/11/CH34x_Install_V1.3.zip
Install per the directions, you might need to reboot  your mac.

Connect the WeMos with the USB cable, the serial line should enumerate within Arduino.

Select: Tools -> Port -> select serial line for 'wch340' entry (not bluetooth entry).

NOTE:
  * If it doesn't appear in Arduino, try rebooting your mac.
  * If the driver file has spaces, it may help to link to a smaller name with no spaces:
  * Example:
    ```
    ln -s /dev/wch340\ serial   /dev/wch340
    ```

== Compile sketch

1) select board type for WeMos mini:
   * Tools -> Board -> "WeMos D1 R2 & Mini"

2) setup wifi_credentials.h to define ssid and wpakey.
This is *not* checked into the repository.  These must be defined for your wifi network.

```
    const char* ssid   = "your_wifi_ssid";
    const char* wpakey = "your_wifi_passwd";
```

3) Compile and flash.
   Note: be sure to use a *communication* usb cable, not just a power charging cable.
   Some cables show connection, but don't actually enumerate to the WCH serial device.

== Usage

  * When registering Wifi, LED shows flashing Red.  It stops flashing upon successfully obtaining IP address.
  * When reading and sending/authenticating the RFID card, LED shows Blue.
  * If card is Denied, LED shows Red about 1 second.
  * If card is Accepted, LED shows Green about 1 second.

== TODO
  * HTTP 'GET' shows example of using encryption key. We should use this someday.
  * Better/More output with flashing LED colors?
  * Output status to Serial for debugging.  We currently do this.


