#ifndef WIFI_CONNECTION
#define WIFI_CONNECTION

#include <WiFi.h>
//#include "WiFiMulti.h"
#include <WiFiClient.h>
#include "LGFX_CLASS.h"

//WiFiMulti wifiMulti;

const char* ssid2 = "VIRGIN131";
const char* password2 = "3D4F2F3311D5";
const char* ssid = "WT32-SC01";
const char* password = "5311Fond";
float init_time;
float connect_time = 0;
bool firstTry = false;
bool send_enabled = false;
bool initscan = false;

void ConnectWifi(LGFX& lcd, uint16_t Wifi_select){
  
  char strConnectTime[3];
  //Serial.println("Connecting to Wifi "); 

  //Serial.print("Wifi_Select=  ");
  //Serial.println(Wifi_select);  
  
  //wifiMulti.addAP(ssid, password);
  //wifiMulti.addAP(ssid2, password2);
  
  //WiFi.disconnect();
  if (Wifi_select == 0){
    WiFi.begin(ssid, password);
  }
  else{
    WiFi.begin(ssid2, password2);
  }
    
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_GREEN);
  //lcd.setTextSize(1);
  //lcd.setFreeFont(&FreeSans12pt7b);
  lcd.drawString("Connecting", lcd.width() / 2, lcd.height() / 2 - 50);
  lcd.drawString("To", lcd.width() / 2, lcd.height() / 2);
  lcd.drawString("Wifi", lcd.width() / 2, lcd.height() / 2 + 50);
  
  while (WiFi.status() != WL_CONNECTED  && (connect_time < 15)) { // 2 attempts
    if(firstTry){
      connect_time = (millis() - init_time) / 1000;
      dtostrf(connect_time,1,0,strConnectTime);        
      //Serial.print("attempts: ");Serial.println(retries);
      lcd.fillScreen(TFT_BLACK);
      lcd.drawString("Retring", lcd.width() / 2, lcd.height() / 2 - 50);
      lcd.drawString(strConnectTime, lcd.width() / 2, lcd.height() / 2);
    }    
    else{
      init_time = millis();
      firstTry = true;
    }
    delay(1000);     
  }
  //Serial.println("");

  if (WiFi.status() == WL_CONNECTED) {
    connect_time = (millis() - init_time) / 1000;
    //Serial.print("WiFi connected in: "); 
    //Serial.print(connect_time);
    //Serial.print(" secs");
    //Serial.print(", IP address: "); 
    //Serial.println(WiFi.localIP());
  
    lcd.fillScreen(TFT_BLACK);
    lcd.drawString("Wifi", lcd.width() / 2, lcd.height() / 2 - 50);
    lcd.drawString("Connected", lcd.width() / 2, lcd.height() / 2); 
    firstTry = false;
    send_enabled = true;
    initscan = true;  // To write header name on Google Sheet on power up 
    delay(500);  
  }
  else
  {
    //Serial.print("Failed to connect"); 
     
    lcd.fillScreen(TFT_BLACK);
    lcd.drawString("Wifi", lcd.width() / 2, lcd.height() / 2 - 50);
    lcd.drawString("Failed", lcd.width() / 2, lcd.height() / 2);
    lcd.drawString("To", lcd.width() / 2, lcd.height() / 2 + 50);
    lcd.drawString("Connect", lcd.width() / 2, lcd.height() / 2 + 100);
    delay(1000); 
  }
}
#endif
