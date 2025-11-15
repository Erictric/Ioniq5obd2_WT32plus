# 1 "C:\\Users\\emond\\AppData\\Local\\Temp\\tmp8y0q0t8n"
#include <Arduino.h>
# 1 "C:/Projects/Ioniq5obd2_WT32plus/src/Ioniq5obd2_WT32plus.ino"
# 19 "C:/Projects/Ioniq5obd2_WT32plus/src/Ioniq5obd2_WT32plus.ino"
#include "Arduino.h"
#include "LGFX_CLASS.h"
#include "BT_communication.h"
#include "WiFi.h"
#include "Wifi_connection.h"
#include "SafeString.h"
#include "EEPROM.h"
#include <Adafruit_GFX.h>
#include <ESP_Google_Sheet_Client.h>
#include <time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "ELMduino.h"

static LGFX lcd;
extern ELM327 myELM327;

#define DEBUG_PORT Serial


#define PROJECT_ID "ioniq5ev-datalogging"


#define CLIENT_EMAIL "ioniq5ev-datalogging@ioniq5ev-datalogging.iam.gserviceaccount.com"


const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\n"
                                   "MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQC1n1jxN7fUFbUV\n"
                                   "Zc3rn+ZgZ4oWv4v30HgTLgOIZr9DOvtDk/xkKK9UPlKgHX6s+KulIOu9cqlEDNz8\n"
                                   "Bh5Dsx9I9K7h6ax+BC6NUPkj5nU4pzl36lP9Mxot01Cck5GmP/7nGZwh05eGy+qf\n"
                                   "ZDvqGNJ2D1GzI84uznCYFT1b0zliINrWls8ylOTjrr/ECoq6a5htC2TD/2EWW7Oy\n"
                                   "xavn4bmkp7B05HtLz9kAXOfa8TX9WfzwAj8FUZiEvZWr87ei4Olj5T3rAaslCAVU\n"
                                   "Ftnu+11sxnRS6AOjwLhOrz63ubaT2ZKSPr8DEzt3nljmpp68aUJOYFX9Lmpa1YbT\n"
                                   "Rt3InN2xAgMBAAECggEAAeTyIcpY7vI1Nw54H4MsFs9sDVdjaTFVGBTrB9CgiCqh\n"
                                   "kixSBD5rY7rM4R6MCQq37/5s29QKFJ7Fhq7Op01pn/jTXFTAwV8ffpOJYbJtGkS3\n"
                                   "1y3lwRArIshSzZGsqEbMsxIQj5OPWUifsn7/MMggMjrv+UKzZOGvPKekdSR55pgL\n"
                                   "u7XzbPGEyTCT75BsWF+8RfBhSTbpNT0XYCzcSxMfOnDbjidtCEc7yjY4qIZm+Bih\n"
                                   "44b+j6v+w91f4IdgW/OQMY1GCKV5e7ApeqlR9o1MQVtKMzac20LtmAhKnXU39zgv\n"
                                   "cV/CNzGT+rCFESy9Fpdv4nm6QlQpagk0AFWV3pVNAQKBgQDcS+OhnXhK8XNBB9Qs\n"
                                   "OMbLkN9mtNY40l4Q81cewhTCd/xgr0oLUgXxeNocnpdO/mgV/GFp3egh7LU/AcSE\n"
                                   "ueE+Xd3Y7IjzrV6TG4M73bRkxUbfYnCIXq5cMqv0GGZvOg1KgbNwXVMO+eW/ddf3\n"
                                   "OoKPN9pj18RHURYeDqHJv+qQQQKBgQDTDt9R4AkiYQcScnK8z3mtBF/3Pr58pODP\n"
                                   "uIvBspzbn8NWlW3M32q260JGrWnXFF3coVDqU6PMhdD4MSiOQ0RpiKwTBhvJ0F4f\n"
                                   "6ZdycmdYb1dzMb1u/tGCrjqqH8C0CiNe1Obq5PikYrgI/fqK66xvEj/JQ/LRo1R4\n"
                                   "nWaNLsbxcQKBgQCnW4/nK7ZDWQLyGHx7y/ZamAjgAens6QRFZFh/KXqT8ots+D4M\n"
                                   "M5gIRoOM0n6oqGVyrnVi9A5yF13qK/Gb04rm0nDDZ47zcHY00+XzCQ8Or0CUXDiZ\n"
                                   "oTRdHrG7kv3e6f9G5xnm9z8uVXLQ7TnQvEaLfycOStD2TQe8dek7V+1fAQKBgDjc\n"
                                   "MN3l9ZAFg9o8axzi6GzsWM5LYRZDdS2BEmXEsO2aRQ32g/ZF2oIdL2XLIlCHdCIU\n"
                                   "c7AtiFt5UasL01lAVhX4dCNL6gCc2j7Ot7Zli+IPXQfzxo04qUkDl1pt44Sdlpnd\n"
                                   "0bhGp5Xh4qLJic4TYAksaXLXk3tW/VLhVNeEWqSBAoGADiHsTVFds00r41q75c1D\n"
                                   "EADUnkE3dPr31wRz9GPl7D0cTuf9NMB+I30CMKWI3wCMlKM5UmcS8/yYEt0N1B19\n"
                                   "LALKwZ4iEouf9KDIojOKyNv9e+cP4AHIKP+FOp8X5KyOgkHKNCfP86JykrQ8nL2L\n"
                                   "QlVT3ReZg1Cgf/Omx4GAtIs=\n"
                                   "-----END PRIVATE KEY-----\n";


const char spreadsheetId[] = "1DDPOXX3uooCHbO_UzsCNnbQAA7hsjsSV75VzBJDNk3o";


void tokenStatusCallback(TokenInfo info);

bool display_off = true;


uint16_t textLvl[10] = {65, 135, 205, 275, 345, 65, 135, 205, 275, 345};
uint16_t drawLvl[10] = {100, 170, 240, 310, 380, 100, 170, 240, 310, 380};

#define N_km 10

boolean ResetOn = true;
int screenNbr = 0;

uint8_t record_code = 0;
float mem_energy = 0;
float mem_PrevSoC = 0;
float mem_SoC = 0;
float mem_Power = 0;
float mem_LastSoC = 0;
float Wifi_select = 0;
bool data_ready = false;
bool code_sent = false;
bool sd_condition1 = false;
bool sd_condition2 = false;
bool SoC_saved = false;
bool code_received = false;
bool shutdown_esp = false;
bool wifiReconn = false;
bool datasent = false;
bool failsent = false;
uint16_t nbr_fails = 0;
uint16_t nbr_notReady = 0;
uint16_t nbr_saved = 0;

float BattMinT;
float BattMaxT;
float AuxBattV;
float BATTv;
float BATTc;
float MAXcellv;
float MINcellv;
int MAXcellvNb;
int MINcellvNb;
float CellVdiff;
float CEC;
float CED;
float CDC;
float CCC;
float BmsSoC;
float Max_Pwr;
float Max_Reg;
float SoC;
float SOH;
float PID_kWhLeft;
int MinDetNb;
int MaxDetNb;
float Heater;
float COOLtemp;
float OUTDOORtemp;
float INDOORtemp;
char SpdSelect;
unsigned long SpdSelectTimer = 0;
float Odometer;
float Speed;
float Motor1rpm;
float Motor2rpm;
byte TransSelByte;
byte Park;
byte Reverse;
byte Neutral;
byte Drive;
char selector[1];
byte StatusWord;
byte BMS_ign;
byte StatusWord2;
byte BMS_relay;
byte Charging;
float OPtimemins;
float OPtimehours;
float TireFL_P;
float TireFR_P;
float TireRL_P;
float TireRR_P;
float TireFL_T;
float TireFR_T;
float TireRL_T;
float TireRR_T;
float Power;
float CurrInitOdo = 0;
float CurrInitCEC = 0;
float CurrInitCED = 0;
float CurrTripOdo;
float CurrNet_kWh;
float CurrInitRemain;
float CurrInitAccEnergy;
float CurrAccEnergy;
float Prev_kWh = 0;
float Net_kWh = 0;
float Net_kWh2 = 0;
float UsedSoC = 0;
float Net_Ah = 0;
float DischAh = 0;
float RegenAh = 0;
float TripOdo = 0;
float InitOdo = 0;
float PrevOPtimemins = 0;
float TripOPtime = 0;
float CurrTimeInit = 0;
float CurrOPtime = 0;
float InitSoC = 0;
float InitCEC = 0;
float InitCED = 0;
float InitCCC = 0;
float InitCDC = 0;
float PrevSoC = 0;
float PrevBmsSoC = 0;
float Regen = 0;
float Discharg = 0;
float LastSoC = 0;
float integrateP_timer = 0.0;
float integrateI_timer = 0.0;
float start_kWh;
float acc_energy = 0.0;
float InitRemain_kWh = 0.0;
float delta_energy = 0.0;
float previous_kWh = 0.0;
float delta_kWh = 0.0;
float acc_regen;
float acc_Ah = 0.0;
float last_energy = 0.0;
float last_time = 0.0;
float last_odo = 0.0;
int energy_array_index = 0;
float energy_array[11];
float span_energy = 0.0;
float speed_interval = 0.0;
float init_speed_timer = 0.0;
float int_speed = 0.0;
float distance = 0.0;
float prev_dist = 0;
float interval_dist = 0;
float Trip_dist = 0;
float dist_save = 0;
float init_distsave = 0;
bool save_sent = false;
float prev_odo = 0;
float prev_power = 0.0;
float full_kWh;
float EstFull_Ah;
float kWh_corr;
float left_kWh;
float used_kWh;
float degrad_ratio;
float degrad_ratio_update = 1;
float degrad_ratio_change = 0;
bool Init_degrad = true;
float old_PIDkWh_100km = 14;
float old_lost = 1;
float kWh_100km;
float span_kWh_100km;
float PIDkWh_100;
float Est_range;
float Est_range2;
float Est_range3;
unsigned long RangeCalcTimer;
unsigned long RangeCalcUpdate = 2000;
float acc_kWh_25;
float acc_kWh_10;
float acc_kWh_0;
float acc_kWh_m10;
float acc_kWh_m20;
float acc_kWh_m20p;
float acc_time_25;
float acc_time_10;
float acc_time_0;
float acc_time_m10;
float acc_time_m20;
float acc_time_m20p;
float acc_dist_25;
float acc_dist_10;
float acc_dist_0;
float acc_dist_m10;
float acc_dist_m20;
float acc_dist_m20p;
bool DriveOn = false;
float StartWifi = 0;
bool InitRst = false;
bool TrigRst = false;
bool kWh_update = false;
bool SoC_decreased = false;
bool corr_update = false;
bool ESP_on = false;
bool DrawBackground = true;
char titre[10][13];
char value[10][7];
char prev_value[10][7];
bool negative_flag[10];
float value_float[10];
int nbr_decimal[10];
bool Charge_page = false;
bool Power_page = false;
unsigned long read_timer;
unsigned long read_data_interval = 2000;

