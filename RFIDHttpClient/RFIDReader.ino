/**
 * RFID reader is Mifare Reader RC522
 * returns 4 byte unique id
 */

#include <SPI.h>
#include <MFRC522.h>  // v 1.2.0

// SS, MOSI, MISO, SCK (gpio15, gpio13, gpio12, gpio14)
// wemos mini: SS=D8, MOSI=D7, MISO=D6, SCK=D5
// There are specific pins for SCK, MISO, MOSI, SS.  Use any pin for RST
#define SS_PIN D8
#define RST_PIN D4 // we use D4/gpio2 (has external 10k pull-up resistor)

// NOTE: gpio2(D4), gpio0(D3) have external 10k pull-up resistors (used in programming)
// We can pull down with 'call' button and check for LOW condition.
// This can be useful to implement a 'Guest Call' button on the reader box.


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

