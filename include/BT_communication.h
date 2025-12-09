#ifndef BT_COMMUNICATION
#define BT_COMMUNICATION

#include "BLEClientSerial.h"
#include <ELMduino.h>
#include "LGFX_CLASS.h"

BLEClientSerial BLESerial;
#define ELM_PORT   BLESerial

ELM327 myELM327;    //Object for OBD2 device

bool OBD2connected = false;

// Function to draw BLE connection error screen
void drawBLEConnectionError(LGFX& lcd) {
  lcd.fillScreen(TFT_BLACK);
  lcd.setFont(&FreeSans18pt7b);
  lcd.setTextColor(TFT_RED);
  lcd.drawString("CONNECTION", 160, 80);
  lcd.drawString("ERROR", 160, 120);
  
  lcd.setFont(&FreeSans12pt7b);
  lcd.setTextColor(TFT_ORANGE);
  lcd.drawString("BLE: Device Not Found", 160, 180);
  
  lcd.setTextColor(TFT_LIGHTGREY);
  lcd.setFont(&FreeSans9pt7b);
  lcd.drawString("Check if device is powered", 160, 240);
  
  // Select Device button (top)
  lcd.fillRoundRect(60, 300, 200, 50, 10, TFT_ORANGE);
  lcd.setTextColor(TFT_WHITE);
  lcd.setFont(&FreeSans12pt7b);
  lcd.drawString("SELECT DEVICE", 160, 325);
  
  // Retry button (bottom left)
  lcd.fillRoundRect(20, 380, 130, 60, 10, TFT_GREEN);
  lcd.drawString("RETRY", 85, 410);
  
  // Continue button (bottom right)
  lcd.fillRoundRect(170, 380, 130, 60, 10, TFT_BLUE);
  lcd.drawString("CONTINUE", 235, 410);
}

// Function to draw protocol error screen
void drawProtocolError(LGFX& lcd) {
  lcd.fillScreen(TFT_BLACK);
  lcd.setFont(&FreeSans18pt7b);
  lcd.setTextColor(TFT_RED);
  lcd.drawString("PROTOCOL", 160, 80);
  lcd.drawString("ERROR", 160, 120);
  
  lcd.setFont(&FreeSans12pt7b);
  lcd.setTextColor(TFT_ORANGE);
  lcd.drawString("OBD2: Initialization Failed", 160, 180);
  
  lcd.setTextColor(TFT_LIGHTGREY);
  lcd.drawString("Cannot initialize ELM327", 160, 240);
  
  // Select Device button (top)
  lcd.fillRoundRect(60, 300, 200, 50, 10, TFT_ORANGE);
  lcd.setTextColor(TFT_WHITE);
  lcd.setFont(&FreeSans12pt7b);
  lcd.drawString("SELECT DEVICE", 160, 325);
  
  // Retry button (bottom left)
  lcd.fillRoundRect(20, 380, 130, 60, 10, TFT_GREEN);
  lcd.drawString("RETRY", 85, 410);
  
  // Continue button (bottom right)
  lcd.fillRoundRect(170, 380, 130, 60, 10, TFT_BLUE);
  lcd.drawString("CONTINUE", 235, 410);
}

void ConnectToOBD2(LGFX& lcd){
  char strRetries[2];
  extern String selectedBLEDevice;
  
  if (!ELM_PORT.begin(selectedBLEDevice.c_str())) {
    Serial.println("BLE initialization failed");
    lcd.fillScreen(TFT_BLACK);
    lcd.setFont(&FreeSans18pt7b);
    lcd.setTextColor(TFT_RED);
    lcd.drawString("SYSTEM", lcd.width() / 2, 250);
    lcd.drawString("ERROR", lcd.width() / 2, 290);
    
    lcd.setFont(&FreeSans12pt7b);
    lcd.setTextColor(TFT_ORANGE);
    lcd.drawString("BLE: Init Failed", lcd.width() / 2, 330);
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
    drawBLEConnectionError(lcd);
    
    return;
  }

  if (!myELM327.begin(ELM_PORT)) // select protocol '6'
  {
    Serial.println("ELM327 protocol initialization failed");
    OBD2connected = false;
    extern bool showOBD2FailScreen;
    showOBD2FailScreen = true;
    
    // Show OBD2 protocol failure screen with buttons
    drawProtocolError(lcd);
    
    return;
  }

  else{
  //Serial.println("Connected to OBDII");
  
  // Clear "Connecting" message
  lcd.setTextColor(TFT_BLACK);
  lcd.drawString("OBD2: Connecting...", lcd.width() / 2, 410);
  
  // Show connected message
  lcd.setTextColor(TFT_GREEN);
  lcd.drawString("OBD2: Connected", lcd.width() / 2, 410);
  
  // Show device name
  lcd.setFont(&FreeSans9pt7b);
  lcd.setTextColor(TFT_DARKGREY);
  char deviceDisplay[64];
  snprintf(deviceDisplay, sizeof(deviceDisplay), "Device: %s", ELM_PORT.getDeviceName().c_str());
  lcd.drawString(deviceDisplay, lcd.width() / 2, 435);
  lcd.setFont(&FreeSans12pt7b);

  OBD2connected = true;
  
  // Update selectedBLEDevice to match the actual connected device and save to EEPROM
  extern String selectedBLEDevice;
  extern void saveBLEDevice(String deviceName);
  selectedBLEDevice = ELM_PORT.getDeviceName();
  saveBLEDevice(selectedBLEDevice);
  Serial.printf("Successfully connected to: %s (saved to EEPROM)\n", selectedBLEDevice.c_str());

  delay(1500);
  }
}
#endif
