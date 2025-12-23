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

// --- BLE Device Selection UI Pagination ---

#define BLE_DEVICES_PER_PAGE 4 // Only 4 devices per page, plus default

// Function to draw BLE device selection screen with pagination (4 per page + default at bottom)
void drawBLEDeviceSelection(LGFX& lcd, char deviceList[][21], int deviceCount, int page, int selectedIdx) {
  lcd.fillScreen(TFT_BLACK);
  lcd.setFont(&FreeSans18pt7b);
  lcd.setTextColor(TFT_WHITE);
  lcd.drawString("Select OBD2 Device", 160, 40);

  lcd.setFont(&FreeSans9pt7b);
  lcd.setTextColor(TFT_LIGHTGREY);
  if (deviceCount == 0) {
    lcd.drawString("No devices found", 160, 90);
  } else {
    lcd.drawString("Tap device to select:", 160, 90);
  }

  // Calculate which devices to show on this page
  int startIdx = page * BLE_DEVICES_PER_PAGE;
  int endIdx = startIdx + BLE_DEVICES_PER_PAGE;
  if (endIdx > deviceCount) endIdx = deviceCount;

  lcd.setFont(&FreeSans12pt7b);
  // Show up to 4 devices per page (from startIdx)
  // Device buttons at y = 120, 165, 210, 255 (matching touch handler coordinates)
  for (int i = 0; i < BLE_DEVICES_PER_PAGE; ++i) {
    int deviceIdx = startIdx + i;
    if (deviceIdx >= deviceCount) break;
    int yPos = 120 + (i * 45);
    // The selectedIdx passed in should be relative to the current page:
    // -1 = default, 0 = first device on page, 1 = second, etc.
    bool isSelected = (selectedIdx == i);
    lcd.fillRoundRect(20, yPos, 280, 40, 8, isSelected ? TFT_GREEN : TFT_DARKGREY);
    lcd.setTextColor(TFT_WHITE);
    lcd.drawString(deviceList[deviceIdx], 160, yPos + 20);
  }

  // Always show 'Default' option at the bottom (after devices)
  // Default button at y = 300 (120 + 4*45)
  int defaultYPos = 120 + (BLE_DEVICES_PER_PAGE * 45);
  bool defaultSelected = (selectedIdx == -1);
  lcd.fillRoundRect(20, defaultYPos, 280, 40, 8, defaultSelected ? TFT_GREEN : TFT_BLUE);
  lcd.setTextColor(TFT_WHITE);
  lcd.setFont(&FreeSans9pt7b);
  lcd.drawString("Use Default (IOS-Vlink)", 160, defaultYPos + 20);

  // Draw page navigation if needed (PREV/NEXT buttons at top: y=350-390)
  if (deviceCount > BLE_DEVICES_PER_PAGE) {
    int totalPages = (deviceCount + BLE_DEVICES_PER_PAGE - 1) / BLE_DEVICES_PER_PAGE;
    if (page > 0) {
      lcd.fillRoundRect(20, 350, 90, 40, 8, TFT_ORANGE);
      lcd.setTextColor(TFT_WHITE);
      lcd.drawString("PREV", 65, 370);
    }
    if (page < totalPages - 1) {
      lcd.fillRoundRect(210, 350, 90, 40, 8, TFT_ORANGE);
      lcd.setTextColor(TFT_WHITE);
      lcd.drawString("NEXT", 255, 370);
    }
  }

  // Always draw Scan, Back, and OK buttons at bottom (y=400-450)
  lcd.fillRoundRect(20, 400, 90, 50, 10, TFT_ORANGE);
  lcd.setTextColor(TFT_WHITE);
  lcd.setFont(&FreeSans9pt7b);
  lcd.drawString("SCAN", 65, 425);
  
  lcd.fillRoundRect(120, 400, 90, 50, 10, TFT_RED);
  lcd.drawString("BACK", 165, 425);
  
  lcd.fillRoundRect(220, 400, 80, 50, 10, TFT_GREEN);
  lcd.drawString("OK", 260, 425);
}