float pwr_interval;
float int_pwr;
float curr_interval;
float int_curr;

float fullBattCapacity = 73.25;
float SoC100 = 100;
double b = 0.671;
double a;
float max_kwh;
float min_kwh;
float return_kwh;




static int xMargin = 20, yMargin = 420, margin = 20, btnWidth = 80, btnHeigth = 55;
const char* BtnAtext = "CONS";
const char* BtnBtext = "BATT";
const char* BtnCtext = "POWER";
char Maintitre[][13] = {"Consommation", "Batt. Info", "Energie", "Batt. Info 2"};
uint16_t MainTitleColor = TFT_WHITE;
uint16_t BtnOnColor = TFT_GREEN;
uint16_t BtnOffColor = TFT_LIGHTGREY;

unsigned long initTouchTime = 0;
unsigned long TouchTime = 0;
bool TouchLatch = false;
bool Btn1SetON = true;
bool Btn2SetON = false;
bool Btn3SetON = false;

struct RoundedRect {
  int xStart;
  int yStart;
  int xWidth;
  int yHeight;
  byte cornerRadius;
  uint16_t color;
  const char* BtnText;
};

RoundedRect btnAon = {
  xMargin,
  yMargin,
  btnWidth,
  btnHeigth,
  4,
  BtnOnColor,
  BtnAtext
};

RoundedRect btnBon = {
  btnAon.xStart + btnAon.xWidth + margin,
  yMargin,
  btnWidth,
  btnHeigth,
  4,
  BtnOnColor,
  BtnBtext
};

RoundedRect btnCon = {
  btnBon.xStart + btnBon.xWidth + margin,
  yMargin,
  btnWidth,
  btnHeigth,
  4,
  BtnOnColor,
  BtnCtext
};

RoundedRect btnAoff = {
  xMargin,
  yMargin,
  btnWidth,
  btnHeigth,
  4,
  BtnOffColor,
  BtnAtext
};

RoundedRect btnBoff = {
  btnAoff.xStart + btnAoff.xWidth + margin,
  yMargin,
  btnWidth,
  btnHeigth,
  4,
  BtnOffColor,
  BtnBtext
};

RoundedRect btnCoff = {
  btnBoff.xStart + btnBoff.xWidth + margin,
  yMargin,
  btnWidth,
  btnHeigth,
  4,
  BtnOffColor,
  BtnCtext
};


unsigned long ESPinitTimer = 0;
unsigned long ESPTimerInterval = 1200;
unsigned long shutdown_timer = 0;
unsigned long stopESP_timer = 0;



bool sending_data = false;
bool send_data = false;
bool data_sent = false;
bool ready = false;
bool success = false;
unsigned long sendInterval = 10000;
unsigned long GSheetTimer = 0;
bool sendIntervalOn = false;
tm timeinfo;
uint16_t nbrDays[13] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366};
char EventTime[18];
const char* EventCode0 = "New Trip";
const char* EventCode1 = "Recharge Reset";
const char* EventCode2 = "Button Reset";
const char* EventCode3 = "Reset on Acc Energy less 0.2";
const char* EventCode4 = "Reset 100 to 99%";
const char* EventCode5 = "Normal Shutdown";
const char* EventCode6 = "Timer Shutdown";
const char* Mess_SoC = "SoC:";
const char* Mess_Power = "Power:";
const char* Mess_LastSoC = "LastSoC:";
const char* Mess_PrevSoC = "PrevSoC:";
const char* Mess_Energy = "Energy:";
const char* Mess_SD = "Shutdown Timer:";


const char* ntpServer = "pool.ntp.org";


uint8_t pid_counter = 0;


struct dataFrames_struct {
  char frames[9][20];
};

typedef struct dataFrames_struct dataFrames;
dataFrames results;
void callback();
tm getTime();
void IRAM_ATTR Timer0_ISR();
void setup(void);
void clearResultFrames(dataFrames& results);
void processPayload(char* OBDdata, size_t datalen, dataFrames& results);
int convertToInt(char* dataFrame, size_t startByte, size_t numberBytes);
void read_data();
void UpdateNetEnergy();
void Integrat_power();
void Integrat_current();
void N_km_energy(float latest_energy);
void Integrat_speed();
void RangeCalc();
float calc_kwh(float min_SoC, float max_SoC);
void sendGoogleSheet(void * pvParameters);
void reset_trip();
void ResetCurrTrip();
void initial_eeprom();
void save_lost(char selector);
void stop_esp();
void button();
void drawRoundedRect(RoundedRect toDraw);
void DisplayPage();
void page1();
void page2();
void page3();
void page4();
void loop();
#line 431 "C:/Projects/Ioniq5obd2_WT32plus/src/Ioniq5obd2_WT32plus.ino"
void callback(){

}



tm getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return timeinfo;
  }
  time(&now);
# 457 "C:/Projects/Ioniq5obd2_WT32plus/src/Ioniq5obd2_WT32plus.ino"
  return timeinfo;
}

hw_timer_t *Timer0_Cfg = NULL;
int send_update = 10000000;

void IRAM_ATTR Timer0_ISR()
{
  if (ready) {
      sending_data = true;
    }
  else {
    sendIntervalOn = true;
  }
}


static int32_t x,y;

void setup(void)
{
  DEBUG_PORT.begin(9600);



  esp_log_level_set("BT_BTM", ESP_LOG_ERROR);





  DEBUG_PORT.println("Serial Monitor - STARTED");
  delay(1000);

  lcd.init();
  lcd.setRotation(2);
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_GREEN);
  lcd.setCursor(0, 0);
  lcd.setTextDatum(MC_DATUM);
  lcd.setTextSize(1);
  lcd.setFreeFont(&FreeSans12pt7b);


  EEPROM.begin(148);




  InitRemain_kWh = EEPROM.readFloat(0);
  InitCED = EEPROM.readFloat(4);
  InitCEC = EEPROM.readFloat(8);
  InitSoC = EEPROM.readFloat(12);
  previous_kWh = EEPROM.readFloat(16);

  InitOdo = EEPROM.readFloat(20);
  InitCDC = EEPROM.readFloat(24);
  InitCCC = EEPROM.readFloat(28);
  old_lost = EEPROM.readFloat(32);
  old_PIDkWh_100km = EEPROM.readFloat(36);
  Wifi_select = EEPROM.readFloat(40);
  PrevOPtimemins = EEPROM.readFloat(44);
  kWh_corr = EEPROM.readFloat(48);
  acc_energy = EEPROM.readFloat(52);
  LastSoC = EEPROM.readFloat(56);
  nbr_saved = EEPROM.readFloat(60);
  acc_Ah = EEPROM.readFloat(64);
  StartWifi = EEPROM.readFloat(68);
