/**
 * RFID reader is Mifare Reader RC522
 * returns 4 byte unique id
 */

#include <SPI.h>
#include <MFRC522.h>  // v 1.2.0

// See wemos pinout diagram for SS, MOSI, MISO, SCK (wemos mini: D8, D13, D12, D14)
#define SS_PIN D8 // D8/gpio15 for SS
#define RST_PIN D4 // any pin, we use D4/gpio2

// vars....
MFRC522 rfid(SS_PIN, RST_PIN);
// Not using key yet...  MFRC522::MIFARE_Key key;



void SetupRFID() {
  USE_SERIAL.println("spi start");
  SPI.begin();
  rfid.PCD_Init();
}



// #############################################################################
// return true if card available to scan
bool CardAvailable() {
  if ( ! rfid.PICC_IsNewCardPresent()) {
    return false;
  }
  if ( ! rfid.PICC_ReadCardSerial()) {
    return false;
  }
#ifdef DEBUG
  Serial.println("card available");
#endif
  return true;
}




// #############################################################################
// convert 0x0 - 0xF => '0'...'F'
char ZZZ_Hex2Char(byte hex) {
  hex &= 0x0f; // mask lower nibble
  if (hex <= 0x09) {
    return '0' + hex;
  }
  else if (hex <= 0x0F) {
    return 'A' + hex - 0x0A;
  }
  return '*'; // error
}


// #############################################################################
// Return a buffer of bytes as a String of hex values
// NOTE: must pre-allocate str
void ZZZ_Hex2Str(byte *buffer, byte bufferSize, char* str) {

uint32_t ui = (buffer[3]<<24) + (buffer[2]<<16) + (buffer[1]<<8) + (buffer[0]);
sprintf(str,"%010i",ui); // format zero-padded for 10 digits
#ifdef DEBUG
Serial.print ("UI=");
Serial.println (ui, DEC);
Serial.print ("str=");
Serial.println (str);
#endif
}



// #############################################################################
// scan card from rfid reader into 'rfid' struct, must be already present
bool ScanCard(char* cardbuf) {
  // convert 4-byte binary into unsigned int.
  uint32_t ui = (rfid.uid.uidByte[3]<<24) + (rfid.uid.uidByte[2]<<16) + (rfid.uid.uidByte[1]<<8) + (rfid.uid.uidByte[0]);
  sprintf(cardbuf,"%010i",ui); // format zero-padded for 10 digits

  // reset RFID, to fix glitch/hang.
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  // mfr522 cards should be 4 bytes
  if (rfid.uid.size != 4)  {
      Serial.println("ERROR: uid!=4");
      return false;
  }
  return true;
}

