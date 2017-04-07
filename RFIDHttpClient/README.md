# RFIDHttpClient Arduino Sketch

This implementation uses the small inexpensive "WeMos mini" ESP8266 microcontroller boards.

The WeMos boards can be found on Amazon for around $5.

We use the RC-522A RFID reader board with SPI breakout and PC board antenna.  These sell for around $5 and often include a card or key fob.  These are 13.56 MHz RFID keys.

## Set up Arduino

1. Install recent Arduino software from http://arduino.cc

2. Open Arduino software, go to preferences preferences and enter the following URL into the 'Additional Boards Manager URLs' textbox then click OK:
* http://arduino.esp8266.com/stable/package_esp8266com_index.json

3. Restart Arduino

4. Install board:
    1. Click Tools -> Board -> Boards Manager
    2. Filter for 'esp8266'
    3. Click the row for es8266 and click "Install"
	4. Click close after installation completes

5. Install libraries:
    1. Click Sketch -> Include Library -> Manage Libraries
    2. Filter for 'fastled'
    3. Click the row for FastLED v3.1.3 (or later) and click "Install"
    4. Filter for 'MFRC522'
    5. Click the row for MFRC522 v1.2.0 (or later) and click "Install"
	6. Click "Close"

The WeMos boards use the popular WCH341 Interface for USB to Serial. Install the necessary driver as described below (currently only MacOS)

## For MAC OS 10.9 - 10.12:
NOTE: The Winchiphead driver should work for wch340 and 341.

Obtain the ZIP file from: https://blog.sengotta.net/signed-mac-os-driver-for-winchiphead-ch340-serial-bridge/ then extract the files and install the PKG.

Install per the directions, you might need to reboot  your mac.

Open the Arduino IDE

Connect the WeMos with the USB cable, the serial line should register within Arduino.

Select: Tools -> Port -> select serial line '/dev/cu.wchusbserial1410' entry (not the bluetooth entry).

NOTE:
  * If it doesn't appear in Arduino, try rebooting your mac.
  * If the driver file has spaces, it may help to link to a smaller name with no spaces:
  * Example: ` ln -s /dev/cu.wch340\ serial   /dev/wch340 `

## Compile sketch

1. Open the sketch Badge_Controller/RFIDHttpClient/RFIDHttpClient.ino

2. Select board type for WeMos mini:
   1. Tools -> Board -> "WeMos D1 R2 & Mini"

2. Setup wifi_credentials.h to define ssid and wpakey.
   This is *not* checked into the repository.  These must be defined for your wifi network.

```
  $ cd Badge_Controller/RFIDHttpClient
  $ cat > wifi_credentials.h << EOF
const char* ssid   = "your_wifi_ssid";
const char* wpakey = "your_wifi_passwd";
EOF
```

3. Compile and flash.
   Note: be sure to use a *communication* usb cable, not just a power charging cable. Some cables show connection, but don't actually enumerate to the WCH serial device.
   * Click "Sketch -> Verify/Compile"
   * When compilation is complete, click "Sketch -> Upload"
   * Compilation and flashing is complete when the message "Done uploading" is displayed

## Usage

  * When registering WIFI, LED shows flashing Red.  It stops flashing upon successfully obtaining IP address.
  * When reading and sending/authenticating the RFID card, LED shows Blue.
  * If card is Denied, LED shows Red about 1 second.
  * If card is Accepted, LED shows Green about 1 second.