# 547 "C:/Projects/Ioniq5obd2_WT32plus/src/Ioniq5obd2_WT32plus.ino"
  SafeString::setOutput(Serial);




  ConnectToOBD2(lcd);





  if ((StartWifi == 1) && OBD2connected) {
    ConnectWifi(lcd, Wifi_select);


    configTime(0, 0, ntpServer);

    GSheet.printf("ESP Google Sheet Client v%s\n\n", ESP_GOOGLE_SHEET_CLIENT_VERSION);


    GSheet.setTokenCallback(tokenStatusCallback);


    GSheet.setPrerefreshSeconds(10 * 60);


    GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);
  }
# 587 "C:/Projects/Ioniq5obd2_WT32plus/src/Ioniq5obd2_WT32plus.ino"
  xTaskCreatePinnedToCore(
    sendGoogleSheet,
    "sendGoogleSheet",
    10000,
    NULL,
    0,
    NULL,
    0);
  delay(500);

  lcd.fillScreen(TFT_BLACK);


  Timer0_Cfg = timerBegin(0, 80, true);
  timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);
  timerAlarmWrite(Timer0_Cfg, send_update, true);
  timerAlarmEnable(Timer0_Cfg);
}
# 614 "C:/Projects/Ioniq5obd2_WT32plus/src/Ioniq5obd2_WT32plus.ino"
void clearResultFrames(dataFrames& results) {
  for (int i = 0; i < 9; i++) {
    results.frames[i][0] = '\0';
  }
}


void processPayload(char* OBDdata, size_t datalen, dataFrames& results) {
  cSFPS(data, OBDdata, datalen);
  clearResultFrames(results);
  size_t idx = data.indexOf(':');
  while (idx < data.length()) {
    int frameIdx = data[idx - 1] - '0';
    if ((frameIdx < 0) || (frameIdx > 8)) {


      idx = data.indexOf(':', idx + 1);
      continue;
    }
    cSFA(frame, results.frames[frameIdx]);
    idx++;
    size_t nextIdx = data.indexOf(':', idx);
    if (nextIdx == data.length()) {
      data.substring(frame, idx);
    } else {
      data.substring(frame, idx, nextIdx - 1);
    }

    idx = nextIdx;
  }
}
# 654 "C:/Projects/Ioniq5obd2_WT32plus/src/Ioniq5obd2_WT32plus.ino"
int convertToInt(char* dataFrame, size_t startByte, size_t numberBytes) {
  int offset = (startByte - 1) * 2;

  cSFP(frame, dataFrame);
  cSF(hexSubString, frame.capacity());
  frame.substring(hexSubString, offset, offset + (numberBytes * 2));
  hexSubString.debug(F(" hex number "));
  long num = 0;
  if (!hexSubString.hexToLong(num)) {
    hexSubString.debug(F(" invalid hex number "));
  }
  return num;
}
# 676 "C:/Projects/Ioniq5obd2_WT32plus/src/Ioniq5obd2_WT32plus.ino"
void read_data() {

  Serial.print("pid_counter: ");
  Serial.println(pid_counter);

  button();




  myELM327.sendCommand("AT SH 7E4");


  if (myELM327.queryPID("220101")) {

    char* payload = myELM327.payload;
    size_t payloadLen = myELM327.recBytes;
    Serial.print("payloadLen: ");
    Serial.println(payloadLen);

    processPayload(payload, payloadLen, results);

    int BattMinTraw = convertToInt(results.frames[2], 6, 1);
    if (BattMinTraw > 127) {
      BattMinT = -1 * (256 - BattMinTraw);
    } else {
      BattMinT = BattMinTraw;
    }
    int BattMaxTraw = convertToInt(results.frames[2], 5, 1);
    if (BattMaxTraw > 127) {
      BattMaxT = -1 * (256 - BattMaxTraw);
    } else {
      BattMaxT = BattMaxTraw;
    }
    AuxBattV = convertToInt(results.frames[4], 6, 1) * 0.1;
    BATTv = convertToInt(results.frames[2], 3, 2) * 0.1;
    int CurrentByte1 = convertToInt(results.frames[2], 1, 1);
    int CurrentByte2 = convertToInt(results.frames[2], 2, 1);
    if (CurrentByte1 > 127) {
      BATTc = -1 * (((255 - CurrentByte1) * 256) + (256 - CurrentByte2)) * 0.1;
    } else {
      BATTc = ((CurrentByte1 * 256) + CurrentByte2) * 0.1;
    }
    CEC = convertToInt(results.frames[6], 1, 4) * 0.1;
    CED = ((convertToInt(results.frames[6], 5, 3) << 8) + convertToInt(results.frames[7], 1, 1)) * 0.1;
    CCC = ((convertToInt(results.frames[4], 7, 1) << 24) + convertToInt(results.frames[5], 1, 3)) * 0.1;
    CDC = convertToInt(results.frames[5], 4, 4) * 0.1;
    BmsSoC = convertToInt(results.frames[1], 2, 1) * 0.5;
    StatusWord = convertToInt(results.frames[7], 6, 1);
    BMS_ign = bitRead(StatusWord, 2);
    StatusWord2 = convertToInt(results.frames[1], 7, 1);
    BMS_relay = bitRead(StatusWord2, 0);
    MAXcellv = convertToInt(results.frames[3], 7, 1) * 0.02;
    MAXcellvNb = convertToInt(results.frames[4], 1, 1);
    MINcellv = convertToInt(results.frames[4], 2, 1) * 0.02;
    MINcellvNb = convertToInt(results.frames[4], 3, 1);
    OPtimemins = convertToInt(results.frames[7], 2, 4) * 0.01666666667;
    OPtimehours = OPtimemins * 0.01666666667;
    Motor1rpm = convertToInt(results.frames[8], 2, 2);
    Motor2rpm = convertToInt(results.frames[8], 4, 2);
  }
  if (BATTc > 0) {
    ESP_on = true;
    BMS_ign = true;
  }
  if (BATTc < 0) {
    Charging = true;
  }
  if (Speed > 0) {
    SpdSelect = 'D';
  }
  else {
    SpdSelect = 'P';
  }
  UpdateNetEnergy();

  if (pid_counter > 5){
    pid_counter = 0;
  }
  else if (BMS_ign || Charging){

    switch (pid_counter) {
      case 1:

        button();
        myELM327.sendCommand("AT SH 7E4");

        if (myELM327.queryPID("220105")) {

          char* payload = myELM327.payload;
          size_t payloadLen = myELM327.recBytes;

          processPayload(payload, payloadLen, results);
          Max_Pwr = convertToInt(results.frames[3], 2, 2) * 0.01;
          Max_Reg = (((convertToInt(results.frames[2], 7, 1)) << 8) + convertToInt(results.frames[3], 1, 1)) * 0.01;
          SoC = convertToInt(results.frames[5], 1, 1) * 0.5;
          SOH = convertToInt(results.frames[4], 2, 2) * 0.1;
          MaxDetNb = convertToInt(results.frames[4], 4, 1);
          MinDetNb = convertToInt(results.frames[4], 7, 1);
          PID_kWhLeft = convertToInt(results.frames[4], 5, 2) * 0.002;
          int HeaterRaw = convertToInt(results.frames[3], 7, 1);
          if (HeaterRaw > 127) {
            Heater = -1 * (256 - HeaterRaw);
          } else {
            Heater = HeaterRaw;
          }
        }
        break;

      case 2:

        button();
        myELM327.sendCommand("AT SH 7E4");

        if (myELM327.queryPID("220106")) {

          char* payload = myELM327.payload;
          size_t payloadLen = myELM327.recBytes;

          processPayload(payload, payloadLen, results);
          int COOLtempRaw = convertToInt(results.frames[1], 2, 1) * 0.01;
          if (COOLtempRaw > 127) {
            COOLtemp = -1 * (256 - COOLtempRaw);
          } else {
            COOLtemp = COOLtempRaw;
          }
        }
        break;

      case 3:

        button();
        myELM327.sendCommand("AT SH 7C6");

        if (myELM327.queryPID("22B002")) {

          char* payload = myELM327.payload;
          size_t payloadLen = myELM327.recBytes;

          processPayload(payload, payloadLen, results);
          Odometer = convertToInt(results.frames[1], 4, 3);
        }
        break;

      case 4:

        button();
        myELM327.sendCommand("AT SH 7B3");

        if (myELM327.queryPID("220100")) {

          char* payload = myELM327.payload;
          size_t payloadLen = myELM327.recBytes;

          processPayload(payload, payloadLen, results);
          INDOORtemp = (((convertToInt(results.frames[1], 3, 1)) * 0.5) - 40);
          OUTDOORtemp = (((convertToInt(results.frames[1], 4, 1)) * 0.5) - 40);
          Speed = (convertToInt(results.frames[4], 6, 1));
          Integrat_speed();
        }
        break;

      case 5:

        button();
        myELM327.sendCommand("AT SH 7A0");

        if (myELM327.queryPID("22C00B")) {

          char* payload = myELM327.payload;
          size_t payloadLen = myELM327.recBytes;

          processPayload(payload, payloadLen, results);
          TireFL_P = convertToInt(results.frames[1], 2, 1) * 0.2;
          TireFL_T = convertToInt(results.frames[1], 3, 1) - 50;
          TireFR_P = convertToInt(results.frames[1], 7, 1) * 0.2;
          TireFR_T = convertToInt(results.frames[2], 1, 1) - 50;
          TireRL_P = convertToInt(results.frames[2], 5, 1) * 0.2;
          TireRL_T = convertToInt(results.frames[2], 6, 1) - 50;
          TireRR_P = convertToInt(results.frames[3], 3, 1) * 0.2;
          TireRR_T = convertToInt(results.frames[3], 4, 1) - 50;
        }
        pid_counter = 0;
        data_ready = true;
        break;
    }




  Power = (BATTv * BATTc) * 0.001;
  Integrat_power();
  Integrat_current();

    if (!ResetOn) {
      TripOdo = Odometer - InitOdo;

      CurrTripOdo = Odometer - CurrInitOdo;

      CurrOPtime = OPtimemins - CurrTimeInit;

      TripOPtime = CurrOPtime + PrevOPtimemins;

      UsedSoC = InitSoC - SoC;

      if (UsedSoC < 0.5){
        EstFull_Ah = 111,2;
      }
      else{
        EstFull_Ah = 100 * Net_Ah / UsedSoC;
      }

      CellVdiff = MAXcellv - MINcellv;

      if (PrevSoC != SoC) {
        if (InitRst) {
          Serial.print("1st Reset");
          initscan = true;
          record_code = 2;
          reset_trip();
          kWh_corr = 0;
          PrevSoC = SoC;
          Prev_kWh = Net_kWh;
          used_kWh = calc_kwh(SoC, InitSoC);
          left_kWh = calc_kwh(0, SoC) * degrad_ratio;
          InitRst = false;
        }
        else {

          if (((acc_energy < 0.25) && (PrevSoC > SoC)) || ((SoC > 98.5) && ((PrevSoC - SoC) > 0.5))) {
            if ((acc_energy < 0.25) && (PrevSoC > SoC)) {
              initscan = true;
              mem_energy = acc_energy;
              mem_PrevSoC = PrevSoC;
              mem_SoC = SoC;
              record_code = 3;
              Serial.print("2nd Reset");
              reset_trip();
              kWh_corr = 0;
              used_kWh = calc_kwh(SoC, InitSoC);
              left_kWh = calc_kwh(0, SoC) * degrad_ratio;
              PrevSoC = SoC;
              Prev_kWh = Net_kWh;
              kWh_update = true;
              SoC_decreased = true;
            }
            else {
              record_code = 4;
            }

          }
          else if (((PrevSoC > SoC) && ((PrevSoC - SoC) < 1)) || ((PrevSoC < SoC) && (Charging))) {
            kWh_corr = 0;
            used_kWh = calc_kwh(SoC, InitSoC);
            left_kWh = calc_kwh(0, SoC) * degrad_ratio;
            SoC_decreased = true;
            PrevSoC = SoC;
            Prev_kWh = Net_kWh;
            kWh_update = true;
            Integrat_power();

            if ((used_kWh >= 3) && (SpdSelect == 'D')) {
              degrad_ratio = Net_kWh2 / used_kWh;

              if (degrad_ratio > 1.01) {
                degrad_ratio = 1.01;
              }
              else if (degrad_ratio < 0.9){
                degrad_ratio = 0.9;
              }
              old_lost = degrad_ratio;
            }
            else {
              degrad_ratio = old_lost;
              if (degrad_ratio > 1.01) {
                degrad_ratio = 1.01;
              }
              else if (degrad_ratio < 0.9){
                degrad_ratio = 0.9;
              }
            }
            start_kWh = calc_kwh(InitSoC, 100) * degrad_ratio;

            full_kWh = Net_kWh2 + (start_kWh + left_kWh);

          }
        }

      }
      else if ((Prev_kWh < Net_kWh) && !kWh_update) {
        kWh_corr += (Net_kWh - Prev_kWh);
        used_kWh = calc_kwh(PrevSoC, InitSoC) + kWh_corr;
        left_kWh = (calc_kwh(0, PrevSoC) * degrad_ratio) - kWh_corr;
        Prev_kWh = Net_kWh;
        corr_update = true;
      }
# 980 "C:/Projects/Ioniq5obd2_WT32plus/src/Ioniq5obd2_WT32plus.ino"
      if (sendIntervalOn) {
        if (kWh_update) {
          Prev_kWh = Net_kWh;
          kWh_update = false;
        }
        if (corr_update) {
          corr_update = false;
        }
        sendIntervalOn = false;
      }

      if ((LastSoC + 1) < SoC && (Power > 0) && (LastSoC != 0)) {
        mem_Power = Power;
        mem_LastSoC = LastSoC;
        mem_SoC = SoC;
        initscan = true;
        record_code = 1;
        reset_trip();
      }



      if ((millis() - RangeCalcTimer) > RangeCalcUpdate){
        RangeCalc();
        RangeCalcTimer = millis();
      }





      if (Max_Pwr < 100 && (Max_Pwr < (Power + 20)) && !Power_page) {
        DrawBackground = true;
        screenNbr = 2;
        Power_page = true;
        if (Btn1SetON){
          Btn1SetON = false;
        }
        if (Btn2SetON){
          Btn2SetON = false;
        }
      }
      if (Power < 0 && (SpdSelect == 'P') && !Charge_page) {
        DrawBackground = true;
        screenNbr = 2;
        Charge_page = true;
        if (Btn1SetON){
          Btn1SetON = false;
        }
        if (Btn2SetON){
          Btn2SetON = false;
        }
      }
    }

    save_lost(SpdSelect);
  }
}







