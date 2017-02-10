#!/bin/bash
#
# flash esp -- a script to flash with bad connection to esp
#
# flaky cable made it difficult to flash the wemos
# use this to catch when device is stable and flash the sketch
#
# Warning: dont use this!
#

while [ ! -f /dev/ttyUSB0 ]; do
    echo -n ''
done

echo "FLASH..."

esptool -vv -cd nodemcu -cb 115200 -cp /dev/ttyUSB0 -ca 0x00000 -cf /tmp/arduino_build_932085/BadgeReader.ino.bin 

