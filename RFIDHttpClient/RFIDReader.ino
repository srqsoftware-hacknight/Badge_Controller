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
//MFRC522::MIFARE_Key key;



void SetupRFID() {
  //RFID
  Serial.println("spi start");
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
  //Serial.println("card available");
  return true;
}




// #############################################################################
// convert 0x0 - 0xF => '0'...'F'
char Hex2Char(byte hex) {
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
void Hex2Str(byte *buffer, byte bufferSize, char* str) {
  str[0] = '\0';
  byte s = 0;
  //Serial.println();
  //Serial.print("Hex2Str: ");
  for (byte i = 0; i < bufferSize; i += 1) {
    //Serial.print("[");
    //Serial.print(i);
    //Serial.print("]=");
    //Serial.print(buffer[i],HEX);
    //Serial.print(" ");
    str[s++] =  Hex2Char(buffer[i] >> 4); // upper nibble
    str[s++] =  Hex2Char(buffer[i]);      // lower nibble
    //str[s++] =  ' '; // insert space (is this needed?)
  }
  str[s] =  '\0';
  //Serial.println();
}



// #############################################################################
// scan card from rfid reader, must be already present
bool ScanCard(char* cardbuf) {
  //Serial.print("detect card, size=");
  //Serial.println(rfid.uid.size);
  Hex2Str(rfid.uid.uidByte, rfid.uid.size, cardbuf);
  //Serial.print("cardbuf=");
  //Serial.println(cardbuf);
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  // mfr522 cards should be 4 bytes
  if (rfid.uid.size != 4)  {
      Serial.println("ERROR: uid!=4");
      return false;
  }
  return true;
}