void UpdateNetEnergy() {

  if (InitCED == 0) {
    InitCED = CED;
    InitSoC = SoC;
    CurrInitCED = CED;
  }
  if (InitCDC == 0) {
    InitCDC = CDC;
  }
  if (InitCEC == 0) {
    InitCEC = CEC;
    CurrInitCEC = CEC;
  }
  if (InitCCC == 0) {
    InitCCC = CCC;
  }

  Discharg = CED - InitCED;
  Regen = CEC - InitCEC;
  Net_kWh = Discharg - Regen;

  Net_kWh2 = InitRemain_kWh - PID_kWhLeft;

  DischAh = CDC - InitCDC;
  RegenAh = CCC - InitCCC;
  Net_Ah = DischAh - RegenAh;

  CurrAccEnergy = acc_energy - CurrInitAccEnergy;

  CurrNet_kWh = CurrInitRemain - PID_kWhLeft;
}







void Integrat_power() {

  pwr_interval = (millis() - integrateP_timer) / 1000;
  integrateP_timer = millis();
  int_pwr = Power * pwr_interval / 3600;
  acc_energy += int_pwr;
  if (int_pwr < 0) {
    acc_regen += -(int_pwr);
  }
}







void Integrat_current() {

  curr_interval = (millis() - integrateI_timer) / 1000;
  integrateI_timer = millis();
  int_curr = BATTc * curr_interval / 3600;
  acc_Ah += int_curr;
}







void N_km_energy(float latest_energy) {
  energy_array[energy_array_index] = latest_energy;
  energy_array_index++;
  if (energy_array_index > N_km) {
    energy_array_index = 0;
  }
  span_energy = latest_energy - energy_array[energy_array_index];
}







void Integrat_speed() {
  speed_interval = (millis() - init_speed_timer) / 1000;
  init_speed_timer = millis();
  int_speed = Speed * speed_interval / 3600;
  distance += (int_speed * 1);
}





