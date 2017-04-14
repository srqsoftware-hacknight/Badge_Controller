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

