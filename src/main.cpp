#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

#define PIN_CS_RFID  15
#define PIN_SCK      16
#define PIN_MISO     17
#define PIN_MOSI     18
#define PIN_RST_RFID 8

MFRC522 rfid(PIN_CS_RFID, PIN_RST_RFID);
MFRC522::MIFARE_Key key;

bool readBlock(byte blockNum, byte* buffer) {
  byte size = 18;
  if (rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(rfid.uid)) != MFRC522::STATUS_OK) 
    return false;
  if (rfid.MIFARE_Read(blockNum, buffer, &size) != MFRC522::STATUS_OK) 
    return false;
  return true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS_RFID);
  rfid.PCD_Init();
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  
  Serial.println("\n=== LECTURA COMPLETA MIFARE 1K ===");
  Serial.println("Acerca una tarjeta...\n");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;
  
  Serial.println("Tarjeta detectada!\n");
  
  // Mostrar UID
  Serial.print("UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) Serial.print("0");
    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");
  }
  Serial.println("\n");
  
  // Tabla de memoria
  Serial.println("Bloque | Datos (HEX)                              | ASCII");
  Serial.println("-------|------------------------------------------|----------------");
  
  byte buffer[18];
  for (byte block = 0; block < 64; block++) {
    // Imprimir número de bloque
    if (block < 10) Serial.print(" ");
    Serial.print(block);
    Serial.print("    | ");
    
    if (readBlock(block, buffer)) {
      // Imprimir HEX
      for (byte i = 0; i < 16; i++) {
        if (buffer[i] < 0x10) Serial.print("0");
        Serial.print(buffer[i], HEX);
        Serial.print(" ");
      }
      Serial.print(" | ");
      
      // Imprimir ASCII
      for (byte i = 0; i < 16; i++) {
        if (buffer[i] >= 32 && buffer[i] <= 126) 
          Serial.print((char)buffer[i]);
        else 
          Serial.print(".");
      }
      Serial.println();
    } else {
      Serial.println("ACCESO DENEGADO         | ................");
    }
    
    delay(50);
  }
  
  Serial.println("\n=== LECTURA COMPLETADA ===\n");
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(3000);
}