void RangeCalc() {

  if ((prev_odo != CurrTripOdo) && (distance < 0.9)) {
    if (TripOdo < 2) {
      InitOdo = Odometer - distance;
      TripOdo = Odometer - InitOdo;
    }
    CurrInitOdo = Odometer - distance;
    CurrTripOdo = Odometer - CurrInitOdo;
    prev_dist = distance;
    prev_odo = CurrTripOdo;
    N_km_energy(acc_energy);
  }
  else if (prev_odo != CurrTripOdo) {
    prev_dist = distance;
    prev_odo = CurrTripOdo;
    N_km_energy(acc_energy);
  }
  interval_dist = distance - prev_dist;
  Trip_dist = CurrTripOdo + interval_dist;
  dist_save = Trip_dist - init_distsave;
  save_sent = false;

  if (Trip_dist >= 0.25 && !ResetOn) {
    kWh_100km = CurrAccEnergy * 100 / Trip_dist;
    PIDkWh_100 = CurrNet_kWh * 100 / Trip_dist;
  }
  else {
    kWh_100km = old_PIDkWh_100km;
    PIDkWh_100 = old_PIDkWh_100km;
  }
  if (kWh_100km < 5 || kWh_100km > 999 || PIDkWh_100 < 5 || PIDkWh_100 > 999){
    kWh_100km = 18;
    PIDkWh_100 = 18;
  }

  if (Trip_dist >= (N_km + 1)) {
    span_kWh_100km = span_energy * 100 / N_km;
  }
  else {
    span_kWh_100km = kWh_100km;
  }

  if (kWh_100km > 1) {
    Est_range = (PID_kWhLeft / kWh_100km) * 100;
    Est_range2 = (PID_kWhLeft / span_kWh_100km) * 100;
    Est_range3 = (PID_kWhLeft / PIDkWh_100) * 100;
    if (Est_range3 < 0){
      Est_range3 = 999;
    }
  }
  else {
    Est_range = 999;
    Est_range2 = 999;
    Est_range3 = 999;
  }
}
# 1275 "C:/Projects/Ioniq5obd2_WT32plus/src/Ioniq5obd2_WT32plus.ino"
float calc_kwh(float min_SoC, float max_SoC) {

  a = (fullBattCapacity - (b * SoC100)) / pow(SoC100,2);

  max_kwh = a * pow(max_SoC,2) + b * max_SoC;
  min_kwh = a * pow(min_SoC,2) + b * min_SoC;

  return_kwh = max_kwh - min_kwh;
  return return_kwh;
}


void tokenStatusCallback(TokenInfo info){
    if (info.status == token_status_error){
        GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
        GSheet.printf("Token error: %s\n", GSheet.getTokenError(info).c_str());
    }
    else{
        GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
    }
}





void sendGoogleSheet(void * pvParameters){
  for(;;){
    if (ready && (send_data || record_code != 0)) {

      code_sent = false;

      FirebaseJson response;

      Serial.println("\nAppend spreadsheet values...");
      Serial.println("----------------------------");

      FirebaseJson valueRange;

      if(initscan || record_code != 0 || shutdown_esp){

        switch (record_code)
        {
        case 0:
          valueRange.add("majorDimension","COLUMNS");
          valueRange.set("values/[0]/[0]", EventCode0);
          initscan = false;
          shutdown_esp = false;
          break;

        case 1:
          valueRange.add("majorDimension","COLUMNS");
          valueRange.set("values/[0]/[0]", EventCode1);
          valueRange.set("values/[1]/[0]", Mess_SoC);
          valueRange.set("values/[2]/[0]", mem_SoC);
          valueRange.set("values/[3]/[0]", Mess_LastSoC);
          valueRange.set("values/[4]/[0]", mem_LastSoC);
          initscan = true;
          break;

        case 2:
          valueRange.add("majorDimension","COLUMNS");
          valueRange.set("values/[0]/[0]", EventCode2);
          initscan = true;
          break;

        case 3:
          valueRange.add("majorDimension","COLUMNS");
          valueRange.set("values/[0]/[0]", EventCode3);
          valueRange.set("values/[1]/[0]", Mess_SoC);
          valueRange.set("values/[2]/[0]", mem_SoC);
          valueRange.set("values/[3]/[0]", Mess_PrevSoC);
          valueRange.set("values/[4]/[0]", mem_PrevSoC);
          valueRange.set("values/[5]/[0]", Mess_Energy);
          valueRange.set("values/[6]/[0]", mem_energy);
          initscan = true;
          break;

        case 4:
          valueRange.add("majorDimension","COLUMNS");
          valueRange.set("values/[0]/[0]", EventCode4);
          valueRange.set("values/[1]/[0]", Mess_SoC);
          valueRange.set("values/[2]/[0]", mem_SoC);
          valueRange.set("values/[3]/[0]", Mess_PrevSoC);
          valueRange.set("values/[4]/[0]", mem_PrevSoC);
          initscan = true;
          break;

        case 5:
          valueRange.add("majorDimension","COLUMNS");
          valueRange.set("values/[0]/[0]", EventCode5);
          valueRange.set("values/[1]/[0]", Mess_SoC);
          valueRange.set("values/[2]/[0]", mem_SoC);
          valueRange.set("values/[3]/[0]", Mess_Power);
          valueRange.set("values/[4]/[0]", Power);

          code_received = true;
          Serial.println("Code Received");
          break;

        case 6:
          valueRange.add("majorDimension","COLUMNS");
          valueRange.set("values/[0]/[0]", EventCode6);
          valueRange.set("values/[1]/[0]", Mess_Power);
          valueRange.set("values/[2]/[0]", Power);
          valueRange.set("values/[3]/[0]", Mess_SD);
          valueRange.set("values/[4]/[0]", shutdown_timer);
          code_received = true;
          break;

        }
      }
      else{
        valueRange.add("majorDimension","COLUMNS");
        valueRange.set("values/[0]/[0]", EventTime);
        valueRange.set("values/[1]/[0]", SoC);
        valueRange.set("values/[2]/[0]", BmsSoC);
        valueRange.set("values/[3]/[0]", Power);
        valueRange.set("values/[4]/[0]", TripOdo);
        valueRange.set("values/[5]/[0]", BattMinT);
        valueRange.set("values/[6]/[0]", BattMaxT);
        valueRange.set("values/[7]/[0]", Heater);
        valueRange.set("values/[8]/[0]", OUTDOORtemp);
        valueRange.set("values/[9]/[0]", INDOORtemp);
        valueRange.set("values/[10]/[0]", Net_kWh);
        valueRange.set("values/[11]/[0]", Net_kWh2);
        valueRange.set("values/[12]/[0]", acc_energy);
        valueRange.set("values/[13]/[0]", Net_Ah);
        valueRange.set("values/[14]/[0]", acc_Ah);
        valueRange.set("values/[15]/[0]", EstFull_Ah);
        valueRange.set("values/[16]/[0]", Max_Pwr);
        valueRange.set("values/[17]/[0]", Max_Reg);
        valueRange.set("values/[18]/[0]", MAXcellv);
        valueRange.set("values/[19]/[0]", MINcellv);
        valueRange.set("values/[20]/[0]", MAXcellvNb);
        valueRange.set("values/[21]/[0]", MINcellvNb);
        valueRange.set("values/[22]/[0]", BATTv);
        valueRange.set("values/[23]/[0]", BATTc);
        valueRange.set("values/[24]/[0]", AuxBattV);
        valueRange.set("values/[25]/[0]", CEC);
        valueRange.set("values/[26]/[0]", CED);
        valueRange.set("values/[27]/[0]", CDC);
        valueRange.set("values/[28]/[0]", CCC);
        valueRange.set("values/[29]/[0]", SOH);
        valueRange.set("values/[30]/[0]", used_kWh);
        valueRange.set("values/[31]/[0]", left_kWh);
        valueRange.set("values/[32]/[0]", full_kWh);
        valueRange.set("values/[33]/[0]", start_kWh);
        valueRange.set("values/[34]/[0]", PID_kWhLeft);
        valueRange.set("values/[35]/[0]", degrad_ratio);
        valueRange.set("values/[36]/[0]", InitSoC);
        valueRange.set("values/[37]/[0]", LastSoC);
        valueRange.set("values/[38]/[0]", PIDkWh_100);
        valueRange.set("values/[39]/[0]", kWh_100km);
        valueRange.set("values/[40]/[0]", span_kWh_100km);
        valueRange.set("values/[41]/[0]", Trip_dist);
        valueRange.set("values/[42]/[0]", distance);
        valueRange.set("values/[43]/[0]", Speed);
        valueRange.set("values/[44]/[0]", Odometer);
        valueRange.set("values/[45]/[0]", OPtimemins);
        valueRange.set("values/[46]/[0]", TripOPtime);
        valueRange.set("values/[47]/[0]", CurrOPtime);
        valueRange.set("values/[48]/[0]", TireFL_P);
        valueRange.set("values/[49]/[0]", TireFR_P);
        valueRange.set("values/[50]/[0]", TireRL_P);
        valueRange.set("values/[51]/[0]", TireRR_P);
        valueRange.set("values/[52]/[0]", TireFL_T);
        valueRange.set("values/[53]/[0]", TireFR_T);
        valueRange.set("values/[54]/[0]", TireRL_T);
        valueRange.set("values/[55]/[0]", TireRR_T);
        valueRange.set("values/[56]/[0]", nbr_saved);

      }


      if (send_data || record_code != 0){
        success = GSheet.values.append(&response , spreadsheetId , "Sheet1!A1" , &valueRange );
        send_data = false;
      }

      record_code = 0;
      vTaskDelay(10);
      if (success){
        response.toString(Serial, true);
        valueRange.clear();
        datasent = true;
      }
      else{
        Serial.print("GSheet sent error: ");
        Serial.println(GSheet.errorReason());
        valueRange.clear();
        failsent = true;
        nbr_fails += 1;
      }
      GSheetTimer = millis();
      Serial.println();
      Serial.print("FreeHeap: ");
      Serial.println(ESP.getFreeHeap());

      if(kWh_update){
        Prev_kWh = Net_kWh;
        kWh_update = false;
      }
      if(corr_update){
        corr_update = false;
      }
      if (code_received){
        Serial.println("Eventcode Sent");
        code_sent = true;
      }

    }
    vTaskDelay(10);
  }
}



