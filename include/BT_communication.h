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

// Holds the selected BLE device name (max 20 chars + null terminator)
extern char selectedBLEDevice[21];

void ConnectToOBD2(LGFX& lcd){
  char strRetries[2];

  // Only draw connection phase messages at the bottom, preserving the rest of the welcome display
  int msgY = 440; // Start below "OBD2: Connecting..."
  lcd.setFont(&FreeSans12pt7b);
  int msgX = lcd.width() / 2;
  int clearHeight = 160; // Height for connection messages area
  // Clear only the area for connection messages
  lcd.fillRect(0, msgY - 5, lcd.width(), clearHeight, TFT_BLACK);

  // BLE initialization phase (use the same y as attempt messages)
  int attemptMsgY = msgY + 30;
  // Always clear the whole connection message area before each new message
  lcd.fillRect(0, msgY - 5, lcd.width(), clearHeight, TFT_BLACK);
  lcd.setTextColor(TFT_LIGHTGREY);
  lcd.drawString("BLE: Initializing...", msgX, attemptMsgY);
  if (!ELM_PORT.begin(selectedBLEDevice)) {
    Serial.println("BLE initialization failed");
    lcd.fillRect(0, msgY - 5, lcd.width(), clearHeight, TFT_BLACK);
    lcd.setTextColor(TFT_RED);
    lcd.drawString("BLE: Init Failed", msgX, attemptMsgY);
    OBD2connected = false;
    delay(2000);
    // Optionally clear the message area again
    lcd.fillRect(0, msgY - 5, lcd.width(), clearHeight, TFT_BLACK);
    return;
  }

  // OBD2 connection attempts
  int retries = 0;
  bool connected = false;
  for (retries = 0; retries < 4; retries++) {
    // Always clear the whole connection message area before each new message
    lcd.fillRect(0, msgY - 5, lcd.width(), clearHeight, TFT_BLACK);
    char attemptMsg[32];
    snprintf(attemptMsg, sizeof(attemptMsg), "OBD2: Attempt %d...", retries + 1);
    lcd.setTextColor(TFT_LIGHTGREY);
    lcd.drawString(attemptMsg, msgX, attemptMsgY);
    Serial.printf("OBD2 connection attempt %d\n", retries + 1);
    delay(600); // Give user time to see the message before connect attempt
    if (ELM_PORT.connect()) {
      connected = true;
      break;
    }
    delay(1500); // Give more time for connection to establish
  }

  // Check if connection failed after retries
  if (!connected) {
    Serial.println("Failed to connect to OBD2 device after retries");
    // Always clear the whole connection message area before showing failure
    lcd.fillRect(0, msgY - 5, lcd.width(), clearHeight, TFT_BLACK);
    lcd.setTextColor(TFT_RED);
    lcd.drawString("OBD2: Connect Failed", msgX, attemptMsgY);
    delay(1200); // Give user time to see the failure message
    OBD2connected = false;
    extern bool showOBD2FailScreen;
    showOBD2FailScreen = true;
    drawBLEConnectionError(lcd);
    return;
  }

  // Protocol initialization phase
  lcd.setTextColor(TFT_LIGHTGREY);
  lcd.drawString("OBD2: Protocol Init...", msgX, msgY + 30 + 4 * 25);
  if (!myELM327.begin(ELM_PORT)) // select protocol '6'
  {
    Serial.println("ELM327 protocol initialization failed");
    lcd.setTextColor(TFT_RED);
    lcd.drawString("OBD2: Protocol Fail", msgX, msgY + 30 + 5 * 25);
    OBD2connected = false;
    extern bool showOBD2FailScreen;
    showOBD2FailScreen = true;
    drawProtocolError(lcd);
    return;
  }

  // Success: update 'OBD2: Connecting...' to 'OBD2: Connected' in green, and show device name below
  // Overwrite the 'OBD2: Connecting...' message area
  lcd.setTextColor(TFT_BLACK);
  lcd.drawString("OBD2: Connecting...", msgX, 410); // Clear previous message
  lcd.setTextColor(TFT_GREEN);
  lcd.drawString("OBD2: Connected", msgX, 410);

  // Show device name just below
  lcd.setFont(&FreeSans9pt7b);
  lcd.setTextColor(TFT_DARKGREY);
  char deviceDisplay[64];
  snprintf(deviceDisplay, sizeof(deviceDisplay), "Device: %s", ELM_PORT.getDeviceName().c_str());
  lcd.drawString(deviceDisplay, msgX, 435);
  lcd.setFont(&FreeSans12pt7b);

  OBD2connected = true;

  // Update selectedBLEDevice to match the actual connected device and save to EEPROM
  extern char selectedBLEDevice[21];
  extern void saveBLEDevice(const char* deviceName);
  strncpy(selectedBLEDevice, ELM_PORT.getDeviceName().c_str(), 20);
  selectedBLEDevice[20] = '\0'; // Ensure null-termination
  saveBLEDevice(selectedBLEDevice);
  Serial.printf("Successfully connected to: %s (saved to EEPROM)\n", selectedBLEDevice);

  delay(1500);
  // Optionally clear the message area again
  // lcd.fillRect(0, msgY - 5, lcd.width(), clearHeight, TFT_BLACK);
}
#endif