void ConnectToOBD2(LGFX& lcd){
  char strRetries[2];
  char deviceDisplay[64];

  // Only draw connection phase messages at the bottom, preserving the rest of the welcome display
  int msgY = 440; // Start below "OBD2: Connecting..."
  //lcd.setFont(&FreeSans12pt7b);
  int msgX = lcd.width() / 2;
  int clearHeight = 160; // Height for connection messages area
  // Clear only the area for connection messages
  lcd.fillRect(0, msgY - 5, lcd.width(), clearHeight, TFT_BLACK);

  // Display device name below "OBD2: Connecting..."
  lcd.setFont(&FreeSans9pt7b);
  lcd.setTextColor(TFT_DARKGREY);
  snprintf(deviceDisplay, sizeof(deviceDisplay), "Device: %s", selectedBLEDevice);
  lcd.drawString(deviceDisplay, msgX, msgY - 5);

  // BLE initialization phase (use the same y as attempt messages)
  int attemptMsgY = msgY + 30;  
  
  // Start BLE intialisation on ESP32-S3  
  if (!ELM_PORT.begin(selectedBLEDevice)) {
    Serial.println("BLE initialization failed");
    // Clear all the area for connection messages including device name
    lcd.fillRect(0, msgY - 5, lcd.width(), clearHeight, TFT_BLACK);
    lcd.setFont(&FreeSans9pt7b);
    lcd.setTextColor(TFT_DARKGREY);
    lcd.drawString(deviceDisplay, msgX, msgY - 5);
    lcd.setFont(&FreeSans12pt7b);
    lcd.setTextColor(TFT_RED);
    lcd.drawString("BLE: Init Failed", msgX, attemptMsgY);
    OBD2connected = false;
    delay(1000);
    lcd.fillRect(0, msgY - 5, lcd.width(), clearHeight, TFT_BLACK);
    return;
  }
  
  // Display "looking for device" with countdown (4 seconds total)
  int rssi;
  bool deviceFound = false;
  for (int countdown = 4; countdown > 0; countdown--) {
    lcd.fillRect(0, msgY - 5, lcd.width(), clearHeight, TFT_BLACK);
    lcd.setFont(&FreeSans9pt7b);
    lcd.setTextColor(TFT_DARKGREY);
    lcd.drawString(deviceDisplay, msgX, msgY - 5);
    
    lcd.setFont(&FreeSans12pt7b);
    lcd.setTextColor(TFT_LIGHTGREY);
    char scanMsg[40];
    snprintf(scanMsg, sizeof(scanMsg), "Looking for device... %d", countdown);
    lcd.drawString(scanMsg, msgX, attemptMsgY);
    Serial.printf("Scanning for device... %d seconds remaining\n", countdown);
    delay(1000);
    // Check if device was found after scan completes
    rssi = ELM_PORT.getRSSI();
    if (rssi != 0) {
      // Device found
      lcd.fillRect(0, 420, lcd.width(), clearHeight, TFT_BLACK);
      lcd.setFont(&FreeSans9pt7b);
      lcd.setTextColor(TFT_DARKGREY);
      snprintf(deviceDisplay, sizeof(deviceDisplay), "Device: %s  Signal: %d dBm", selectedBLEDevice, rssi);
      lcd.drawString(deviceDisplay, msgX, msgY - 5);
      lcd.setFont(&FreeSans12pt7b);
      lcd.setTextColor(TFT_GREEN);
      lcd.drawString("BLE: Device Found", msgX, attemptMsgY);
      //delay(1000);
      Serial.printf("Device found with RSSI: %d dBm\n", rssi);  
      deviceFound = true;
      delay(1500);
      break;
    } 
  } 
  if (deviceFound) {
    // Proceed to OBD2 connection attempts  
    int retries = 0;
    bool connected = false;
    
    for (retries = 0; retries < 4; retries++) {
      Serial.printf("OBD2 connection attempt %d (RSSI: %d)\n", retries + 1, rssi);      
      delay(500); // Give user time to see the message before connect attempt
      if (ELM_PORT.connect()) {
        connected = true;
        lcd.fillRect(0, msgY + 15, lcd.width(), clearHeight - 20, TFT_BLACK);
        lcd.setFont(&FreeSans12pt7b);
        lcd.setTextColor(TFT_GREEN);
        lcd.drawString("BLE: Connected", msgX, attemptMsgY);
        delay(250);
        break;
      }
    }
    delay(500); // Give more time for connection to establish
      
    lcd.setTextColor(TFT_LIGHTGREY);
    lcd.fillRect(0, msgY + 15, lcd.width(), clearHeight - 20, TFT_BLACK);
    lcd.drawString("OBD2: Protocol Init...", msgX, attemptMsgY);
    delay(500); // Give time for display update
    if (!myELM327.begin(ELM_PORT)) // select protocol '6'
    {
      Serial.println("ELM327 protocol initialization failed");
      lcd.setTextColor(TFT_RED);
      lcd.fillRect(0, 420, lcd.width(), 60, TFT_BLACK);
      lcd.drawString("OBD2: Protocol Fail", msgX, attemptMsgY);
      delay(1000); // Brief pause after last attempt before showing failure
      OBD2connected = false;
      extern bool showOBD2FailScreen;
      showOBD2FailScreen = true;
      drawProtocolError(lcd);
      return;
    }
    lcd.setTextColor(TFT_GREEN);
    lcd.fillRect(0, msgY + 15, lcd.width(), clearHeight - 20, TFT_BLACK);
    lcd.drawString("Init. Completed", msgX, attemptMsgY);
    delay(1000);

    // Success: clear the connection message area and show success
    // Clear the message area below the device name
    lcd.fillRect(0, msgY + 15, lcd.width(), clearHeight - 20, TFT_BLACK);
    
    rssi = ELM_PORT.getRSSI();
        
    // Overwrite the 'OBD2: Connecting...' message area with success message
    lcd.setFont(&FreeSans12pt7b);    
    lcd.fillRect(0, msgY - 45, lcd.width(), 180, TFT_BLACK);
    lcd.setTextColor(TFT_GREEN);
    lcd.drawString("OBD2: Connected", msgX, 410);

    // Show device name with signal strength on same line (reuse rssi from connection attempts)
    lcd.setFont(&FreeSans9pt7b);
    lcd.setTextColor(TFT_DARKGREY);
    if (rssi != 0) {
      snprintf(deviceDisplay, sizeof(deviceDisplay), "Device: %s  Signal: %d dBm", ELM_PORT.getDeviceName().c_str(), rssi);
      Serial.printf("Connected with RSSI: %d dBm\n", rssi);
    } else {
      snprintf(deviceDisplay, sizeof(deviceDisplay), "Device: %s  Signal: N/A", ELM_PORT.getDeviceName().c_str());
      Serial.printf("Connected with RSSI: N/A\n");
    }
    lcd.drawString(deviceDisplay, msgX, 435);
    
    OBD2connected = true;

    // Update selectedBLEDevice to match the actual connected device and save to EEPROM only if changed
    extern char selectedBLEDevice[21];
    extern void saveBLEDevice(const char* deviceName);
    char newDeviceName[21];
    strncpy(newDeviceName, ELM_PORT.getDeviceName().c_str(), 20);
    newDeviceName[20] = '\0'; // Ensure null-termination
    if (strncmp(selectedBLEDevice, newDeviceName, 21) != 0) {
      strncpy(selectedBLEDevice, newDeviceName, 21);
      saveBLEDevice(selectedBLEDevice);
      Serial.printf("Successfully connected to: %s (saved to EEPROM)\n", selectedBLEDevice);
    } else {
      Serial.printf("Successfully connected to: %s (already saved)\n", selectedBLEDevice);
    }

  delay(1000);
  // Optionally clear the message area again
  // lcd.fillRect(0, msgY - 5, lcd.width(), clearHeight, TFT_BLACK);

  }
  else {
    // Device not found
    lcd.fillRect(0, 420, lcd.width(), 180, TFT_BLACK);
    int rssi = ELM_PORT.getRSSI();
    if (rssi != 0) {
      snprintf(deviceDisplay, sizeof(deviceDisplay), "Device: %s  Signal: %d dBm", selectedBLEDevice, rssi);
    } else {
      snprintf(deviceDisplay, sizeof(deviceDisplay), "Device: %s  Signal: N/A", selectedBLEDevice);
    }
    // Redraw device name with signal strength
    lcd.setFont(&FreeSans9pt7b);
    lcd.setTextColor(TFT_DARKGREY);      
    lcd.drawString(deviceDisplay, msgX, msgY - 5);
    lcd.setFont(&FreeSans12pt7b);
    lcd.setTextColor(TFT_RED);
    lcd.drawString("BLE: Device not found", msgX, attemptMsgY);
    Serial.println("Device not found");
    delay(1500);

    Serial.println("Failed to connect to OBD2 device after retries");
    delay(500); // Brief pause after last attempt before showing failure
    // Clear only the message area below the device name
    lcd.fillRect(0, msgY + 15, lcd.width(), clearHeight - 20, TFT_BLACK);    
    
    // Show failure message
    lcd.fillRect(0, 440, lcd.width(), 40, TFT_BLACK);
    lcd.setFont(&FreeSans12pt7b);
    lcd.setTextColor(TFT_RED);
    lcd.drawString("OBD2: Connect Failed", msgX, attemptMsgY);
    delay(2000); // Give user time to see the failure message
    OBD2connected = false;
    extern bool showOBD2FailScreen;
    showOBD2FailScreen = true;
    drawBLEConnectionError(lcd);
    return;

  }  
}
#endif