void reset_trip() {

  Serial.println("saving");
  InitOdo = Odometer;
  InitCED = CED;
  InitSoC = SoC;
  InitCEC = CEC;
  InitCDC = CDC;
  InitCCC = CCC;
  Net_kWh = 0;
  acc_energy = 0;
  InitRemain_kWh = PID_kWhLeft;
  previous_kWh = 0;
  acc_Ah = 0;
  UsedSoC = 0;
  kWh_corr = 0;
  Discharg = 0;
  Regen = 0;
  Net_Ah = 0;
  DischAh = 0;
  RegenAh = 0;
  PrevOPtimemins = 0;
  LastSoC = SoC;
  PrevBmsSoC = BmsSoC;
  CurrInitCED = CED;
  CurrInitCEC = CEC;
  CurrInitOdo = Odometer;
  CurrInitRemain = PID_kWhLeft;
  CurrTimeInit = OPtimemins;
  integrateP_timer = millis();
  integrateI_timer = millis();
  distance = 0;
  CurrInitAccEnergy = 0;

  last_energy = acc_energy;
  start_kWh = calc_kwh(InitSoC, 100) * degrad_ratio;
  left_kWh = calc_kwh(0, SoC) * degrad_ratio;
  full_kWh = Net_kWh2 + (start_kWh + left_kWh);

  EEPROM.writeFloat(52, acc_energy);
  EEPROM.writeFloat(0, InitRemain_kWh);
  EEPROM.writeFloat(4, InitCED);
  EEPROM.writeFloat(8, InitCEC);
  EEPROM.writeFloat(12, InitSoC);
  EEPROM.writeFloat(16, previous_kWh);
  EEPROM.writeFloat(20, InitOdo);
  EEPROM.writeFloat(24, InitCDC);
  EEPROM.writeFloat(28, InitCCC);

  EEPROM.commit();
  Serial.println("value saved to EEPROM");
}



void ResetCurrTrip() {

  if (ResetOn && (SoC > 1) && (Odometer > 1) && (CED > 1) && data_ready) {
    integrateP_timer = millis();
    integrateI_timer = millis();
    RangeCalcTimer = millis();
    read_timer = millis();
    GSheetTimer = millis();
    CurrInitAccEnergy = acc_energy;
    CurrInitCED = CED;
    CurrInitCEC = CEC;
    CurrInitOdo = Odometer;

    CurrInitRemain = PID_kWhLeft;
    CurrTimeInit = OPtimemins;
    Serial.println("Trip Reset");
    Prev_kWh = Net_kWh;
    last_energy = acc_energy;

    degrad_ratio = old_lost;
    if (degrad_ratio > 1.01) {
      degrad_ratio = 1.01;
    }
    else if (degrad_ratio < 0.9){
      degrad_ratio = 0.9;
    }
    used_kWh = calc_kwh(SoC, InitSoC);
    left_kWh = (calc_kwh(0, SoC) * degrad_ratio);
    start_kWh = calc_kwh(InitSoC, 100) * degrad_ratio;
    full_kWh = Net_kWh2 + (start_kWh + left_kWh);

    PrevSoC = SoC;
    PrevBmsSoC = BmsSoC;
    ResetOn = false;
    for (uint8_t i = 0; i < N_km; i++) {
      energy_array[i] = acc_energy;
    }
  }
}

void initial_eeprom() {

  for (int i = 0; i < 148; i += 4) {
    if (isnan(EEPROM.readFloat(i))) {
      EEPROM.writeFloat(i, 0);
    }
  }
  EEPROM.commit();
}



void save_lost(char selector) {
  if (selector == 'D' && !DriveOn) {
    DriveOn = true;
    SpdSelectTimer = millis();
  }
  if ((selector == 'P' || selector == 'N') && (DriveOn) && SoC > 0 && ((millis() - SpdSelectTimer) > 90000)) {
    DriveOn = false;

    nbr_saved += 1;

    EEPROM.writeFloat(16, previous_kWh);
    EEPROM.writeFloat(32, degrad_ratio);
    Serial.println("new_lost saved to EEPROM");
    EEPROM.writeFloat(36, PIDkWh_100);
    EEPROM.writeFloat(40, Wifi_select);
    EEPROM.writeFloat(44, TripOPtime);
    EEPROM.writeFloat(48, kWh_corr);
    EEPROM.writeFloat(52, acc_energy);
    EEPROM.writeFloat(56, SoC);
    EEPROM.writeFloat(60, nbr_saved);
    EEPROM.writeFloat(64, acc_Ah);
# 1639 "C:/Projects/Ioniq5obd2_WT32plus/src/Ioniq5obd2_WT32plus.ino"
    EEPROM.writeFloat(140, acc_regen);
    EEPROM.commit();
  }
}

void stop_esp() {
  ESP_on = false;
  if (DriveOn && (mem_SoC > 0)) {

    EEPROM.writeFloat(16, previous_kWh);
    EEPROM.writeFloat(32, degrad_ratio);
    Serial.println("new_lost saved to EEPROM");
    EEPROM.writeFloat(36, PIDkWh_100);
    EEPROM.writeFloat(40, Wifi_select);
    EEPROM.writeFloat(44, TripOPtime);
    EEPROM.writeFloat(48, kWh_corr);
    EEPROM.writeFloat(52, acc_energy);
    EEPROM.writeFloat(56, mem_SoC);

    EEPROM.writeFloat(64, acc_Ah);
    EEPROM.writeFloat(68, acc_kWh_25);
    EEPROM.writeFloat(72, acc_kWh_10);
    EEPROM.writeFloat(76, acc_kWh_0);
    EEPROM.writeFloat(80, acc_kWh_m10);
    EEPROM.writeFloat(84, acc_kWh_m20);
    EEPROM.writeFloat(88, acc_kWh_m20p);
    EEPROM.writeFloat(92, acc_time_25);
    EEPROM.writeFloat(96, acc_time_10);
    EEPROM.writeFloat(100, acc_time_0);
    EEPROM.writeFloat(104, acc_time_m10);
    EEPROM.writeFloat(108, acc_time_m20);
    EEPROM.writeFloat(112, acc_time_m20p);
    EEPROM.writeFloat(116, acc_dist_25);
    EEPROM.writeFloat(120, acc_dist_10);
    EEPROM.writeFloat(124, acc_dist_0);
    EEPROM.writeFloat(128, acc_dist_m10);
    EEPROM.writeFloat(132, acc_dist_m20);
    EEPROM.writeFloat(136, acc_dist_m20p);
    EEPROM.writeFloat(140, acc_regen);
    EEPROM.commit();
  }

  if (sd_condition2){
    lcd.setTextSize(1);
    lcd.setFreeFont(&FreeSans12pt7b);
    lcd.fillScreen(TFT_BLACK);
    lcd.setTextColor(TFT_GREEN);
    lcd.drawString("ESP", lcd.width() / 2, lcd.height() / 2 - 50);
    lcd.drawString("Stopped", lcd.width() / 2, lcd.height() / 2);
    delay(1000);
    esp_deep_sleep_start();
  }
  else{
    lcd.setTextSize(1);
    lcd.setFreeFont(&FreeSans12pt7b);
    lcd.fillScreen(TFT_BLACK);
    lcd.setTextColor(TFT_GREEN);
    if (WiFi.status() == WL_CONNECTED){
      lcd.drawString("Wifi", lcd.width() / 2, lcd.height() / 2 - 50);
      lcd.drawString("Stopped", lcd.width() / 2, lcd.height() / 2);
      WiFi.disconnect();
      Serial.println("Wifi Stopped");
    }
    else{
      lcd.drawString("Going", lcd.width() / 2, lcd.height() / 2 - 50);
      lcd.drawString("Stand", lcd.width() / 2, lcd.height() / 2);
      lcd.drawString("By", lcd.width() / 2, lcd.height() / 2 + 50);
    }

    delay(1000);

    shutdown_esp = false;
    send_enabled = false;
    wifiReconn = false;
    DrawBackground = true;
  }
}





