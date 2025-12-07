#ifndef BT_COMMUNICATION
#define BT_COMMUNICATION

#include "BLEClientSerial.h"
#include <ELMduino.h>
#include "LGFX_CLASS.h"

BLEClientSerial BLESerial;
#define ELM_PORT   BLESerial

ELM327 myELM327;    //Object for OBD2 device

bool OBD2connected = false;
extern bool showBLESelection;
extern bool bleScreenDrawn;
extern int bleScreenPage;
extern String storedBLEDevice;

void ConnectToOBD2(LGFX& lcd){
  char strRetries[2];   
  
  // Try to initialize BLE - if it fails, handle gracefully
  if (!ELM_PORT.begin(storedBLEDevice.c_str())) {
    Serial.println("BLE initialization failed");
    OBD2connected = false;
    return;
  }
  else {
    Serial.println("BLE initialized successfully");
    Serial.println("...Connection to OBDII...");
  }
  
  int retries = 0;
  while (!ELM_PORT.connect() && (retries < 4)) // Device name of iCar Vgate pro BT4.0 OBD adapter
  {
    Serial.printf("Couldn't connect to OBD scanner - Phase 1 (retry %d)\n", retries);
    delay(500);
    retries++;
  }
  
  // If all connection attempts failed, show selection screen
  if (retries >= 4) {
    Serial.println("Failed to find OBD device after retries");
    showBLESelection = true;
    bleScreenDrawn = false;  // Force initial draw
    bleScreenPage = 0;  // Start with connection failed message
    OBD2connected = false;
    return;
  }

  if (!myELM327.begin(ELM_PORT)) // select protocol '6'
  {
    Serial.println("Couldn't connect to OBD scanner - Phase 2");
  }
  else{
    Serial.println("Connected to OBDII");
    OBD2connected = true;
    extern bool DrawBackground;  // Forward declaration
    DrawBackground = true;  // Ensure display is drawn after connection
  }
}
#endif
