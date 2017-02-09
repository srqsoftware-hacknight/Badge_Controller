
=BadgeReader Notes

==FastLED for Neopixel indicator
Use Arduino 'manage libraries' to install `FastLED 3.1.3` or newer.
Older versions like 3.0.3 dont support ESP8266 properly.

==Mifare RFID reader
Use Arduino 'manage libraries' to install `MFRC522`
Connect using SPI bus.

==USB setup for Wemos

Add the 'wemos' usb id to udev so we can setup chmod correctly.
Otherwise, /dev/ttyUSB0 is rw by root only, not normal users.

```
cat /etc/udev/rules.d/99-CH341.rules 
SUBSYSTEM=="usb", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="5512", MODE="0666"
SUBSYSTEM=="usb", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="7523", MODE="0666"
```