void button(){
  if (lcd.getTouch(&x, &y) && !OBD2connected) {
    lcd.setBrightness(128);
    ConnectToOBD2(lcd);
  }
  else if (lcd.getTouch(&x, &y)) {


    if ((x >= btnAon.xStart && x <= btnAon.xStart + btnAon.xWidth) && (y >= btnAon.yStart && y <= btnAon.yStart + btnAon.yHeight)) {
      TouchTime = (millis() - initTouchTime) / 1000;
      if (TouchTime >= 2 & !TouchLatch){
        Serial.println("Button1 Long Press");
        TouchLatch = true;
        if (StartWifi == 1){
          StartWifi = 0;
          EEPROM.writeFloat(68, StartWifi);
          EEPROM.commit();
          send_enabled = false;
        }
        else{
          StartWifi = 1;
          EEPROM.writeFloat(68, StartWifi);
          EEPROM.commit();
          save_lost('P');
          ESP.restart();
        }
      }
      if (!Btn1SetON)
      {
          screenNbr = 0;
          Serial.println("Button1 Touched");
          Serial.println("Button1 set to ON");
          DrawBackground = true;
          Btn1SetON = true;
        if (Btn2SetON){
          Btn2SetON = false;
        }
        if (Btn3SetON){
          Btn3SetON = false;
        }
      }
    }


    if ((x >= btnBon.xStart && x <= btnBon.xStart + btnBon.xWidth) && (y >= btnBon.yStart && y <= btnBon.yStart + btnBon.yHeight)) {
      TouchTime = (millis() - initTouchTime) / 1000;
      if (TouchTime >= 2 & !TouchLatch){
        TouchLatch = true;
        Serial.println("Button2 Long Press");
        if (Wifi_select == 0){

          Wifi_select = 1;
        }
        else{

          Wifi_select = 0;
        }

        Serial.println("DONE");
        Btn2SetON = true;
      }
      if (!Btn2SetON){
        screenNbr = 1;
        Serial.println("Button2 Touched");
        Serial.println("Button2 set to ON");
        DrawBackground = true;
        Btn2SetON = true;
        if (Btn1SetON){
          Btn1SetON = false;
        }
        if (Btn3SetON){
          Btn3SetON = false;
        }
      }
    }


    if ((x >= btnCon.xStart && x <= btnCon.xStart + btnCon.xWidth) && (y >= btnCon.yStart && y <= btnCon.yStart + btnCon.yHeight)) {
      TouchTime = (millis() - initTouchTime) / 1000;
      if (TouchTime >= 2 & !TouchLatch){
        TouchLatch = true;
        Serial.println("Button3 Long Press");

        display_off = true;
      }
      if (!Btn3SetON) {
        screenNbr = 2;
        Serial.println("Button3 Touched");
        Serial.println("Button3 set to ON");
        DrawBackground = true;
        Btn3SetON = true;
        if (Btn1SetON){
          Btn1SetON = false;
        }
        if (Btn2SetON){
          Btn2SetON = false;
        }
      }
    }


    if (x >= 0 && x <= 320 && y >= 65 && y <= 400 && (screenNbr == 1 || screenNbr == 3)) {
      if (!TouchLatch && screenNbr == 1) {
        screenNbr = 3;
        Serial.println("Screen Touched");
        TouchLatch = true;
        DrawBackground = true;
      }
      else if (!TouchLatch && screenNbr == 3)
      {
        screenNbr = 1;
        Serial.println("Screen Touched");
        TouchLatch = true;
        DrawBackground = true;
      }
    }


    if (x >= 0 && x <= 320 && y >= 65 && y <= 400 && (screenNbr == 0 || screenNbr == 2)) {
      if (!TouchLatch && screenNbr == 0) {
        screenNbr = 2;
        Serial.println("Screen Touched");
        TouchLatch = true;
        DrawBackground = true;
        Btn1SetON = false;
      }
      else if (!TouchLatch && screenNbr == 2) {
        screenNbr = 0;
        Serial.println("Screen Touched");
        TouchLatch = true;
        DrawBackground = true;
        Btn3SetON = false;
      }
    }


    if (x >= 0 && x <= 320 && y >= 0 && y <= 60 && screenNbr == 2) {
      TouchTime = (millis() - initTouchTime) / 1000;
      if (!TouchLatch && TouchTime >= 2) {
        Serial.println("Screen Touched");
        TouchLatch = true;
        InitRst = true;
        PrevSoC = 0;
      }
    }


    if (x >= 0 && x <= 320 && y >= 0 && y <= 60 && screenNbr == 0) {
      TouchTime = (millis() - initTouchTime) / 1000;
      if (!TouchLatch && TouchTime >= 2) {
        Serial.println("Screen Touched");
        TouchLatch = true;
        DriveOn = true;
        save_lost('P');
        ESP.restart();
      }
    }


    if (x >= 0 && x <= 320 && y >= 0 && y <= 60 && screenNbr == 1) {
      TouchTime = (millis() - initTouchTime) / 1000;
      if (!TouchLatch && TouchTime >= 2) {
        Serial.println("Screen Touched");
        TouchLatch = true;
        DriveOn = true;
        save_lost('P');
      }
    }
  }
  else{
    initTouchTime = millis();
    TouchLatch = false;
  }
}






void drawRoundedRect(RoundedRect toDraw){
  lcd.fillRoundRect(
    toDraw.xStart,
    toDraw.yStart,
    toDraw.xWidth,
    toDraw.yHeight,
    toDraw.cornerRadius,
    toDraw.color
  );

  int box1TextX = toDraw.xStart + (toDraw.xWidth / 2);
  int box1TextY = toDraw.yStart + (toDraw.yHeight / 2);
  lcd.setCursor(box1TextX, box1TextY);
  lcd.setTextDatum(MC_DATUM);
  lcd.setTextPadding(toDraw.xWidth);
  lcd.setTextSize(1);
  lcd.setFreeFont(&FreeSans9pt7b);
  lcd.setTextColor(TFT_BLACK,toDraw.color);
  lcd.drawString(toDraw.BtnText, box1TextX, box1TextY);
}





void DisplayPage() {
  if (DrawBackground) {
    lcd.fillScreen(TFT_BLACK);
    lcd.setTextPadding(160);
    lcd.setTextSize(1);
    lcd.setFreeFont(&FreeSans12pt7b);
    lcd.setTextColor(TFT_WHITE, TFT_BLUE);


    for (int i = 0; i < 10; i++) {
      if (i < 5) {
        lcd.drawString(titre[i], lcd.width() / 4, textLvl[i]);
      }
      else {
        lcd.drawString(titre[i], 3 * (lcd.width() / 4), textLvl[i]);
      }
    }


    lcd.drawLine(1,textLvl[0] - 13,319,textLvl[0] - 13,TFT_DARKGREY);
    lcd.drawLine(lcd.width() / 2,textLvl[0] - 13,lcd.width() / 2,drawLvl[4] + 24,TFT_DARKGREY);
    lcd.drawLine(1,drawLvl[4] + 24,319,drawLvl[4] + 24,TFT_DARKGREY);


    for (int i = 0; i < 10; i++) {
      strcpy(prev_value[i], "");
    }


    switch (screenNbr) {

      case 0:

        drawRoundedRect(btnAon);
        drawRoundedRect(btnBoff);
        drawRoundedRect(btnCoff);
        lcd.setTextSize(1);
        lcd.setFreeFont(&FreeSans18pt7b);
        lcd.setTextColor(MainTitleColor, TFT_BLACK);
        lcd.drawString(Maintitre[screenNbr], lcd.width() / 2, 22);
        break;

      case 1:

        drawRoundedRect(btnAoff);
        drawRoundedRect(btnBon);
        drawRoundedRect(btnCoff);
        lcd.setTextSize(1);
        lcd.setFreeFont(&FreeSans18pt7b);
        lcd.setTextColor(MainTitleColor, TFT_BLACK);
        lcd.drawString(Maintitre[screenNbr], lcd.width() / 2, 22);
        break;

      case 2:

        drawRoundedRect(btnAoff);
        drawRoundedRect(btnBoff);
        drawRoundedRect(btnCon);
        lcd.setTextSize(1);
        lcd.setFreeFont(&FreeSans18pt7b);
        lcd.setTextColor(MainTitleColor, TFT_BLACK);
        lcd.drawString(Maintitre[screenNbr], lcd.width() / 2, 22);
        break;

      case 3:

        drawRoundedRect(btnAoff);
        drawRoundedRect(btnBoff);
        drawRoundedRect(btnCoff);
        lcd.setTextSize(1);
        lcd.setFreeFont(&FreeSans18pt7b);
        lcd.setTextColor(MainTitleColor, TFT_BLACK);
        lcd.drawString(Maintitre[screenNbr], lcd.width() / 2, 22);
        break;
    }
    DrawBackground = false;
  }
  lcd.setTextSize(1);
  lcd.setFreeFont(&FreeSans18pt7b);


  for (int i = 0; i < 10; i++) {
    if (value_float[i] < 0) {
      negative_flag[i] = true;
    }
    else {
      negative_flag[i] = false;
    }

    dtostrf(value_float[i], 3, nbr_decimal[i], value[i]);


    if ((value[i] != prev_value[i]) && (i < 5)) {
      lcd.setTextColor(TFT_BLACK, TFT_BLACK);
      lcd.drawString(prev_value[i], lcd.width() / 4, drawLvl[i]);
      if (negative_flag[i]) {
        lcd.setTextColor(TFT_ORANGE, TFT_BLACK);
        lcd.drawString(value[i], lcd.width() / 4, drawLvl[i]);
      }
      else {
        lcd.setTextColor(TFT_GREEN, TFT_BLACK);
        lcd.drawString(value[i], lcd.width() / 4, drawLvl[i]);
      }
      strcpy(prev_value[i], value[i]);
    }
    else if (value[i] != prev_value[i]) {
      lcd.setTextColor(TFT_BLACK, TFT_BLACK);
      lcd.drawString(prev_value[i], 3 * (lcd.width() / 4), drawLvl[i]);
      if (negative_flag[i]) {
        lcd.setTextColor(TFT_ORANGE, TFT_BLACK);
        lcd.drawString(value[i], 3 * (lcd.width() / 4), drawLvl[i]);
      }
      else {
        lcd.setTextColor(TFT_GREEN, TFT_BLACK);
        lcd.drawString(value[i], 3 * (lcd.width() / 4), drawLvl[i]);
      }
      strcpy(prev_value[i], value[i]);
    }
  }
}






