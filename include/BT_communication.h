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
  ELM_PORT.begin("IOS-Vlink");
  
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_GREEN);
  lcd.drawString("Connecting", lcd.width() / 2, lcd.height() / 2 - 50);
  lcd.drawString("To", lcd.width() / 2, lcd.height() / 2);
  lcd.drawString("OBDII", lcd.width() / 2, lcd.height() / 2 + 50);
  lcd.drawString("Device", lcd.width() / 2, lcd.height() / 2 + 100);
  Serial.println("...Connection to OBDII...");
  
  int retries = 0;
  while (!ELM_PORT.connect() && (retries < 4)) // Device name of iCar Vgate pro BT4.0 OBD adapter
  {
    dtostrf(retries,1,0,strRetries);
    //Serial.println("Couldn't connect to OBD scanner - Phase 1");
    lcd.fillScreen(TFT_BLACK);
    lcd.drawString("Couldn't", lcd.width() / 2, lcd.height() / 2 - 100);
    lcd.drawString("connect to", lcd.width() / 2, lcd.height() / 2 - 50);
    lcd.drawString("OBDII", lcd.width() / 2, lcd.height() / 2);
    lcd.drawString("scanner", lcd.width() / 2, lcd.height() / 2 + 50);
    lcd.drawString(" Phase 1", lcd.width() / 2, lcd.height() / 2 + 100); 
    delay(500);
    retries++;   
    lcd.fillScreen(TFT_BLACK);
    lcd.drawString("Connection", lcd.width() / 2, lcd.height() / 2 - 50);
    lcd.drawString("Retry", lcd.width() / 2, lcd.height() / 2);
    lcd.drawString(strRetries, lcd.width() / 2, lcd.height() / 2 + 50);        
  }

  if (!myELM327.begin(ELM_PORT)) // select protocol '6'
  {
    //Serial.println("Couldn't connect to OBD scanner - Phase 2");    
    lcd.fillScreen(TFT_BLACK);    
    lcd.drawString("Couldn't", lcd.width() / 2, lcd.height() / 2 - 50);
    lcd.drawString("connect to", lcd.width() / 2, lcd.height() / 2);
    lcd.drawString("OBDII", lcd.width() / 2, lcd.height() / 2 + 50);
    lcd.drawString("scanner", lcd.width() / 2, lcd.height() / 2 + 100);
    lcd.drawString(" Phase 2", lcd.width() / 2, lcd.height() / 2 + 150);
    delay(500);       
    
    //esp_deep_sleep_start();
  }

  else{
  //Serial.println("Connected to OBDII");
      
  lcd.fillScreen(TFT_BLACK);
  lcd.drawString("Connected",  lcd.width() / 2, lcd.height() / 2 - 50);
  lcd.drawString("to OBDII", lcd.width() / 2, lcd.height() / 2);

  OBD2connected = true;

  delay(500);
  lcd.fillScreen(TFT_BLACK);
  }
}
#endif
