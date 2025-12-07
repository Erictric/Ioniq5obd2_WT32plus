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
float send_enabled_float = 0.0;  // Float version for EEPROM storage (0.0 = disabled, 1.0 = enabled)
bool initscan = false;

void ConnectWifi(LGFX& lcd, uint16_t Wifi_select){
  
  char strConnectTime[3];
  //Serial.println("Connecting to Wifi "); 

  //Serial.print("Wifi_Select=  ");
  //Serial.println(Wifi_select);  
  
  //wifiMulti.addAP(ssid, password);
  //wifiMulti.addAP(ssid2, password2);
  
  //WiFi.disconnect();

  // Try to load custom WiFi credentials from EEPROM first
  String savedSSID = "";
  String savedPassword = "";
  extern bool loadWiFiCredentials(String &ssid, String &password);  // Forward declaration
  
  bool hasCustomWiFi = false;
  
  // Try to load custom credentials
  if (loadWiFiCredentials(savedSSID, savedPassword) && savedSSID.length() > 0) {
    // Use saved credentials
    Serial.printf("Using saved WiFi credentials: %s\n", savedSSID.c_str());
    WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
    hasCustomWiFi = true;
  }
  else {
    // Fall back to hardcoded credentials
    Serial.println("No valid saved credentials, using hardcoded WiFi");
    if (Wifi_select == 0){
      Serial.printf("Connecting to: %s\n", ssid);
      WiFi.begin(ssid, password);
    }
    else{
      Serial.printf("Connecting to: %s\n", ssid2);
      WiFi.begin(ssid2, password2);
    }
  }
    
  // WiFi connection without updating display (welcome screen handles display)
  while (WiFi.status() != WL_CONNECTED  && (connect_time < 15)) { // 2 attempts
    if(firstTry){
      connect_time = (millis() - init_time) / 1000;
    }    
    else{
      init_time = millis();
      firstTry = true;
    }
    delay(1000);     
  }

  if (WiFi.status() == WL_CONNECTED) {
    connect_time = (millis() - init_time) / 1000;
    Serial.print("WiFi connected in: "); 
    Serial.print(connect_time);
    Serial.println(" secs");
    Serial.print("IP address: "); 
    Serial.println(WiFi.localIP());
    
    firstTry = false;
    send_enabled = true;
    initscan = true;  // To write header name on Google Sheet on power up 
  }
  else
  {
    Serial.println("Failed to connect to WiFi");
  }
}
#endif