void page1() {

  strcpy(titre[0],"SoC");
  strcpy(titre[1],"PID Cons");
  strcpy(titre[2],"PWR Int Cons");
  strcpy(titre[3],"Cons. 10Km");
  strcpy(titre[4],"PID_kWhLeft");
  strcpy(titre[5],"TripOdo");
  strcpy(titre[6],"Est. range");
  strcpy(titre[7],"Est. range");
  strcpy(titre[8],"Est. range");
  strcpy(titre[9],"full_kWh");
  value_float[0] = SoC;
  value_float[1] = PIDkWh_100;
  value_float[2] = kWh_100km;
  value_float[3] = span_kWh_100km;
  value_float[4] = PID_kWhLeft;
  value_float[5] = TripOdo;
  value_float[6] = Est_range3;
  value_float[7] = Est_range;
  value_float[8] = Est_range2;
  value_float[9] = full_kWh;


  for (int i = 0; i < 10; i++) {
    if (value_float[i] >= 1000) {
      nbr_decimal[i] = 0;
    }
    else if ((value_float[i] < 100) && (value_float[i] > -10)) {
      nbr_decimal[i] = 2;
    }
    else {
      nbr_decimal[i] = 1;
    }
  }

  DisplayPage();
}



void page2() {

  strcpy(titre[0], "SoC");
  strcpy(titre[1], "Full Ah");
  strcpy(titre[2], "MAXcellv");
  strcpy(titre[3], "SOH");
  strcpy(titre[4], "AuxBattV");
  strcpy(titre[5], "BmsSoC");
  strcpy(titre[6], "BATTv");
  strcpy(titre[7], "Cell Vdiff");
  strcpy(titre[8], "InitSoC");
  strcpy(titre[9], "degrad_ratio");
  value_float[0] = SoC;
  value_float[1] = EstFull_Ah;
  value_float[2] = MAXcellv;
  value_float[3] = SOH;
  value_float[4] = AuxBattV;
  value_float[5] = BmsSoC;
  value_float[6] = BATTv;
  value_float[7] = CellVdiff;
  value_float[8] = InitSoC;
  value_float[9] = degrad_ratio;


  for (int i = 0; i < 10; i++) {
    if (value_float[i] >= 1000) {
      nbr_decimal[i] = 0;
    }
    else if ((value_float[i] < 100) && (value_float[i] > -10)) {
      nbr_decimal[i] = 2;
    }
    else {
      nbr_decimal[i] = 1;
    }
  }

  DisplayPage();
}



void page3() {

  strcpy(titre[0], "Power");
  strcpy(titre[1], "MIN Temp");
  strcpy(titre[2], "PID kWh");
  strcpy(titre[3], "kWh Used");
  strcpy(titre[4], "SoC");
  strcpy(titre[5], "Max Pwr");
  strcpy(titre[6], "MAX Temp");
  strcpy(titre[7], "Int. Energ");
  strcpy(titre[8], "kWh Left");
  strcpy(titre[9], "Chauf. Batt.");
  value_float[0] = Power;
  value_float[1] = BattMinT;
  value_float[2] = Net_kWh2;
  value_float[3] = used_kWh;
  value_float[4] = SoC;
  value_float[5] = Max_Pwr;
  value_float[6] = BattMaxT;
  value_float[7] = acc_energy;
  value_float[8] = left_kWh;
  value_float[9] = Heater;


  for (int i = 0; i < 10; i++) {
    if (value_float[i] >= 1000) {
      nbr_decimal[i] = 0;
    }
    else if ((value_float[i] < 100) && (value_float[i] > -10)) {
      nbr_decimal[i] = 2;
    }
    else {
      nbr_decimal[i] = 1;
    }
  }

  DisplayPage();
}



void page4() {

  strcpy(titre[0], "Wifi_select");
  strcpy(titre[1], "MINcellv");
  strcpy(titre[2], "MAXcellv");
  strcpy(titre[3], "Max_Reg");
  strcpy(titre[4], "distance");
  strcpy(titre[5], "dist_save");
  strcpy(titre[6], "Cell nbr");
  strcpy(titre[7], "Cell nbr");
  strcpy(titre[8], "OUT temp");
  strcpy(titre[9], "IN temp");
  value_float[0] = Wifi_select;
  value_float[1] = MINcellv;
  value_float[2] = MAXcellv;
  value_float[3] = Max_Reg;
  value_float[4] = distance;
  value_float[5] = dist_save;
  value_float[6] = MINcellvNb;
  value_float[7] = MAXcellvNb;
  value_float[8] = OUTDOORtemp;
  value_float[9] = INDOORtemp;


  for (int i = 0; i < 10; i++) {
    if (value_float[i] >= 1000) {
      nbr_decimal[i] = 0;
    }
    else if ((value_float[i] < 100) && (value_float[i] > -10)) {
      nbr_decimal[i] = 2;
    }
    else {
      nbr_decimal[i] = 1;
    }
  }

  DisplayPage();
}






void loop()
{

  if ((BMS_ign || Charging || ResetOn) && OBD2connected){
    pid_counter++;
    read_data();
  }
  else if (((millis() - read_timer) > read_data_interval) && OBD2connected){
    read_data();
    read_timer = millis();
  }


  button();


  if (send_enabled){
    ready = GSheet.ready();
    if (!ready){
      nbr_notReady += 1;
      Serial.print("GSheet not ready");
      GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);
      ready = GSheet.ready();
    }
    if(nbr_fails > 10 || nbr_notReady > 10){
      save_lost('P');
      send_enabled = false;
      nbr_fails = 0;
      nbr_notReady = 0;

    }
    if(dist_save >= 25 && !save_sent){
      save_sent = true;
      save_lost('P');
      init_distsave = Trip_dist;
    }
    if (sending_data){

      timeinfo = getTime();
      Serial.print("Time updated: ");


      sprintf(EventTime, "%02d-%02d-%04d %02d:%02d:%02d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

      send_data = true;
      sending_data = false;
    }


    if (datasent){
      lcd.fillCircle(20, 20, 6,TFT_GREEN);
      if (millis() - GSheetTimer >= 500){
        datasent = false;
      }
    }
    else if (failsent){
      lcd.fillCircle(20, 20, 6,TFT_RED);
      if (millis() - GSheetTimer >= 500){
        failsent = false;
      }
    }
    else if (!ready){
      lcd.fillCircle(20, 20, 6,TFT_WHITE);
    }
    else{
      lcd.fillCircle(20, 20, 6,TFT_BLACK);
    }
  }



  if ((ESP_on || Charging) && SoC != 0) {
    Serial.println(" ESP is ON");
    if (display_off){
      Serial.println("Turning Display ON");
      lcd.setBrightness(128);
      display_off = false;
      SoC_saved = false;

      Serial.println("Display going ON");
      DrawBackground = true;
    }

    if (StartWifi == 1) {
      if (!send_enabled){
        lcd.fillCircle(300, 20, 6,TFT_WHITE);
      }
      else if (WiFi.status() == WL_CONNECTED){
        lcd.fillCircle(300, 20, 6,TFT_GREEN);
      }
      else{
        lcd.fillCircle(300, 20, 6,TFT_RED);
      }
    }
     else{
      lcd.fillCircle(300, 20, 6,TFT_BLACK);
    }


    switch (screenNbr) {
      case 0: page1(); break;
      case 1: page2(); break;
      case 2: page3(); break;
      case 3: page4(); break;
    }
  }


  else {

    display_off = true;
  }

  ResetCurrTrip();
}