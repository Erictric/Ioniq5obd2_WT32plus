#ifndef BT_COMMUNICATION
#define BT_COMMUNICATION

#include "BLEClientSerial.h"
#include <ELMduino.h>
#include "LGFX_CLASS.h"

BLEClientSerial BLESerial;
#define ELM_PORT   BLESerial

ELM327 myELM327;    //Object for OBD2 device

bool OBD2connected = false;

void ConnectToOBD2(LGFX& lcd){
  char strRetries[2];  
  
  if (!ELM_PORT.begin("IOS-Vlink")) {
    Serial.println("BLE initialization failed");
    lcd.setTextColor(TFT_RED);
    lcd.drawString("OBDII BLE", lcd.width() / 2, 310);
    lcd.drawString("Init Failed", lcd.width() / 2, 340);
    OBD2connected = false;
    delay(2000);
    lcd.fillScreen(TFT_BLACK);
    return;
  }
  
  int retries = 0;
  while (!ELM_PORT.connect() && (retries < 4)) // Device name of iCar Vgate pro BT4.0 OBD adapter
  {
    Serial.printf("OBD2 connection attempt %d\n", retries);
    retries++;   
    delay(500);
  }

  // Check if connection failed after retries
  if (retries >= 4) {
    Serial.println("Failed to connect to OBD2 device after retries");
    OBD2connected = false;
    extern bool showOBD2FailScreen;
    showOBD2FailScreen = true;
    
    // Show OBD2 connection failure screen with buttons
    lcd.fillScreen(TFT_BLACK);
    lcd.setFont(&FreeSans18pt7b);
    lcd.setTextColor(TFT_RED);
    lcd.drawString("OBD2 Connection", 160, 80);
    lcd.drawString("Failed", 160, 120);
    
    lcd.setFont(&FreeSans12pt7b);
    lcd.setTextColor(TFT_WHITE);
    lcd.drawString("Device not found", 160, 180);
    lcd.setFont(&FreeSans9pt7b);
    lcd.setTextColor(TFT_LIGHTGREY);
    lcd.drawString("Check if device is powered", 160, 210);
    
    // Retry button (left)
    lcd.fillRoundRect(20, 280, 130, 60, 10, TFT_GREEN);
    lcd.setTextColor(TFT_WHITE);
    lcd.drawString("RETRY", 85, 310);
    
    // Continue button (right)
    lcd.fillRoundRect(170, 280, 130, 60, 10, TFT_BLUE);
    lcd.drawString("CONTINUE", 235, 310);
    
    return;
  }

  if (!myELM327.begin(ELM_PORT)) // select protocol '6'
  {
    Serial.println("ELM327 protocol initialization failed");
    OBD2connected = false;
    extern bool showOBD2FailScreen;
    showOBD2FailScreen = true;
    
    // Show OBD2 protocol failure screen with buttons
    lcd.fillScreen(TFT_BLACK);
    lcd.setFont(&FreeSans18pt7b);
    lcd.setTextColor(TFT_RED);
    lcd.drawString("OBD2 Protocol", 160, 80);
    lcd.drawString("Failed", 160, 120);
    
    lcd.setFont(&FreeSans12pt7b);
    lcd.setTextColor(TFT_WHITE);
    lcd.drawString("Cannot initialize ELM327", 160, 180);
    
    // Retry button (left)
    lcd.fillRoundRect(20, 280, 130, 60, 10, TFT_GREEN);
    lcd.setTextColor(TFT_WHITE);
    lcd.drawString("RETRY", 85, 310);
    
    // Continue button (right)
    lcd.fillRoundRect(170, 280, 130, 60, 10, TFT_BLUE);
    lcd.drawString("CONTINUE", 235, 310);
    
    return;
  }

  else{
  //Serial.println("Connected to OBDII");
  
  // Clear "Connecting To OBDII" message
  lcd.setTextColor(TFT_BLACK);
  lcd.drawString("Connecting To OBDII", lcd.width() / 2, 280);
  
  // Show connected message
  lcd.setTextColor(TFT_GREEN);
  lcd.drawString("OBDII Connected", lcd.width() / 2, 280);

  OBD2connected = true;

  delay(1500);
  }
}
#endif
