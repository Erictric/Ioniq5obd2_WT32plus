/*  Ionic5obd2 for Hyundai Ioniq 5 + OBD Vgate iCar Pro BT4.0 + WT32-SC01 3.5" display
    Version: v2.3.0

    SafeString by Matthew Ford: https://www.forward.com.au/pfod/ArduinoProgramming/SafeString/index.html
    Elmduino by PowerBroker2: https://github.com/PowerBroker2/ELMduino
    Data looging to Google Sheet: https://randomnerdtutorials.com/esp32-datalogging-google-sheets/

  Requirements:
  - Development board : WT32-SC01-Plus_ESP32-S3
  - Arduino Library - Display/Touch : LovyanGFX
  - Board selected in Arduino : ESP32S3 Dev Module
    ELMduino library used is version 2.41, problem not resolved with newer version
    ELMduino.h needs to be modified as follow: bool begin(Stream& stream, char protocol='6', uint16_t payloadLen = 240);
  
  Library folder:
  c:\Users\emond\OneDrive\Documents\Arduino\libraries\
*/

#include "Arduino.h"
#include "LGFX_CLASS.h"
#include "BT_communication.h"
#include "WiFi.h"
#include "Wifi_connection.h"
#include <WebServer.h>
#include "SafeString.h"
#include "EEPROM.h"
#include <Adafruit_GFX.h>
#include <time.h>
#include <freertos/FreeRTOS.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <freertos/task.h>
#include "ELMduino.h"
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include "WebServer_handlers.h"

// Application Version - Update with code changes
// Format: MAJOR.MINOR.PATCH
// MAJOR: Breaking changes or major new features
// MINOR: New features, improvements, non-breaking changes
// PATCH: Bug fixes and minor tweaks
const char* APP_VERSION = "v2.3.2";

static LGFX lcd;            // declare display variable
extern ELM327 myELM327;     // declare ELM327 object

#define DEBUG_PORT Serial

bool display_off = true;
bool sdCardAvailable = false;
const char* csvFilename = "/ioniq5_log.csv";
const char* socDecreaseFilename = "/ioniq5_soc_decrease.csv";
const char* archiveDir = "/archive";

// Archive settings
const unsigned long MAX_FILE_SIZE = 1 * 1024 * 1024;  // 1MB max file size before archiving
const unsigned long MAX_FILE_AGE_DAYS = 30;  // Archive files older than 30 days
bool autoArchiveEnabled = true;  // Enable automatic archiving

// Web server on port 80
WebServer server(80);

//LCD y positions for texts and numbers
uint16_t textLvl[10] = {65, 135, 205, 275, 345, 65, 135, 205, 275, 345};  // y coordinates for text
uint16_t drawLvl[10] = {100, 170, 240, 310, 380, 100, 170, 240, 310, 380}; // and numbers

#define N_km 10        //variable for the calculating kWh/100km over a N_km

boolean ResetOn = true;
int screenNbr = 0;
bool showSaveConfirmation = false;  // Flag to show save confirmation dialog
bool showWiFiInfo = false;  // Flag to show WiFi information screen
bool showOBD2FailScreen = false;  // Flag to show OBD2 connection failure screen
bool showDeviceSelection = false;  // Flag to show BLE device selection screen
bool lowBattAlertShown = false;  // Flag to track if low 12V battery alert has been shown
char selectedBLEDevice[21] = "";  // Selected OBD2 device name (max 20 chars + null)
char bleDeviceList[10][21] = { "" };  // Store up to 10 BLE devices found (max 20 chars + null)
int bleDeviceCount = 0;  // Number of BLE devices found
int bleDevicePageNumber = 0;  // Current page in BLE device list (0-based)
bool bleScanning = false;  // Flag to indicate BLE scan in progress
int wifiScreenPage = 0;  // 0=scan list, 1=keyboard for SSID, 2=keyboard for password
char wifiSSIDList[20][17] = { "" };  // Store up to 20 WiFi networks (max 16 chars + null)
int wifiRSSI[20];         // Store signal strength
int wifiCount = 0;        // Number of networks found
int selectedWiFi = -1;    // Selected network index
char enteredSSID[17] = "";  // Manual SSID entry (max 16 chars + null)
char enteredPassword[17] = "";  // Password entry (max 16 chars + null)
// Different character sets for keyboard
char keyboardLower[] = "abcdefghijklmnopqrstuvwxyz";
char keyboardUpper[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char keyboardNum[] = "0123456789_-@.!#$%&*";
char* currentKeyboard = keyboardLower;  // Pointer to current keyboard set
int currentKeyboardSize = 26;  // Size of current keyboard set
int keyboardMode = 0;  // 0=lowercase, 1=uppercase, 2=numbers/symbols
int keyboardIndex = 0;

uint8_t record_code = 0;
extern bool initscan;  // Defined in Wifi_connection.h
float mem_energy = 0;
float mem_PrevSoC = 0;
float mem_SoC = 0;
float mem_Power = 0;
float mem_LastSoC = 0;
float Wifi_select = 1;
bool data_ready = false;
bool sd_condition1 = false;
bool sd_condition2 = false;
bool SoC_saved = false;
bool shutdown_esp = false;
bool wifiReconn = false;
uint16_t nbr_saved = 0;

float BattMinT;
float BattMaxT;
float AuxBattV;
float AuxBattV_old;
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
float FrontMotor;
float RearMotor;
byte TransSelByte;
byte Park;
byte Reverse;
byte Neutral;
byte Drive;
char selector[1];
bool OBDscanning = false;
bool Charging = false;
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

float AuxBatt_SoC = 0;              // 12V battery State of Charge (%)
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
float energy_array[11]; //needs to hold 11 values in order to calculate last 10 energy values
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
unsigned long  RangeCalcUpdate = 2000; 
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
bool StartWifi = true;
bool InitRst = false;
bool TrigRst = false;
bool kWh_update = false;
bool SoC_decreased = false;
bool corr_update = false;
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

float fullBattCapacity = 73.25;  // capacity of displayed SoC, extra 4.5% (3kWh) is available below 0% display SoC
float SoC100 = 100;
double b = 0.671;
double a;
float max_kwh;
float min_kwh;
float return_kwh;

// Variables for touch x,y
static int32_t x, y;
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

/*///////ESP shutdown variables///////*/
unsigned long ESPinitTimer = 0;
unsigned long ESPTimerInterval = 1200;  // time in seconds to turn off ESP when it power-up during 12V battery charge cycle.
unsigned long shutdown_timer = 0;
unsigned long stopESP_timer = 0;

/*////// Variables for SD card data logging ////////////*/

bool sending_data = false;  // Flag to trigger SD card save every 10 seconds
unsigned long sendInterval = 10000;  // SD card save interval in millisec
unsigned long SDsaveTimer = 0;  // Timer for SD card save LED flash
bool sendIntervalOn = false;  // Flag for kWh_corr timing logic
bool sdCardSaved = false;  // Flag to indicate SD card save for LED flash
tm timeinfo;   // Variable to save current epoch time
uint16_t nbrDays[13] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366};  // Array for summer time logics
char EventTime[18];   // Array to send formatted time
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

// NTP server to request epoch time
const char* ntpServer = "pool.ntp.org";

// Timezone configuration (select your timezone)
// Common timezones (offset in seconds, DST offset in seconds)
// EST/EDT (Eastern): -18000, 3600  |  CST/CDT (Central): -21600, 3600
// MST/MDT (Mountain): -25200, 3600 |  PST/PDT (Pacific): -28800, 3600
// UTC: 0, 0  |  CET/CEST (Central Europe): 3600, 3600
// JST (Japan): 32400, 0  |  AEST/AEDT (Sydney): 36000, 3600
const long gmtOffset_sec = -18000;     // EST: -5 hours in seconds
const int daylightOffset_sec = 3600;   // DST: +1 hour in seconds

/*////// Variables for OBD data timing ////////////*/
uint8_t pid_counter = 0;

/*///////Define a structure to store the PID query data frames ///////////*/
struct dataFrames_struct {
  char frames[9][20];  // 9 frames each of 20chars
};

typedef struct dataFrames_struct dataFrames;  // create a simple name for this type of data
dataFrames results;                           // this struct will hold the results

void callback(){  //required function for touch wake
  //placeholder callback function
}

hw_timer_t *Timer0_Cfg = NULL;
int send_update = 10000000;   // Send data update time in uSec (10 seconds for SD card saves)

void IRAM_ATTR Timer0_ISR()
{
  if (OBD2connected) {
    sending_data = true;  // Trigger SD card save every 10 seconds
  }
}

// Function that gets current epoch time

tm getTime() {
  static time_t fallbackTime = 0;  // Persistent fallback time counter
  time_t now;
  struct tm timeinfo;
  
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time - using simulated time");
    
    // Initialize fallback time to a default date if not set
    if (fallbackTime == 0) {
      // Set default to November 27, 2025, 12:00:00 EST
      struct tm defaultTime;
      defaultTime.tm_year = 2025 - 1900;
      defaultTime.tm_mon = 1;   // November
      defaultTime.tm_mday = 1;
      defaultTime.tm_hour = 12;
      defaultTime.tm_min = 0;
      defaultTime.tm_sec = 0;
      fallbackTime = mktime(&defaultTime);
    } else {
      // Increment by send_update interval (convert microseconds to seconds)
      fallbackTime += send_update / 1000000;  // send_update is in microseconds
    }
    
    // Convert fallback time to tm struct
    localtime_r(&fallbackTime, &timeinfo);
    return timeinfo;
  }
  
  time(&now);
  fallbackTime = 0;  // Reset fallback when real time is available
  
  return timeinfo;
}

// Web server handlers are in WebServer_handlers.h

/*/////////////////////////////////////////////////////////////////*/
/*                    FILE ARCHIVING FUNCTIONS                     */
/*/////////////////////////////////////////////////////////////////*/

// Create archive directory if it doesn't exist
bool ensureArchiveDir() {
  if (!SD.exists(archiveDir)) {
    Serial.println("Creating archive directory...");
    if (SD.mkdir(archiveDir)) {
      Serial.println("Archive directory created");
      return true;
    } else {
      Serial.println("Failed to create archive directory");
      return false;
    }
  }
  return true;
}

// Get file size
size_t getFileSize(const char* filename) {
  if (!SD.exists(filename)) {
    return 0;
  }
  File file = SD.open(filename, FILE_READ);
  if (!file) {
    return 0;
  }
  size_t size = file.size();
  file.close();
  return size;
}

// Archive a file by moving it to archive directory with timestamp
bool archiveFile(const char* filename) {
  if (!sdCardAvailable || !SD.exists(filename)) {
    return false;
  }
  
  // Ensure archive directory exists
  if (!ensureArchiveDir()) {
    return false;
  }
  
  // Get current timestamp
  char timestamp[32];
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    strcpy(timestamp, "unknown");
  } else {
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", &timeinfo);
  }
  
  // Create archived filename
  char originalName[64];
  strncpy(originalName, filename, sizeof(originalName) - 1);
  originalName[sizeof(originalName) - 1] = '\0';
  // Remove leading slash if present
  if (originalName[0] == '/') {
    memmove(originalName, originalName + 1, strlen(originalName));
  }
  char archivedName[128];
  snprintf(archivedName, sizeof(archivedName), "%s/%s", archiveDir, originalName);

  // Add timestamp before extension
  char* dot = strrchr(archivedName, '.');
  if (dot) {
    char base[128], ext[16];
    size_t baseLen = dot - archivedName;
    strncpy(base, archivedName, baseLen);
    base[baseLen] = '\0';
    strncpy(ext, dot, sizeof(ext) - 1);
    ext[sizeof(ext) - 1] = '\0';
    snprintf(archivedName, sizeof(archivedName), "%s_%s%s", base, timestamp, ext);
  } else {
    strncat(archivedName, "_", sizeof(archivedName) - strlen(archivedName) - 1);
    strncat(archivedName, timestamp, sizeof(archivedName) - strlen(archivedName) - 1);
  }
  
  Serial.printf("Archiving %s to %s\n", filename, archivedName);
  
  // Read source file
  File sourceFile = SD.open(filename, FILE_READ);
  if (!sourceFile) {
    Serial.println("Failed to open source file for archiving");
    return false;
  }
  
  // Create destination file
  File destFile = SD.open(archivedName, FILE_WRITE);
  if (!destFile) {
    Serial.println("Failed to create archive file");
    sourceFile.close();
    return false;
  }
  
  // Copy data
  uint8_t buffer[512];
  while (sourceFile.available()) {
    size_t bytesRead = sourceFile.read(buffer, sizeof(buffer));
    destFile.write(buffer, bytesRead);
  }
  
  sourceFile.close();
  destFile.close();
  
  // Delete original file
  if (SD.remove(filename)) {
    Serial.printf("File archived successfully: %s\n", archivedName);
    return true;
  } else {
    Serial.println("Failed to delete original file after archiving");
    return false;
  }
}

// Check if file needs archiving based on size
bool checkFileSize(const char* filename) {
  size_t fileSize = getFileSize(filename);
  if (fileSize >= MAX_FILE_SIZE) {
    Serial.printf("File %s size (%lu bytes) exceeds limit, archiving...\n", filename, fileSize);
    return archiveFile(filename);
  }
  return false;
}

// Archive old files and recreate with headers
// forceArchive: if true, archives regardless of size (used for event codes)
void archiveAndRecreateFiles(bool forceArchive = false) {
  if (!sdCardAvailable || !autoArchiveEnabled) {
    return;
  }
  
  // Check and archive main log file
  bool mainArchived = false;
  if (forceArchive) {
    Serial.println("Event code detected - archiving main log file...");
    mainArchived = archiveFile(csvFilename);
  } else {
    mainArchived = checkFileSize(csvFilename);
  }
  
  if (mainArchived) {
    // Recreate main log file with header
    File file = SD.open(csvFilename, FILE_WRITE);
    if (file) {
      file.println("Timestamp\tSoC\tBmsSoC\tPower\tTripOdo\tBattMinT\tBattMaxT\tHeater\tOUTDOORtemp\tINDOORtemp\tNet_kWh\tNet_kWh2\tacc_energy\tNet_Ah\tacc_Ah\tEstFull_Ah\tMax_Pwr\tMax_Reg\tMAXcellv\tMINcellv\tMAXcellvNb\tMINcellvNb\tBATTv\tBATTc\tAuxBattV\tCEC\tCED\tCDC\tCCC\tSOH\tused_kWh\tleft_kWh\tfull_kWh\tstart_kWh\tPID_kWhLeft\tdegrad_ratio\tInitSoC\tLastSoC\tPIDkWh_100\tkWh_100km\tspan_kWh_100km\tTrip_dist\tdistance\tSpeed\tOdometer\tOPtimemins\tTripOPtime\tCurrOPtime\tTireFL_P\tTireFR_P\tTireRL_P\tTireRR_P\tTireFL_T\tTireFR_T\tTireRL_T\tTireRR_T\tnbr_saved\tAuxBattV_old");
      file.close();
      Serial.println("Main log file recreated with header");
    }
  }
  
  // Check and archive SoC decrease file
  bool socArchived = false;
  if (forceArchive) {
    Serial.println("Event code detected - archiving SoC decrease file...");
    socArchived = archiveFile(socDecreaseFilename);
  } else {
    socArchived = checkFileSize(socDecreaseFilename);
  }
  
  if (socArchived) {
    // Recreate SoC decrease file with header
    File file = SD.open(socDecreaseFilename, FILE_WRITE);
    if (file) {
      file.println("Timestamp\tSoC\tBmsSoC\tPower\tTripOdo\tBattMinT\tBattMaxT\tHeater\tOUTDOORtemp\tINDOORtemp\tNet_kWh\tNet_kWh2\tacc_energy\tNet_Ah\tacc_Ah\tEstFull_Ah\tMax_Pwr\tMax_Reg\tMAXcellv\tMINcellv\tMAXcellvNb\tMINcellvNb\tBATTv\tBATTc\tAuxBattV\tCEC\tCED\tCDC\tCCC\tSOH\tused_kWh\tleft_kWh\tfull_kWh\tstart_kWh\tPID_kWhLeft\tdegrad_ratio\tInitSoC\tLastSoC\tPIDkWh_100\tkWh_100km\tspan_kWh_100km\tTrip_dist\tdistance\tSpeed\tOdometer\tOPtimemins\tTripOPtime\tCurrOPtime\tTireFL_P\tTireFR_P\tTireRL_P\tTireRR_P\tTireFL_T\tTireFR_T\tTireRL_T\tTireRR_T\tnbr_saved\tAuxBattV_old");
      file.close();
      Serial.println("SoC decrease file recreated with header");
    }
  }
}

// Function to draw the welcome screen - used both at startup and on retry
void drawWelcomeScreen() {
  lcd.fillScreen(TFT_BLACK);
  
  // Title Section - centered between top (0) and separator line (230)
  // Main title with shadow effect and larger font
  lcd.setFont(&FreeSansBold24pt7b);
  // Shadow
  lcd.setTextColor(TFT_DARKGREY);
  lcd.drawString("IONIQ 5", 162, 62);
  // Main text with gradient effect using multiple colors
  lcd.setTextColor(TFT_WHITE);
  lcd.drawString("IONIQ 5", 160, 60);
  
  // Subtitle with bold font and electric blue accent
  lcd.setFont(&FreeSansBold18pt7b);
  // Draw "OBD2" with electric blue glow effect
  lcd.setTextColor(0x047F); // Dark blue shadow
  lcd.drawString("OBD2", 162, 127);
  lcd.setTextColor(TFT_CYAN);
  lcd.drawString("OBD2", 160, 125);
  
  // "Data Logger" subtitle
  lcd.setFont(&FreeSans18pt7b);
  lcd.setTextColor(TFT_LIGHTGREY);
  lcd.drawString("Data Logger", 160, 165);
  
  // Version display
  lcd.setFont(&FreeSans12pt7b);
  lcd.setTextColor(TFT_DARKGREY);
  lcd.drawString(APP_VERSION, 160, 200);
  
  // Separator line with gradient effect
  lcd.drawLine(40, 230, 280, 230, TFT_CYAN);
  lcd.drawLine(40, 231, 280, 231, TFT_DARKGREY);
  
  // System Status Section
  lcd.setFont(&FreeSans12pt7b);
  lcd.setTextColor(TFT_LIGHTGREY);
  lcd.drawString("System Initialization", 160, 270);
}

void setup(void)
{
  // Use 115200 baud for faster serial communication and to match ESP32 default.
  // This is necessary for high-speed data transfer and compatibility with most ESP32 development tools.
  DEBUG_PORT.begin(115200);
  DEBUG_PORT.setDebugOutput(false);
  
  // Suppress BT_BTM debug messages
  esp_log_level_set("BT_BTM", ESP_LOG_ERROR);
  
  //while (!Serial) {
  //  ;  // wait for serial port to connect. Needed for native USB port only
  //}

  DEBUG_PORT.println("=== IONIQ 5 OBD2 Logger ===");
  DEBUG_PORT.println("Serial Monitor Active\n");
  delay(1000);

  // Check for PSRAM and initialize display with PSRAM buffer if available
  bool psramAvailable = psramInit();
  if (psramAvailable) {
    size_t psramSize = ESP.getPsramSize();
    size_t freePsram = ESP.getFreePsram();
    DEBUG_PORT.printf("PSRAM detected: %d bytes total, %d bytes free\n", psramSize, freePsram);
    DEBUG_PORT.println("Display will use PSRAM for sprite buffers");
  } else {
    DEBUG_PORT.println("No PSRAM detected, using internal RAM for display buffers");
  }

  lcd.init();
  lcd.setRotation(2);  // 0 & 2 Portrait. 1 & 3 landscape
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_GREEN);
  lcd.setCursor(0, 0);
  lcd.setTextDatum(MC_DATUM);
  lcd.setTextSize(1);
  lcd.setFreeFont(&FreeSans12pt7b);

  /*////// initialize EEPROM with predefined size ////////*/
  EEPROM.begin(256);  // Increased to accommodate WiFi credentials and BLE device name (72-103: SSID, 104-167: Password, 168: hasCustomWiFi, 169-200: BLE Device)

  
  /*////// Get the stored value from last re-initialisation /////*/

  InitRemain_kWh = EEPROM.readFloat(0);
  InitCED = EEPROM.readFloat(4);
  InitCEC = EEPROM.readFloat(8);
  InitSoC = EEPROM.readFloat(12);
  previous_kWh = EEPROM.readFloat(16);
  //UsedSoC = EEPROM.readFloat(16);
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
  send_enabled_float = EEPROM.readFloat(68);
  send_enabled = (send_enabled_float >= 0.5);  // Restore send_enabled from EEPROM (1.0 = enabled, 0.0 = disabled)
  
  // Load saved BLE device name from EEPROM (address 169-200, 32 bytes)
  char savedBLEDevice[32];
  for (int i = 0; i < 32; i++) {
    savedBLEDevice[i] = EEPROM.read(169 + i);
  }
  savedBLEDevice[31] = '\0';  // Ensure null termination
  
  // If a valid device name is saved, use it; otherwise default to "IOS-Vlink"
  if (savedBLEDevice[0] != 0 && savedBLEDevice[0] != 255) {
    strncpy(selectedBLEDevice, savedBLEDevice, 20);
    selectedBLEDevice[20] = '\0';
    Serial.printf("Loaded BLE device from EEPROM: %s\n", selectedBLEDevice);
  } else {
    strncpy(selectedBLEDevice, "IOS-Vlink", 20);
    selectedBLEDevice[20] = '\0';
    Serial.println("No saved BLE device, using default: IOS-Vlink");
  }
  
  //acc_kWh_10 = EEPROM.readFloat(72);
  //acc_kWh_0 = EEPROM.readFloat(76);
  //acc_kWh_m10 = EEPROM.readFloat(80);
  //acc_kWh_m20 = EEPROM.readFloat(84);
  //acc_kWh_m20p = EEPROM.readFloat(88);
  //acc_time_25 = EEPROM.readFloat(92);
  //acc_time_10 = EEPROM.readFloat(96);
  //acc_time_0 = EEPROM.readFloat(100);
  //acc_time_m10 = EEPROM.readFloat(104);
  //acc_time_m20 = EEPROM.readFloat(108);
  //acc_time_m20p = EEPROM.readFloat(112);
  //acc_dist_25 = EEPROM.readFloat(116);
  //acc_dist_10 = EEPROM.readFloat(120);
  //acc_dist_0 = EEPROM.readFloat(124);
  //acc_dist_m10 = EEPROM.readFloat(128);
  //acc_dist_m20 = EEPROM.readFloat(132);
  //acc_dist_m20p = EEPROM.readFloat(136);
  //acc_regen = EEPROM.readFloat(140);

  //initial_eeprom(); //if a new eeprom memory is used it needs to be initialize to something first

  /* uncomment if you need to display Safestring results on Serial Monitor */
  SafeString::setOutput(Serial);

  /*/////////////////////////////////////////////////////////////////*/
  /*                    INITIALIZE SD CARD (SPI MODE)                */
  /*/////////////////////////////////////////////////////////////////*/
  //Serial.println("Initializing SD card via SPI...");
  // WT32-SC01 Plus SPI pins: MOSI=40, MISO=38, SCK=39, CS=41
  SPI.begin(39, 38, 40, 41);  // SCK, MISO, MOSI, CS
  
  if (!SD.begin(41, SPI)) {  // CS pin 41
    Serial.println("SD Card Mount Failed - card may not be formatted as FAT32");
    Serial.println("Please format the SD card as FAT32 on your computer");
    sdCardAvailable = false;
  } else {
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
      Serial.println("No SD card attached");
      sdCardAvailable = false;
    } else {
      Serial.print("SD Card Type: ");
      if (cardType == CARD_MMC) Serial.println("MMC");
      else if (cardType == CARD_SD) Serial.println("SDSC");
      else if (cardType == CARD_SDHC) Serial.println("SDHC");
      else Serial.println("UNKNOWN");
      
      uint64_t cardSize = SD.cardSize() / (1024 * 1024);
      Serial.printf("SD Card Size: %lluMB\n", cardSize);
      sdCardAvailable = true;
      
      // Create CSV header if file doesn't exist
      if (!SD.exists(csvFilename)) {
        File file = SD.open(csvFilename, FILE_WRITE);
        if (file) {
          file.println("Timestamp\tSoC\tBmsSoC\tPower\tTripOdo\tBattMinT\tBattMaxT\tHeater\tOUTDOORtemp\tINDOORtemp\tNet_kWh\tNet_kWh2\tacc_energy\tNet_Ah\tacc_Ah\tEstFull_Ah\tMax_Pwr\tMax_Reg\tMAXcellv\tMINcellv\tMAXcellvNb\tMINcellvNb\tBATTv\tBATTc\tAuxBattV\tCEC\tCED\tCDC\tCCC\tSOH\tused_kWh\tleft_kWh\tfull_kWh\tstart_kWh\tPID_kWhLeft\tdegrad_ratio\tInitSoC\tLastSoC\tPIDkWh_100\tkWh_100km\tspan_kWh_100km\tTrip_dist\tdistance\tSpeed\tOdometer\tOPtimemins\tTripOPtime\tCurrOPtime\tTireFL_P\tTireFR_P\tTireRL_P\tTireRR_P\tTireFL_T\tTireFR_T\tTireRL_T\tTireRR_T\tnbr_saved\tAuxBattV_old");
          file.close();
          Serial.println("CSV header created");
        } else {
          Serial.println("Failed to create CSV header");
        }
      } else {
        Serial.println("CSV file already exists");
      }
      
      // Create SoC decrease file with header if it doesn't exist
      if (!SD.exists(socDecreaseFilename)) {
        File file = SD.open(socDecreaseFilename, FILE_WRITE);
        if (file) {
          file.println("Timestamp\tSoC\tBmsSoC\tPower\tTripOdo\tBattMinT\tBattMaxT\tHeater\tOUTDOORtemp\tINDOORtemp\tNet_kWh\tNet_kWh2\tacc_energy\tNet_Ah\tacc_Ah\tEstFull_Ah\tMax_Pwr\tMax_Reg\tMAXcellv\tMINcellv\tMAXcellvNb\tMINcellvNb\tBATTv\tBATTc\tAuxBattV\tCEC\tCED\tCDC\tCCC\tSOH\tused_kWh\tleft_kWh\tfull_kWh\tstart_kWh\tPID_kWhLeft\tdegrad_ratio\tInitSoC\tLastSoC\tPIDkWh_100\tkWh_100km\tspan_kWh_100km\tTrip_dist\tdistance\tSpeed\tOdometer\tOPtimemins\tTripOPtime\tCurrOPtime\tTireFL_P\tTireFR_P\tTireRL_P\tTireRR_P\tTireFL_T\tTireFR_T\tTireRL_T\tTireRR_T\tnbr_saved\tAuxBattV_old");
          file.close();
          Serial.println("SoC decrease CSV header created");
        } else {
          Serial.println("Failed to create SoC decrease CSV header");
        }
      } else {
        Serial.println("SoC decrease CSV file already exists");
      }
      
      // Check if files need archiving
      Serial.println("Checking for files that need archiving...");
      archiveAndRecreateFiles();
    }
  }

  
  /*/////////////////////////////////////////////////////////////////*/
  /*                  CONFIGURE TIME ZONE                            */
  /*/////////////////////////////////////////////////////////////////*/
  // Set timezone even without WiFi so local time offset is applied
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  //Serial.println("Timezone configured for EST/EDT");
  
  /*/////////////////////////////////////////////////////////////////*/
  /*                    WELCOME SCREEN TITLE                         */
  /*/////////////////////////////////////////////////////////////////*/
  drawWelcomeScreen();
  
  /*/////////////////////////////////////////////////////////////////*/
  /*                     CONNECTION TO WIFI                         */
  /*/////////////////////////////////////////////////////////////////*/

  //DEBUG_PORT.print("StartWifi = ");
  //DEBUG_PORT.println(StartWifi);

  if (StartWifi) {
    //DEBUG_PORT.println("Attempting WiFi connection...");
    ConnectWifi(lcd, Wifi_select);
  }   

  /*/////////////////////////////////////////////////////////////////*/
  /*                    CONNECTION TO OBDII                          */
  /*/////////////////////////////////////////////////////////////////*/
  // Draw connecting message immediately so it appears right after WiFi on screen
  lcd.setTextColor(TFT_YELLOW);
  lcd.drawString("OBD2: Connecting...", lcd.width() / 2, 410);
  Serial.println("...Connection to OBDII...");
  
  ConnectToOBD2(lcd);

  // Setup Web Server after connections (Google Sheets disabled)
  if (StartWifi && WiFi.status() == WL_CONNECTED) {
        
    // Re-configure timezone after WiFi connects to ensure NTP uses local time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    DEBUG_PORT.println("Timezone re-configured after WiFi connection");
    delay(200);  // Brief delay for NTP sync
    
    // Initialize web server for SD card access
    server.on("/", handleRoot);
    server.on("/download", handleDownload);
    server.on("/view", handleView);
    server.on("/delete", handleDelete);
    server.on("/download_soc", handleDownloadSoC);
    server.on("/view_soc", handleViewSoC);
    server.on("/delete_soc", handleDeleteSoC);
    server.on("/archives", handleArchives);
    server.on("/archive_download", handleArchiveDownload);
    server.on("/archive_delete", handleArchiveDelete);
    server.onNotFound(handleNotFound);
    server.begin();
    DEBUG_PORT.print("Web server started at http://");
    DEBUG_PORT.println(WiFi.localIP());
  }

     
  // Only clear screen if OBD2 connected or no fail screen showing
  if (!showOBD2FailScreen) {
    lcd.fillScreen(TFT_BLACK);
  }

  // Configure Timer0 Interrupt  
  Timer0_Cfg = timerBegin(0, 80, true);
  timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);
  timerAlarmWrite(Timer0_Cfg, send_update, true);
  timerAlarmEnable(Timer0_Cfg);
}

/*////////////////////////////////////////////////////////////////////////*/
/*                         END OF SETUP                                   */
/*////////////////////////////////////////////////////////////////////////*/

//----------------------------------------------------------------------------------------
//              OBDII Payloads Processing Functions
//----------------------------------------------------------------------------------------

void clearResultFrames(dataFrames& results) {
  for (int i = 0; i < 9; i++) {
    results.frames[i][0] = '\0';
  }
}

// format is <headerBytes> then <frameNumberByte>:<frameDataBytes> repeated
void processPayload(char* OBDdata, size_t datalen, dataFrames& results) {
  cSFPS(data, OBDdata, datalen);  // wrap in a SafeString
  clearResultFrames(results);
  size_t idx = data.indexOf(':');  // skip over header and find first delimiter
  while (idx < data.length()) {
    int frameIdx = data[idx - 1] - '0';      // the char before :
    if ((frameIdx < 0) || (frameIdx > 8)) {  // error in frame number skip this frame, print a message here

      //SafeString::Output.print("frameIdx:"); SafeString::Output.print(frameIdx); SafeString::Output.print(" outside range data: "); data.debug();
      idx = data.indexOf(':', idx + 1);  // step over : and find next :
      continue;
    }
    cSFA(frame, results.frames[frameIdx]);    // wrap a result frame in a SafeString to store this frame's data
    idx++;                                    // step over :
    size_t nextIdx = data.indexOf(':', idx);  // find next :
    if (nextIdx == data.length()) {
      data.substring(frame, idx);  // next : not found so take all the remaining chars as this field
    } else {
      data.substring(frame, idx, nextIdx - 1);  // substring upto one byte before next :
    }
    //SafeString::Output.print("frameIdx:"); SafeString::Output.print(frameIdx); SafeString::Output.print(" "); frame.debug();
    idx = nextIdx;  // step onto next frame
  }
}

//------------------------------------------------------------------------------------------
//                  End of Payloads Processing
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//             Bytes extraction from dataFrame
//------------------------------------------------------------------------------------------

int convertToInt(char* dataFrame, size_t startByte, size_t numberBytes) {
  int offset = (startByte - 1) * 2;
  // define a local SafeString on the stack for this method
  cSFP(frame, dataFrame);
  cSF(hexSubString, frame.capacity());                                // allow for taking entire frame as a substring
  frame.substring(hexSubString, offset, offset + (numberBytes * 2));  // endIdx in exclusive in SafeString V2
  hexSubString.debug(F(" hex number "));
  long num = 0;
  if (!hexSubString.hexToLong(num)) {
    hexSubString.debug(F(" invalid hex number "));
  }
  return num;
}

//------------------------------------------------------------------------------------------
//               End of Bytes extraction from dataFrame
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//         Data retreived from OBD2 and extract value of it
//------------------------------------------------------------------------------------------

void read_data() {

  Serial.print("pid_counter: ");
  Serial.println(pid_counter);

  button();

  // read in rawData via ODBII
  
  //  Read PID 220101 each iteration to get faster battery power update
  myELM327.sendCommand("AT SH 7E4");  // Set Header for BMS

  
  if (myELM327.queryPID("220101")) {  // Service and Message PID = hex 22 0101 => dec 34, 257
  //if (myELM327.nb_rx_state ==  ELM_SUCCESS) { 
    delayMicroseconds(10); // 10 uSec delay between PID queries
    char* payload = myELM327.payload;
    size_t payloadLen = myELM327.recBytes;
    // Reduce debug output - only print occasionally
    // Serial.print("payloadLen: ");
    // Serial.println(payloadLen);

    processPayload(payload, payloadLen, results);

    int BattMinTraw = convertToInt(results.frames[2], 6, 1);  //specify frame#, starting Byte(o in TorquePro) and # of bytes required
    if (BattMinTraw > 127) {                                  //conversition for negative value[
      BattMinT = -1 * (256 - BattMinTraw);
    } else {
      BattMinT = BattMinTraw;
    }
    int BattMaxTraw = convertToInt(results.frames[2], 5, 1);  //specify frame#, starting Byte(o in TorquePro) and # of bytes required
    if (BattMaxTraw > 127) {                                  //conversition for negative value[
      BattMaxT = -1 * (256 - BattMaxTraw);
    } else {
      BattMaxT = BattMaxTraw;
    }
    // Old 12V battery voltage extraction (from different source for comparison)
    AuxBattV_old = convertToInt(results.frames[4], 6, 1) * 0.1;
    BATTv = convertToInt(results.frames[2], 3, 2) * 0.1;
    int CurrentByte1 = convertToInt(results.frames[2], 1, 1);
    int CurrentByte2 = convertToInt(results.frames[2], 2, 1);
    if (CurrentByte1 > 127) {  // the most significant bit is the sign bit so need to calculate commplement value[ if true
      BATTc = -1 * (((255 - CurrentByte1) * 256) + (256 - CurrentByte2)) * 0.1;
    } else {
      BATTc = ((CurrentByte1 * 256) + CurrentByte2) * 0.1;
    }
    CEC = convertToInt(results.frames[6], 1, 4) * 0.1;
    CED = ((convertToInt(results.frames[6], 5, 3) << 8) + convertToInt(results.frames[7], 1, 1)) * 0.1;
    CCC = ((convertToInt(results.frames[4], 7, 1) << 24) + convertToInt(results.frames[5], 1, 3)) * 0.1;
    CDC = convertToInt(results.frames[5], 4, 4) * 0.1;
    BmsSoC = convertToInt(results.frames[1], 2, 1) * 0.5;
    MAXcellv = convertToInt(results.frames[3], 7, 1) * 0.02;
    MAXcellvNb = convertToInt(results.frames[4], 1, 1);
    MINcellv = convertToInt(results.frames[4], 2, 1) * 0.02;
    MINcellvNb = convertToInt(results.frames[4], 3, 1);
    OPtimemins = convertToInt(results.frames[7], 2, 4) * 0.01666666667;
    OPtimehours = OPtimemins * 0.01666666667;
    //FrontMotor = convertToInt(results.frames[8], 2, 2);
    //RearMotor = convertToInt(results.frames[8], 4, 2);   
    
  }
  if (BmsSoC > 0) {
    OBDscanning = true;
  }  
  
  UpdateNetEnergy();  
  
  if (pid_counter > 6){
    pid_counter = 0;
  }
  else if (OBDscanning){
  // Read remaining PIDs only if BMS relay is ON
    switch (pid_counter) {
      case 1:
  
        button();
        myELM327.sendCommand("AT SH 7E4");  // Set Header for BMS
        //myELM327.queryPID((char*)"220105");  
        if (myELM327.queryPID("220105")) {  // Service and Message PID = hex 22 0105 => dec 34, 261
          delayMicroseconds(10); // 10 uSec delay between PID queries
        //if (myELM327.nb_rx_state ==  ELM_SUCCESS) {
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
          if (HeaterRaw > 127) {  //conversition for negative value[
            Heater = -1 * (256 - HeaterRaw);
          } else {
            Heater = HeaterRaw;
          }
        }
        break;
  
      case 2:
  
        button();
        myELM327.sendCommand("AT SH 7E4");  // Set Header for BMS
        //myELM327.queryPID((char*)"220106");               
        if (myELM327.queryPID("220106")) {  // Service and Message PID = hex 22 0106 => dec 34, 262
          delayMicroseconds(10); // 10 uSec delay between PID queries
        //if (myELM327.nb_rx_state ==  ELM_SUCCESS) {
          char* payload = myELM327.payload;
          size_t payloadLen = myELM327.recBytes;
  
          processPayload(payload, payloadLen, results);
          int COOLtempRaw = convertToInt(results.frames[1], 2, 1) * 0.01;  // Cooling water temperature
          if (COOLtempRaw > 127) {                                         //conversition for negative value[
            COOLtemp = -1 * (256 - COOLtempRaw);
          } else {
            COOLtemp = COOLtempRaw;
          }
        }
        break;
   
      case 3:
        
        button();
        myELM327.sendCommand("AT SH 7C6");  // Set Header for CLU Cluster Module
        //myELM327.queryPID((char*)"22B002");
        if (myELM327.queryPID("22B002")) {
          delayMicroseconds(10); // 10 uSec delay between PID queries
        //if (myELM327.nb_rx_state ==  ELM_SUCCESS) {  
          char* payload = myELM327.payload;
          size_t payloadLen = myELM327.recBytes;
  
          processPayload(payload, payloadLen, results);
          Odometer = convertToInt(results.frames[1], 4, 3);
        }
        break;
  
      case 4:
  
        button();
        myELM327.sendCommand("AT SH 7B3");  //Set Header Aircon   
        //myELM327.queryPID((char*)"220100");  
        if (myELM327.queryPID("220100")) {  // Service and Message PID
          delayMicroseconds(10); // 10 uSec delay between PID queries
        //if (myELM327.nb_rx_state ==  ELM_SUCCESS) {  
          char* payload = myELM327.payload;
          size_t payloadLen = myELM327.recBytes;
  
          processPayload(payload, payloadLen, results);
          INDOORtemp = (((convertToInt(results.frames[1], 3, 1)) * 0.5) - 40);
          OUTDOORtemp = (((convertToInt(results.frames[1], 4, 1)) * 0.5) - 40);
          Speed = (convertToInt(results.frames[4], 6, 1));
          Integrat_speed();
          if (BATTc < 0 && Speed == 0) {
            Charging = true;
          }
          if (Speed > 0) {
            SpdSelect = 'D';    
          }
          else {   
            SpdSelect = 'P';    
          }
        }
        break;
  
      case 5:
  
        button();
        myELM327.sendCommand("AT SH 7A0");  //Set BCM Header   
        //myELM327.queryPID((char*)"22C00B");     
        if (myELM327.queryPID("22C00B")) {  // Service and Message PID
          delayMicroseconds(10); // 10 uSec delay between PID queries
        //if (myELM327.nb_rx_state ==  ELM_SUCCESS) {  
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
        break;
        
      case 6:
        button();
        myELM327.sendCommand("AT SH 7E5");  // Set Header for BMS
        //myELM327.queryPID((char*)"22E021");
        if (myELM327.queryPID("22E011")) {
          delayMicroseconds(10); // 10 uSec delay between PID queries   
        //if (myELM327.nb_rx_state ==  ELM_SUCCESS) {  
          char* payload = myELM327.payload; 
          size_t payloadLen = myELM327.recBytes;

          processPayload(payload, payloadLen, results);
          
          // Extract 12V battery SoC (Frame 3, Byte 4)
          AuxBatt_SoC = convertToInt(results.frames[3], 4, 1);
          
          // Extract 12V battery voltage (Frame 2 Byte 7 + Frame 3 Byte 1, 16-bit value / 1000)
          AuxBattV = ((convertToInt(results.frames[2], 7, 1) << 8) + convertToInt(results.frames[3], 1, 1)) / 1000.0;
                    
          // Auto-switch to Batt. Info screen if AuxBatt_SoC falls below 70% (only once)
          if (AuxBatt_SoC < 70 && !lowBattAlertShown) {
            screenNbr = 1;
            DrawBackground = true;
            lowBattAlertShown = true;
            Serial.printf("Low 12V battery SoC (%d%%), switching to Batt. Info screen\n", AuxBatt_SoC);
          }
          // Reset alert flag when battery recovers above 75% (hysteresis)
          if (AuxBatt_SoC >= 75) {
            lowBattAlertShown = false;
          }
        }
        pid_counter = 0;
        data_ready = true;  // after all PIDs have been read, turn on flag for valid value from OBD2
        break;


    }


  /////// Miscellaneous calculations /////////

  Power = (BATTv * BATTc) * 0.001;
  Integrat_power();
  Integrat_current();  

    if (!ResetOn) {  // On power On, wait for current trip value to be re-initialized before executing the next lines of code
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
      
      if (PrevSoC != SoC) {  // perform "used_kWh" and "left_kWh" when SoC changes
        if (InitRst) {       // On Button Trip reset, initial kWh calculation
          Serial.print("1st Reset");
          initscan = true;
          record_code = 2;
          // Check if files need archiving on trip reset event
          archiveAndRecreateFiles(false);
          reset_trip();        
          kWh_corr = 0;
          PrevSoC = SoC;
          Prev_kWh = Net_kWh;
          used_kWh = calc_kwh(SoC, InitSoC);          
          left_kWh = calc_kwh(0, SoC) * degrad_ratio;          
          InitRst = false;
        }
        else {  // kWh calculation when the Initial reset is not active
          // After a Trip Reset, perform a new reset if SoC changed without a Net_kWh increase (in case SoC was just about to change when the reset was performed)
          if (((acc_energy < 0.25) && (PrevSoC > SoC)) || ((SoC > 98.5) && ((PrevSoC - SoC) > 0.5))) {                      
            if ((acc_energy < 0.25) && (PrevSoC > SoC)) {
              initscan = true;
              mem_energy = acc_energy;
              mem_PrevSoC = PrevSoC;
              mem_SoC = SoC;
              record_code = 3;
              // Check if files need archiving on second reset event
              archiveAndRecreateFiles(false);
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
              // Check if files need archiving on record_code 4 event
              archiveAndRecreateFiles(false);
            }
  
          } 
          else if (((PrevSoC > SoC) && ((PrevSoC - SoC) < 1)) || ((PrevSoC < SoC) && (Charging))) {  // Normal kWh calculation when SoC decreases and exception if a 0 gitch in SoC data
            kWh_corr = 0;
            used_kWh = calc_kwh(SoC, InitSoC);            
            left_kWh = calc_kwh(0, SoC) * degrad_ratio;            
            // Only set SoC_decreased flag when SoC actually decreases (not when charging)
            if (PrevSoC > SoC) {
              SoC_decreased = true;
            }
            PrevSoC = SoC;
            Prev_kWh = Net_kWh;
            kWh_update = true;            
            Integrat_power();                       
  
            if ((used_kWh >= 3) && (SpdSelect == 'D')) {  // Wait till 4 kWh has been used to start calculating ratio to have a better accuracy              
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
            //full_kWh = Net_kWh + (start_kWh + left_kWh) * degrad_ratio;            
          }
        }
  
      } 
      else if ((Prev_kWh < Net_kWh) && !kWh_update) {  // since the SoC has only 0.5 kWh resolution, when the Net_kWh increases, a 0.1 kWh is added to the kWh calculation to interpolate until next SoC change.
        kWh_corr += (Net_kWh - Prev_kWh);
        used_kWh = calc_kwh(PrevSoC, InitSoC) + kWh_corr;        
        left_kWh = (calc_kwh(0, PrevSoC) * degrad_ratio) - kWh_corr;        
        Prev_kWh = Net_kWh;
        corr_update = true;
      } /*
      else if ((Prev_kWh > Net_kWh) && !kWh_update) {  // since the SoC has only 0.5 kWh resolution, when the Net_kWh decreases, a 0.1 kWh is substracted to the kWh calculation to interpolate until next SoC change.
        kWh_corr -= (Prev_kWh - Net_kWh);
        used_kWh = calc_kwh(PrevSoC, InitSoC) + kWh_corr;        
        left_kWh = (calc_kwh(0, PrevSoC) * degrad_ratio) - kWh_corr;        
        Prev_kWh = Net_kWh;
        corr_update = true;
      }*/
  
      if (sendIntervalOn) {  // add condition so "kWh_corr" is not triggered before a cycle after a "kWh_update" when wifi is not connected
        if (kWh_update) {
          Prev_kWh = Net_kWh;
          kWh_update = false;  // reset kWh_update so correction logic starts again
        }
        if (corr_update) {
          corr_update = false;  // reset corr_update since it's not being recorded
        }
        sendIntervalOn = false;
      }
  
      if ((LastSoC + 1) < SoC && (Power > 0) && (LastSoC != 0)) {  // reset trip after a battery recharge      
        mem_Power = Power;
        mem_LastSoC = LastSoC;
        mem_SoC = SoC;
        initscan = true;
        record_code = 1;
        // Check if files need archiving on battery recharge event
        archiveAndRecreateFiles(false);
        reset_trip();      
      }  
      
      //Estleft_kWh = left_kWh * degrad_ratio;
  
      if ((millis() - RangeCalcTimer) > RangeCalcUpdate){
        RangeCalc();
        RangeCalcTimer = millis();
      }
      
      //if (OBDscanning) {
      //  EnergyTOC();
      //}
  
      if (Max_Pwr < 100 && (Max_Pwr < (Power + 20)) && !Power_page) {  //select the Max Power page if Power+20kW exceed Max_Pwr when Max_Pwr is lower then 100kW.
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

//--------------------------------------------------------------------------------------------
//                   Net Energy Calculation Function
//--------------------------------------------------------------------------------------------

/*//////Function to calculate Discharge Energy Since last reset //////////*/

void UpdateNetEnergy() {

  if (InitCED == 0) {  //if discharge value[ have been reinitiate to 0 then
    InitCED = CED;     //initiate to current CED for initial CED value[ and
    InitSoC = SoC;     //initiate to current CED for initial SoC value[ and
    CurrInitCED = CED;
  }
  if (InitCDC == 0) {
    InitCDC = CDC;
  }
  if (InitCEC == 0) {  //if charge value[ have been reinitiate to 0 then
    InitCEC = CEC;     //initiate to current CEC for initial CEC value[
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

//--------------------------------------------------------------------------------------------
//                   Net Energy based on Power integration Function
//--------------------------------------------------------------------------------------------

/*//////Function to calculate Energy by power integration since last reset //////////*/

void Integrat_power() {
  
  pwr_interval = (millis() - integrateP_timer) / 1000;
  integrateP_timer = millis();
  int_pwr = Power * pwr_interval / 3600;
  acc_energy += int_pwr;
  if (int_pwr < 0) {
    acc_regen += -(int_pwr);
  }
}

//--------------------------------------------------------------------------------------------
//                   Net Energy Charge based on Power integration Function
//--------------------------------------------------------------------------------------------

/*//////Function to calculate Energy Charge by current integration since last reset //////////*/

void Integrat_current() {
  
  curr_interval = (millis() - integrateI_timer) / 1000;
  integrateI_timer = millis();
  int_curr = BATTc * curr_interval / 3600;
  acc_Ah += int_curr;
}

//--------------------------------------------------------------------------------------------
//                   Net Energy for last N km Function
//--------------------------------------------------------------------------------------------

/*//////Function to calculate Energy over last N km //////////*/

void N_km_energy(float latest_energy) {
  energy_array[energy_array_index] = latest_energy;
  energy_array_index++;
  if (energy_array_index > N_km) {
    energy_array_index = 0;
  }
  span_energy = latest_energy - energy_array[energy_array_index];  
}

//--------------------------------------------------------------------------------------------
//                   Distance based on Speed integration Function
//--------------------------------------------------------------------------------------------

/*//////Function to calculate distance by speed integration over time //////////*/

void Integrat_speed() {
  speed_interval = (millis() - init_speed_timer) / 1000;
  init_speed_timer = millis();
  int_speed = Speed * speed_interval / 3600;
  distance += (int_speed * 1);  // need to apply a 1.013 to get correct distance
}

//--------------------------------------------------------------------------------------------
//                   Kilometer Range Calculation Function
//--------------------------------------------------------------------------------------------

void RangeCalc() {

  if ((prev_odo != CurrTripOdo) && (distance < 0.9)) {
    if (TripOdo < 2) {
      InitOdo = Odometer - distance;  // correct initial odometer value using speed integration if odometer changes within 0.9km
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

  if (Trip_dist >= (N_km + 1)) {  // wait 11km before calculating consommation for last 10km
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


//--------------------------------------------------------------------------------------------
//                   Energy TOC Function
//--------------------------------------------------------------------------------------------

/*//////Function to record time on condition  //////////*/
/*
void EnergyTOC() {
  if (OUTDOORtemp >= 25) {
    acc_kWh_25 = acc_kWh_25 + (acc_energy - last_energy);
    acc_time_25 = acc_time_25 + (CurrOPtime - last_time);
    acc_dist_25 = acc_dist_25 + (distance - last_odo);
  } else if ((OUTDOORtemp < 25) && (OUTDOORtemp >= 10)) {
    acc_kWh_10 = acc_kWh_10 + (acc_energy - last_energy);
    acc_time_10 = acc_time_10 + (CurrOPtime - last_time);
    acc_dist_10 = acc_dist_10 + (distance - last_odo);
  } else if ((OUTDOORtemp < 10) && (OUTDOORtemp >= 0)) {
    acc_kWh_0 = acc_kWh_0 + (acc_energy - last_energy);
    acc_time_0 = acc_time_0 + (CurrOPtime - last_time);
    acc_dist_0 = acc_dist_0 + (distance - last_odo);
  } else if ((OUTDOORtemp < 0) && (OUTDOORtemp >= -10)) {
    acc_kWh_m10 = acc_kWh_m10 + (acc_energy - last_energy);
    acc_time_m10 = acc_time_m10 + (CurrOPtime - last_time);
    acc_dist_m10 = acc_dist_m10 + (distance - last_odo);
  } else if ((OUTDOORtemp < -10) && (OUTDOORtemp >= -20)) {
    acc_kWh_m20 = acc_kWh_m20 + (acc_energy - last_energy);
    acc_time_m20 = acc_time_m20 + (CurrOPtime - last_time);
    acc_dist_m20 = acc_dist_m20 + (distance - last_odo);
  } else if (OUTDOORtemp < -20) {
    acc_kWh_m20p = acc_kWh_m20p + (acc_energy - last_energy);
    acc_time_m20p = acc_time_m20p + (CurrOPtime - last_time);
    acc_dist_m20p = acc_dist_m20p + (distance - last_odo);
  }
  last_energy = acc_energy;
  last_time = CurrOPtime;
  last_odo = distance;
}*/

//--------------------------------------------------------------------------------------------
//                   Function to calculate energy between two SoC value
//--------------------------------------------------------------------------------------------
/*
double Interpolate(double xvalue[], double yvalue[], int numvalue, double pointX, bool trim = true) {
  if (trim) {
    if (pointX <= xvalue[0]) return yvalue[0];
    if (pointX >= xvalue[numvalue - 1]) return yvalue[numvalue - 1];
  }

  auto i = 0;
  if (pointX <= xvalue[0]) i = 0;
  else if (pointX >= xvalue[numvalue - 1]) i = numvalue - 1;
  else
    while (pointX >= xvalue[i + 1]) i++;
  if (pointX == xvalue[i + 1]) return yvalue[i + 1];

  auto t = (pointX - xvalue[i]) / (xvalue[i + 1] - xvalue[i]);
  t = t * t * (3 - 2 * t);
  return yvalue[i] * (1 - t) + yvalue[i + 1] * t;
}
*/
/*
float calc_kwh(float min_SoC, float max_SoC) {
  
  float fullBattCapacity = 77.4 * 0.97 * degrad_ratio;
  float SoC100 = 100;
  double b = 0.66;
  double a = (fullBattCapacity - (b * SoC100)) / pow(SoC100,2);  
  
  float max_kwh = a * pow(max_SoC,2) + b * max_SoC;
  float min_kwh = a * pow(min_SoC,2) + b * min_SoC;
  float return_kwh;
    
  return_kwh = max_kwh - min_kwh;
  return return_kwh;
}*/

float calc_kwh(float min_SoC, float max_SoC) {
  
  a = (fullBattCapacity - (b * SoC100)) / pow(SoC100,2);  
  
  max_kwh = a * pow(max_SoC,2) + b * max_SoC;
  min_kwh = a * pow(min_SoC,2) + b * min_SoC;
      
  return_kwh = max_kwh - min_kwh;
  return return_kwh;
}

//----------------------------------------------------------------------------------------
//        Save data to SD card in CSV format
//----------------------------------------------------------------------------------------
bool saveToSD(const char* timestamp) {
  Serial.print("saveToSD called, sdCardAvailable: ");
  Serial.println(sdCardAvailable);
  
  if (!OBD2connected) {
    Serial.println("OBD2 not connected, skipping SD save");
    return false;
  }
  
  if (!sdCardAvailable) {
    Serial.println("SD card not available, skipping save");
    return false;
  }
  
  Serial.print("Opening file: ");
  Serial.println(csvFilename);
  
  File file = SD.open(csvFilename, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open SD file for writing");
    return false;
  }
  
  Serial.println("File opened, writing data...");
  
  // Write tab-separated row
  file.print(timestamp); file.print("\t");
  file.print(SoC); file.print("\t");
  file.print(BmsSoC); file.print("\t");
  file.print(Power); file.print("\t");
  file.print(TripOdo); file.print("\t");
  file.print(BattMinT); file.print("\t");
  file.print(BattMaxT); file.print("\t");
  file.print(Heater); file.print("\t");
  file.print(OUTDOORtemp); file.print("\t");
  file.print(INDOORtemp); file.print("\t");
  file.print(Net_kWh); file.print("\t");
  file.print(Net_kWh2); file.print("\t");
  file.print(acc_energy); file.print("\t");
  file.print(Net_Ah); file.print("\t");
  file.print(acc_Ah); file.print("\t");
  file.print(EstFull_Ah); file.print("\t");
  file.print(Max_Pwr); file.print("\t");
  file.print(Max_Reg); file.print("\t");
  file.print(MAXcellv); file.print("\t");
  file.print(MINcellv); file.print("\t");
  file.print(MAXcellvNb); file.print("\t");
  file.print(MINcellvNb); file.print("\t");
  file.print(BATTv); file.print("\t");
  file.print(BATTc); file.print("\t");
  file.print(AuxBattV); file.print("\t");
  file.print(AuxBatt_SoC); file.print("\t");
  file.print(CEC); file.print("\t");
  file.print(CED); file.print("\t");
  file.print(CDC); file.print("\t");
  file.print(CCC); file.print("\t");
  file.print(SOH); file.print("\t");
  file.print(used_kWh); file.print("\t");
  file.print(left_kWh); file.print("\t");
  file.print(full_kWh); file.print("\t");
  file.print(start_kWh); file.print("\t");
  file.print(PID_kWhLeft); file.print("\t");
  file.print(degrad_ratio); file.print("\t");
  file.print(InitSoC); file.print("\t");
  file.print(LastSoC); file.print("\t");
  file.print(PIDkWh_100); file.print("\t");
  file.print(kWh_100km); file.print("\t");
  file.print(span_kWh_100km); file.print("\t");
  file.print(Trip_dist); file.print("\t");
  file.print(distance); file.print("\t");
  file.print(Speed); file.print("\t");
  file.print(Odometer); file.print("\t");
  file.print(OPtimemins); file.print("\t");
  file.print(TripOPtime); file.print("\t");
  file.print(CurrOPtime); file.print("\t");
  file.print(TireFL_P); file.print("\t");
  file.print(TireFR_P); file.print("\t");
  file.print(TireRL_P); file.print("\t");
  file.print(TireRR_P); file.print("\t");
  file.print(TireFL_T); file.print("\t");
  file.print(TireFR_T); file.print("\t");
  file.print(TireRL_T); file.print("\t");
  file.print(TireRR_T); file.print("\t");
  file.print(nbr_saved); file.print("\t");
  file.print(AuxBattV_old);
  
  file.println();  // End the line
  file.close();
  Serial.println("Data saved to SD card");
  
  // Trigger LED flash for SD card save
  sdCardSaved = true;
  SDsaveTimer = millis();
  
  return true;
}

bool saveEventCodeToSD(const char* timestamp) {
  // Only save event codes when initscan is true or record_code is set
  if (!initscan && record_code == 0) {
    return false;  // No event code to save
  }
  
  if (!OBD2connected || !sdCardAvailable) {
    return false;
  }
  
  Serial.println("Saving event code to SD card");
  
  File file = SD.open(csvFilename, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open SD file for event code");
    return false;
  }
  
  // Write event code row with timestamp
  file.print(timestamp); file.print("\t");
  
  if (record_code == 0) {
    // No reset, only header required (ESP32 power reboot)
    file.print(EventCode0);
    initscan = false;
  } else {
    switch (record_code) {
    case 1:
      file.print(EventCode1); file.print("\t");
      file.print(Mess_SoC); file.print("\t"); file.print(mem_SoC); file.print("\t");
      file.print(Mess_LastSoC); file.print("\t"); file.print(mem_LastSoC);
      break;
    case 2:
      file.print(EventCode2);
      break;
    case 3:
      file.print(EventCode3); file.print("\t");
      file.print(Mess_SoC); file.print("\t"); file.print(mem_SoC); file.print("\t");
      file.print(Mess_PrevSoC); file.print("\t"); file.print(mem_PrevSoC); file.print("\t");
      file.print(Mess_Energy); file.print("\t"); file.print(mem_energy);
      break;
    case 4:
      file.print(EventCode4); file.print("\t");
      file.print(Mess_SoC); file.print("\t"); file.print(mem_SoC); file.print("\t");
      file.print(Mess_PrevSoC); file.print("\t"); file.print(mem_PrevSoC);
      break;
    case 5:
      file.print(EventCode5); file.print("\t");
      file.print(Mess_SoC); file.print("\t"); file.print(mem_SoC); file.print("\t");
      file.print(Mess_Power); file.print("\t"); file.print(Power);
      break;
    case 6:
      file.print(EventCode6); file.print("\t");
      file.print(Mess_Power); file.print("\t"); file.print(Power); file.print("\t");
      file.print(Mess_SD); file.print("\t"); file.print(shutdown_timer);
      break;
    default:
      file.print(EventCode0);
      break;
    }
    initscan = true;  // Set initscan for event codes 1-6
  }
  
  file.println();  // End the line
  file.close();
  Serial.println("Event code saved to SD card");
  
  // Reset record_code after saving (same as Google Sheets logic)
  record_code = 0;
  
  return true;
}

bool saveEventCodeToSD_SoC(const char* timestamp) {
  // Only save event codes when initscan is true or record_code is set
  if (!initscan && record_code == 0) {
    return false;  // No event code to save
  }
  
  if (!OBD2connected || !sdCardAvailable) {
    return false;
  }
  
  Serial.println("Saving event code to SoC decrease file");
  
  File file = SD.open(socDecreaseFilename, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open SoC decrease file for event code");
    return false;
  }
  
  // Write event code row with timestamp
  file.print(timestamp); file.print("\t");
  
  if (record_code == 0) {
    // No reset, only header required (ESP32 power reboot)
    file.print(EventCode0);
    initscan = false;
  } else {
    switch (record_code) {
    case 1:
      file.print(EventCode1); file.print("\t");
      file.print(Mess_SoC); file.print("\t"); file.print(mem_SoC); file.print("\t");
      file.print(Mess_LastSoC); file.print("\t"); file.print(mem_LastSoC);
      break;
    case 2:
      file.print(EventCode2);
      break;
    case 3:
      file.print(EventCode3); file.print("\t");
      file.print(Mess_SoC); file.print("\t"); file.print(mem_SoC); file.print("\t");
      file.print(Mess_PrevSoC); file.print("\t"); file.print(mem_PrevSoC); file.print("\t");
      file.print(Mess_Energy); file.print("\t"); file.print(mem_energy);
      break;
    case 4:
      file.print(EventCode4); file.print("\t");
      file.print(Mess_SoC); file.print("\t"); file.print(mem_SoC); file.print("\t");
      file.print(Mess_PrevSoC); file.print("\t"); file.print(mem_PrevSoC);
      break;
    case 5:
      file.print(EventCode5); file.print("\t");
      file.print(Mess_SoC); file.print("\t"); file.print(mem_SoC); file.print("\t");
      file.print(Mess_Power); file.print("\t"); file.print(Power);
      break;
    case 6:
      file.print(EventCode6); file.print("\t");
      file.print(Mess_Power); file.print("\t"); file.print(Power); file.print("\t");
      file.print(Mess_SD); file.print("\t"); file.print(shutdown_timer);
      break;
    default:
      file.print(EventCode0);
      break;
    }
    initscan = true;  // Set initscan for event codes 1-6
  }
  
  file.println();  // End the line
  file.close();
  Serial.println("Event code saved to SoC decrease file");
  
  // Reset record_code after saving (same as Google Sheets logic)
  record_code = 0;
  
  return true;
}

void saveWiFiCredentials(String ssid, String password) {
  // Save WiFi credentials to EEPROM
  // SSID: address 72-103 (32 bytes)
  // Password: address 104-167 (64 bytes)
  // hasCustomWiFi flag: address 168 (1 byte, 0xFF = has custom WiFi)
  
  Serial.println("Saving WiFi credentials to EEPROM");
  
  // Write SSID (max 32 chars)
  for (int i = 0; i < 32; i++) {
    if (i < ssid.length()) {
      EEPROM.write(72 + i, ssid[i]);
    } else {
      EEPROM.write(72 + i, 0);  // Null terminator
    }
  }
  
  // Write Password (max 64 chars)
  for (int i = 0; i < 64; i++) {
    if (i < password.length()) {
      EEPROM.write(104 + i, password[i]);
    } else {
      EEPROM.write(104 + i, 0);  // Null terminator
    }
  }
  
  // Set flag to indicate custom WiFi is saved
  EEPROM.write(168, 0xFF);
  
  EEPROM.commit();
  Serial.println("WiFi credentials saved");
}

bool loadWiFiCredentials(char* ssid, char* password) {
  // Load WiFi credentials from EEPROM
  // Returns true if custom credentials exist and are valid, false otherwise

  // Check if custom WiFi flag is set
  if (EEPROM.read(168) != 0xFF) {
    Serial.println("No custom WiFi credentials found in EEPROM (flag not set)");
    return false;
  }

  Serial.println("Loading WiFi credentials from EEPROM");

  // Read SSID (max 16 chars)
  int ssidLen = 0;
  for (int i = 0; i < 16; i++) {
    char c = EEPROM.read(72 + i);
    if (c == 0) break;  // Null terminator
    // Check for invalid characters (non-printable or garbage)
    if (c < 32 || c > 126) {
      Serial.println("Invalid SSID data detected");
      return false;
    }
    ssid[ssidLen++] = c;
  }
  ssid[ssidLen] = '\0';

  // Validate SSID is not empty and reasonable length
  if (ssidLen == 0 || ssidLen > 16) {
    Serial.println("Invalid SSID length");
    return false;
  }

  // Read Password (max 16 chars)
  int passLen = 0;
  for (int i = 0; i < 16; i++) {
    char c = EEPROM.read(104 + i);
    if (c == 0) break;  // Null terminator
    if (c < 32 || c > 126) {
      Serial.println("Invalid password data detected");
      return false;
    }
    password[passLen++] = c;
  }
  password[passLen] = '\0';

  Serial.printf("Loaded valid SSID: %s (length: %d)\n", ssid, ssidLen);
  return true;
}

void drawWiFiScreen() {
  // Draw WiFi screen based on current wifiScreenPage state
  lcd.fillScreen(TFT_BLACK);
  lcd.setFont(&FreeSans18pt7b);
  lcd.setTextColor(TFT_WHITE);
  
  if (wifiScreenPage == 0) {
    // Network list page
    lcd.drawString("Available Networks", 160, 30);
    lcd.setFont(&FreeSans9pt7b);
    
    // Set left-aligned text for network names
    lcd.setTextDatum(TL_DATUM);
    for (int i = 0; i < wifiCount && i < 8; i++) {
      int yPos = 100 + (i * 35);
      lcd.setTextColor(TFT_CYAN);
      lcd.drawString(wifiSSIDList[i], 10, yPos - 8);
      lcd.setTextColor(TFT_LIGHTGREY);
      char rssiStr[16];
      sprintf(rssiStr, "%d dBm", wifiRSSI[i]);
      lcd.drawString(rssiStr, 235, yPos - 8);
    }
    // Restore centered text alignment
    lcd.setTextDatum(MC_DATUM);
    
    // Buttons
    lcd.fillRoundRect(10, 420, 140, 50, 10, TFT_BLUE);
    lcd.fillRoundRect(170, 420, 140, 50, 10, TFT_RED);
    lcd.setTextColor(TFT_WHITE);
    lcd.drawString("Manual", 80, 445);
    lcd.drawString("Close", 240, 445);
  }
  else if (wifiScreenPage == 1) {
    // SSID entry page
    lcd.drawString("Enter SSID", 160, 30);
    lcd.setFont(&FreeSans12pt7b);
    
    // Show entered SSID
    lcd.setTextColor(TFT_CYAN);
    String displaySSID = enteredSSID;
    if (displaySSID.length() == 0) displaySSID = "(empty)";
    lcd.drawString(displaySSID.c_str(), 160, 70);
    
    // Show SSID length
    lcd.setFont(&FreeSans9pt7b);
    lcd.setTextColor(TFT_LIGHTGREY);
    char lenStr[20];
    sprintf(lenStr, "Length: %d", (int)strlen(enteredSSID));
    lcd.drawString(lenStr, 160, 95);
    
    // Current character selection - larger display
    lcd.setTextColor(TFT_YELLOW);
    lcd.setFont(&FreeSans18pt7b);
    char currentChar = currentKeyboard[keyboardIndex % currentKeyboardSize];
    char charDisplay[2] = {currentChar, '\0'};
    lcd.drawString(charDisplay, 160, 130);
    
    // Mode selection buttons - small buttons at top
    lcd.setFont(&FreeSans9pt7b);
    // abc button
    lcd.fillRoundRect(10, 160, 90, 35, 5, (keyboardMode == 0) ? TFT_DARKGREEN : TFT_DARKGREY);
    lcd.setTextColor(TFT_WHITE);
    lcd.drawString("abc", 55, 177);
    // ABC button
    lcd.fillRoundRect(115, 160, 90, 35, 5, (keyboardMode == 1) ? TFT_DARKGREEN : TFT_DARKGREY);
    lcd.drawString("ABC", 160, 177);
    // 123 button
    lcd.fillRoundRect(220, 160, 90, 35, 5, (keyboardMode == 2) ? TFT_DARKGREEN : TFT_DARKGREY);
    lcd.drawString("123", 265, 177);
    
    // Main action buttons
    lcd.setFont(&FreeSans12pt7b);
    lcd.fillRoundRect(10, 220, 140, 50, 10, TFT_GREEN);
    lcd.fillRoundRect(170, 220, 140, 50, 10, TFT_ORANGE);
    lcd.fillRoundRect(10, 290, 140, 50, 10, TFT_BLUE);
    lcd.fillRoundRect(170, 290, 140, 50, 10, TFT_RED);
    lcd.setTextColor(TFT_WHITE);
    lcd.drawString("Add", 80, 245);
    lcd.drawString("Delete", 240, 245);
    lcd.drawString("Next", 80, 315);
    lcd.drawString("Cancel", 240, 315);
  }
  else if (wifiScreenPage == 2) {
    // Password entry page
    lcd.drawString("Enter Password", 160, 30);
    lcd.setFont(&FreeSans12pt7b);
    
    // Show password in plain text
    lcd.setTextColor(TFT_CYAN);
    String displayPwd = enteredPassword;
    if (displayPwd.length() == 0) displayPwd = "(empty)";
    lcd.drawString(displayPwd.c_str(), 160, 70);
    
    // Show password length
    lcd.setFont(&FreeSans9pt7b);
    lcd.setTextColor(TFT_LIGHTGREY);
    char lenStr[20];
    sprintf(lenStr, "Length: %d", (int)strlen(enteredPassword));
    lcd.drawString(lenStr, 160, 95);
    
    // Current character selection - larger display
    lcd.setTextColor(TFT_YELLOW);
    lcd.setFont(&FreeSans18pt7b);
    char currentChar = currentKeyboard[keyboardIndex % currentKeyboardSize];
    char charDisplay[2] = {currentChar, '\0'};
    lcd.drawString(charDisplay, 160, 130);
    
    // Mode selection buttons - small buttons at top
    lcd.setFont(&FreeSans9pt7b);
    // abc button
    lcd.fillRoundRect(10, 160, 90, 35, 5, (keyboardMode == 0) ? TFT_DARKGREEN : TFT_DARKGREY);
    lcd.setTextColor(TFT_WHITE);
    lcd.drawString("abc", 55, 177);
    // ABC button
    lcd.fillRoundRect(115, 160, 90, 35, 5, (keyboardMode == 1) ? TFT_DARKGREEN : TFT_DARKGREY);
    lcd.drawString("ABC", 160, 177);
    // 123 button
    lcd.fillRoundRect(220, 160, 90, 35, 5, (keyboardMode == 2) ? TFT_DARKGREEN : TFT_DARKGREY);
    lcd.drawString("123", 265, 177);
    
    // Main action buttons
    lcd.setFont(&FreeSans12pt7b);
    lcd.fillRoundRect(10, 220, 140, 50, 10, TFT_GREEN);
    lcd.fillRoundRect(170, 220, 140, 50, 10, TFT_ORANGE);
    lcd.fillRoundRect(10, 290, 65, 50, 10, TFT_PURPLE);
    lcd.fillRoundRect(85, 290, 65, 50, 10, TFT_PURPLE);
    lcd.fillRoundRect(170, 290, 140, 50, 10, TFT_RED);
    lcd.fillRoundRect(10, 360, 300, 50, 10, TFT_BLUE);
    lcd.setTextColor(TFT_WHITE);
    lcd.drawString("Add", 80, 245);
    lcd.drawString("Delete", 240, 245);
    lcd.setFont(&FreeSans18pt7b);
    lcd.drawString("^", 42, 315);
    lcd.drawString("v", 117, 315);
    lcd.setFont(&FreeSans12pt7b);
    lcd.drawString("Back", 240, 315);
    lcd.drawString("Connect", 160, 385);
  }
}

bool saveToSD_SoCDecrease(const char* timestamp) {
  Serial.print("saveToSD_SoCDecrease called, sdCardAvailable: ");
  Serial.println(sdCardAvailable);
  
  if (!OBD2connected) {
    Serial.println("OBD2 not connected, skipping SD save");
    return false;
  }
  
  if (!sdCardAvailable) {
    Serial.println("SD card not available, skipping save");
    return false;
  }
  
  Serial.print("Opening file: ");
  Serial.println(socDecreaseFilename);
  
  File file = SD.open(socDecreaseFilename, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open SD file for writing");
    return false;
  }
  
  Serial.println("File opened, writing SoC decrease data...");
  
  // Write tab-separated row
  file.print(timestamp); file.print("\t");
  file.print(SoC); file.print("\t");
  file.print(BmsSoC); file.print("\t");
  file.print(Power); file.print("\t");
  file.print(TripOdo); file.print("\t");
  file.print(BattMinT); file.print("\t");
  file.print(BattMaxT); file.print("\t");
  file.print(Heater); file.print("\t");
  file.print(OUTDOORtemp); file.print("\t");
  file.print(INDOORtemp); file.print("\t");
  file.print(Net_kWh); file.print("\t");
  file.print(Net_kWh2); file.print("\t");
  file.print(acc_energy); file.print("\t");
  file.print(Net_Ah); file.print("\t");
  file.print(acc_Ah); file.print("\t");
  file.print(EstFull_Ah); file.print("\t");
  file.print(Max_Pwr); file.print("\t");
  file.print(Max_Reg); file.print("\t");
  file.print(MAXcellv); file.print("\t");
  file.print(MINcellv); file.print("\t");
  file.print(MAXcellvNb); file.print("\t");
  file.print(MINcellvNb); file.print("\t");
  file.print(BATTv); file.print("\t");
  file.print(BATTc); file.print("\t");
  file.print(AuxBattV); file.print("\t");
  file.print(AuxBatt_SoC); file.print("\t");
  file.print(CEC); file.print("\t");
  file.print(CED); file.print("\t");
  file.print(CDC); file.print("\t");
  file.print(CCC); file.print("\t");
  file.print(SOH); file.print("\t");
  file.print(used_kWh); file.print("\t");
  file.print(left_kWh); file.print("\t");
  file.print(full_kWh); file.print("\t");
  file.print(start_kWh); file.print("\t");
  file.print(PID_kWhLeft); file.print("\t");
  file.print(degrad_ratio); file.print("\t");
  file.print(InitSoC); file.print("\t");
  file.print(LastSoC); file.print("\t");
  file.print(PIDkWh_100); file.print("\t");
  file.print(kWh_100km); file.print("\t");
  file.print(span_kWh_100km); file.print("\t");
  file.print(Trip_dist); file.print("\t");
  file.print(distance); file.print("\t");
  file.print(Speed); file.print("\t");
  file.print(Odometer); file.print("\t");
  file.print(OPtimemins); file.print("\t");
  file.print(TripOPtime); file.print("\t");
  file.print(CurrOPtime); file.print("\t");
  file.print(TireFL_P); file.print("\t");
  file.print(TireFR_P); file.print("\t");
  file.print(TireRL_P); file.print("\t");
  file.print(TireRR_P); file.print("\t");
  file.print(TireFL_T); file.print("\t");
  file.print(TireFR_T); file.print("\t");
  file.print(TireRL_T); file.print("\t");
  file.print(TireRR_T); file.print("\t");
  file.print(nbr_saved);
  
  file.println();  // End the line
  
  file.close();
  Serial.println("SoC decrease data saved to SD card");
  
  return true;
}

//----------------------------------------------------------------------------------------
//        Google Sheets functionality has been removed
//        SD card logging remains active
//----------------------------------------------------------------------------------------

/*////////////// Full Trip Reset ///////////////// */

void reset_trip() {  //Overall trip reset. Automatic if the car has been recharged to the same level as previous charge or if left button is pressed for more then 3 secondes

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
  //full_kWh = Net_kWh + (start_kWh + left_kWh) * degrad_ratio;  
  EEPROM.writeFloat(52, acc_energy);
  EEPROM.writeFloat(0, InitRemain_kWh);  
  EEPROM.writeFloat(4, InitCED);   //save initial CED to Flash memory
  EEPROM.writeFloat(8, InitCEC);   //save initial CEC to Flash memory
  EEPROM.writeFloat(12, InitSoC);  //save initial SoC to Flash memory
  EEPROM.writeFloat(16, previous_kWh);  
  EEPROM.writeFloat(20, InitOdo);  //save initial Odometer to Flash memory
  EEPROM.writeFloat(24, InitCDC);  //save initial CDC to Flash memory
  EEPROM.writeFloat(28, InitCCC);  //save initial CCC to Flash memory
  
  EEPROM.commit();
  Serial.println("value saved to EEPROM");
}

/*////////////// Current Trip Reset ///////////////// */

void ResetCurrTrip() {  // when the car is turned On, current trip value are resetted.

  if (ResetOn && (SoC > 1) && (Odometer > 1) && (CED > 1) && data_ready) {  // ResetOn condition might be enough, might need to update code...
    integrateP_timer = millis();
    integrateI_timer = millis();
    RangeCalcTimer = millis();
    read_timer = millis();
    SDsaveTimer = millis();    
    CurrInitAccEnergy = acc_energy;
    CurrInitCED = CED;
    CurrInitCEC = CEC;
    CurrInitOdo = Odometer;
    //CurrInitSoC = SoC;
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
    //full_kWh = Net_kWh + (start_kWh + left_kWh) * degrad_ratio;    
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

/*//////Function to save selected BLE device to EEPROM //////////*/


void saveBLEDevice(const char* deviceName) {
  char deviceBuffer[32];
  memset(deviceBuffer, 0, 32);
  strncpy(deviceBuffer, deviceName, 31);
  deviceBuffer[31] = '\0';
  for (int i = 0; i < 32; i++) {
    EEPROM.write(169 + i, deviceBuffer[i]);
  }
  EEPROM.commit();
  Serial.printf("BLE device saved to EEPROM: %s\n", deviceBuffer);
}

/*////// Draw BLE Device Selection UI with pagination //////////*/

void drawBLEDeviceSelection() {
  lcd.fillScreen(TFT_BLACK);
  lcd.setFont(&FreeSans18pt7b);
  lcd.setTextColor(TFT_WHITE);
  lcd.drawString("Select OBD2 Device", 160, 40);
  
  lcd.setFont(&FreeSans9pt7b);
  lcd.setTextColor(TFT_LIGHTGREY);
  
  if (bleDeviceCount == 0) {
    lcd.drawString("No devices found", 160, 90);
  } else {
    lcd.drawString("Tap device to select:", 160, 90);
  }
  
  // Calculate pagination: 4 devices per page
  const int devicesPerPage = 4;
  int totalPages = (bleDeviceCount + devicesPerPage - 1) / devicesPerPage;
  if (totalPages == 0) totalPages = 1;
  int startIdx = bleDevicePageNumber * devicesPerPage;
  int endIdx = min(startIdx + devicesPerPage, bleDeviceCount);
  
  // Display devices for current page at y=120, 165, 210, 255
  lcd.setFont(&FreeSans12pt7b);
  const int deviceYPositions[] = {120, 165, 210, 255};
  for (int i = 0; i < devicesPerPage && (startIdx + i) < bleDeviceCount; i++) {
    int deviceIdx = startIdx + i;
    int yPos = deviceYPositions[i];
    bool isSelected = (strncmp(bleDeviceList[deviceIdx], selectedBLEDevice, 20) == 0);
    lcd.fillRoundRect(20, yPos, 280, 40, 8, isSelected ? TFT_GREEN : TFT_DARKGREY);
    lcd.setTextColor(TFT_WHITE);
    lcd.drawString(bleDeviceList[deviceIdx], 160, yPos + 20);
  }
  
  // "Use Default" button at y=300
  bool defaultSelected = (strncmp(selectedBLEDevice, "IOS-Vlink", 20) == 0);
  lcd.fillRoundRect(20, 300, 280, 40, 8, defaultSelected ? TFT_GREEN : TFT_BLUE);
  lcd.setTextColor(TFT_WHITE);
  lcd.setFont(&FreeSans9pt7b);
  lcd.drawString("Use Default (IOS-Vlink)", 160, 320);
  
  // PREV/NEXT buttons at y=350-390 (only if multiple pages)
  if (totalPages > 1) {
    // PREV button
    lcd.fillRoundRect(20, 350, 135, 40, 10, bleDevicePageNumber > 0 ? TFT_BLUE : TFT_DARKGREY);
    lcd.setTextColor(TFT_WHITE);
    lcd.drawString("PREV", 87, 370);
    
    // NEXT button
    lcd.fillRoundRect(165, 350, 135, 40, 10, bleDevicePageNumber < totalPages - 1 ? TFT_BLUE : TFT_DARKGREY);
    lcd.drawString("NEXT", 232, 370);
  }
  
  // Bottom buttons at y=400-450: SCAN, BACK, OK
  lcd.fillRoundRect(20, 400, 90, 50, 10, TFT_ORANGE);
  lcd.setTextColor(TFT_WHITE);
  lcd.drawString("SCAN", 65, 425);
  
  lcd.fillRoundRect(115, 400, 90, 50, 10, TFT_RED);
  lcd.drawString("BACK", 160, 425);
  
  lcd.fillRoundRect(210, 400, 90, 50, 10, TFT_GREEN);
  lcd.drawString("OK", 255, 425);
}

/*//////Function to scan for available BLE devices //////////*/

void scanBLEDevices() {
  bleScanning = true;
  bleDeviceCount = 0;
  
  Serial.println("Scanning for BLE devices...");
  
  // Deinitialize BLE if already initialized to avoid conflicts
  BLEDevice::deinit(false);
  delay(100);
  
  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(nullptr);
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
  
  Serial.println("Starting BLE scan for 8 seconds...");
  BLEScanResults foundDevices = pBLEScan->start(4, false);
  
  Serial.printf("Scan complete. Found %d devices total\n", foundDevices.getCount());
  
  // Process all found devices
  for (int i = 0; i < foundDevices.getCount() && bleDeviceCount < 10; i++) {
    BLEAdvertisedDevice device = foundDevices.getDevice(i);
    String deviceName = String(device.getName().c_str());
    String deviceAddr = String(device.getAddress().toString().c_str());
    
    Serial.printf("Device %d: Name='%s' Address=%s RSSI=%d\n", 
                  i, deviceName.c_str(), deviceAddr.c_str(), device.getRSSI());
    
    // Add devices with non-empty names
    if (deviceName.length() > 0) {
      strncpy(bleDeviceList[bleDeviceCount], deviceName.c_str(), 20);
      bleDeviceList[bleDeviceCount][20] = '\0';
      Serial.printf("  -> Added to list as #%d\n", bleDeviceCount);
      bleDeviceCount++;
    } else {
      Serial.printf("  -> Skipped (no name)\n");
    }
  }
  
  pBLEScan->clearResults();
  
  // Deinitialize after scan to avoid conflicts with connection
  BLEDevice::deinit(false);
  delay(100);
  
  bleScanning = false;
  Serial.printf("Scan complete. Found %d named devices.\n", bleDeviceCount);
  
  if (bleDeviceCount == 0) {
    Serial.println("WARNING: No named BLE devices found!");
    Serial.println("Make sure your OBD2 device is powered on and in range.");
  }
}

/*//////Function to save some variable before turn off the car //////////*/

void save_lost(char selector) {
  if (selector == 'D' && !DriveOn) {
    DriveOn = true;
    SpdSelectTimer = millis();
  }
  if ((selector == 'P' || selector == 'N') && (DriveOn) && SoC > 0 && ((millis() - SpdSelectTimer) > 90000)) {  // when the selector is set to Park or Neutral, some value are saved to be used the next time the car is started
    DriveOn = false;
        
    nbr_saved += 1;
    //EEPROM.writeFloat(0, InitRemain_kWh);
    EEPROM.writeFloat(16, previous_kWh);    
    EEPROM.writeFloat(32, degrad_ratio);
    Serial.println("new_lost saved to EEPROM");
    EEPROM.writeFloat(36, PIDkWh_100);  //save actual kWh/100 in Flash memory
    EEPROM.writeFloat(40, Wifi_select);
    EEPROM.writeFloat(44, TripOPtime);  //save initial trip time to Flash memory
    EEPROM.writeFloat(48, kWh_corr);    //save cummulative kWh correction (between 2 SoC values) to Flash memory
    EEPROM.writeFloat(52, acc_energy);
    EEPROM.writeFloat(56, SoC);
    EEPROM.writeFloat(60, nbr_saved);
    EEPROM.writeFloat(64, acc_Ah);
    //EEPROM.writeFloat(68, acc_kWh_25);
    //EEPROM.writeFloat(72, acc_kWh_10);
    //EEPROM.writeFloat(76, acc_kWh_0);
    //EEPROM.writeFloat(80, acc_kWh_m10);
    //EEPROM.writeFloat(84, acc_kWh_m20);
    //EEPROM.writeFloat(88, acc_kWh_m20p);
    //EEPROM.writeFloat(92, acc_time_25);
    //EEPROM.writeFloat(96, acc_time_10);
    //EEPROM.writeFloat(100, acc_time_0);
    //EEPROM.writeFloat(104, acc_time_m10);
    //EEPROM.writeFloat(108, acc_time_m20);
    //EEPROM.writeFloat(112, acc_time_m20p);
    //EEPROM.writeFloat(116, acc_dist_25);
    //EEPROM.writeFloat(120, acc_dist_10);
    //EEPROM.writeFloat(124, acc_dist_0);
    //EEPROM.writeFloat(128, acc_dist_m10);
    //EEPROM.writeFloat(132, acc_dist_m20);
    //EEPROM.writeFloat(136, acc_dist_m20p);
    EEPROM.writeFloat(140, acc_regen);
    EEPROM.commit();
  }
}

void stop_esp() { 
  if (DriveOn && (mem_SoC > 0)) {
    //EEPROM.writeFloat(0, InitRemain_kWh);
    EEPROM.writeFloat(16, previous_kWh);
    EEPROM.writeFloat(32, degrad_ratio);
    Serial.println("new_lost saved to EEPROM");
    EEPROM.writeFloat(36, PIDkWh_100);  //save actual kWh/100 in Flash memory
    EEPROM.writeFloat(40, Wifi_select);
    EEPROM.writeFloat(44, TripOPtime);  //save initial trip time to Flash memory
    EEPROM.writeFloat(48, kWh_corr);    //save cummulative kWh correction (between 2 SoC values) to Flash memory
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

//--------------------------------------------------------------------------------------------
//                   Touch Button Handling Function
//--------------------------------------------------------------------------------------------

void button(){
  if (lcd.getTouch(&x, &y)) {
    
    // OBD2 failure screen takes priority
    if (showOBD2FailScreen && !TouchLatch) {
      TouchLatch = true;
      
      // Select Device button (60-260, 300-350)
      if (x >= 60 && x <= 260 && y >= 300 && y <= 350) {
        Serial.println("Select Device pressed");
        showOBD2FailScreen = false;
        showDeviceSelection = true;
        
        // Scan for BLE devices
        lcd.fillScreen(TFT_BLACK);
        lcd.setFont(&FreeSans18pt7b);
        lcd.setTextColor(TFT_CYAN);
        lcd.drawString("Scanning...", 160, 200);
        lcd.setFont(&FreeSans12pt7b);
        lcd.setTextColor(TFT_LIGHTGREY);
        lcd.drawString("Looking for BLE devices", 160, 250);
        
        scanBLEDevices();
        bleDevicePageNumber = 0;  // Reset to first page
        drawBLEDeviceSelection();
        
        return;
      }
      
      // Retry button (20-150, 380-440)
      if (x >= 20 && x <= 150 && y >= 380 && y <= 440) {
        Serial.println("Retry OBD2 connection");
        showOBD2FailScreen = false;
        
        // Redraw welcome screen
        drawWelcomeScreen();
        
        // Show WiFi status
        if (WiFi.status() == WL_CONNECTED) {
          lcd.setTextColor(TFT_GREEN);
          lcd.drawString("WiFi: Connected", 160, 310);
          
          // Show SSID and IP address
          lcd.setFont(&FreeSans9pt7b);
          lcd.setTextColor(TFT_DARKGREY);
          char ssidDisplay[64];
          snprintf(ssidDisplay, sizeof(ssidDisplay), "SSID: %s", WiFi.SSID().c_str());
          lcd.drawString(ssidDisplay, 160, 335);
          char ipDisplay[32];
          snprintf(ipDisplay, sizeof(ipDisplay), "IP: %s", WiFi.localIP().toString().c_str());
          lcd.drawString(ipDisplay, 160, 360);
          lcd.setFont(&FreeSans12pt7b);
        } else {
          lcd.setTextColor(TFT_RED);
          lcd.drawString("WiFi: Failed", 160, 310);
        }
        
        // Show connecting message
        lcd.setTextColor(TFT_YELLOW);
        lcd.drawString("OBD2: Connecting...", lcd.width() / 2, 410);
        Serial.println("...Connection to OBDII...");
        
        ConnectToOBD2(lcd);
        if (OBD2connected) {
          DrawBackground = true;
        }
        return;
      }
      
      // Continue button (170-300, 380-440)
      if (x >= 170 && x <= 300 && y >= 380 && y <= 440) {
        Serial.println("Continue without OBD2");
        showOBD2FailScreen = false;
        showWiFiInfo = true;
        
        // Display WiFi info screen
        lcd.fillScreen(TFT_BLACK);
        lcd.setFont(&FreeSans18pt7b);
        lcd.setTextColor(TFT_WHITE);
        lcd.drawString("WiFi Information", 160, 100);
        
        lcd.setFont(&FreeSans12pt7b);
        if (WiFi.status() == WL_CONNECTED) {
          lcd.setTextColor(TFT_GREEN);
          lcd.drawString("Status: Connected", 160, 160);
          lcd.setTextColor(TFT_CYAN);
          String ssidStr = "SSID: ";
          ssidStr += WiFi.SSID();
          lcd.drawString(ssidStr, 160, 200);
          String ipStr = "IP: ";
          ipStr += WiFi.localIP().toString();
          lcd.drawString(ipStr, 160, 240);
          char rssiStr[32];
          sprintf(rssiStr, "Signal: %d dBm", WiFi.RSSI());
          lcd.drawString(rssiStr, 160, 280);
        } else {
          lcd.setTextColor(TFT_RED);
          lcd.drawString("Status: Disconnected", 160, 200);
        }
        
        lcd.setFont(&FreeSans9pt7b);
        lcd.setTextColor(TFT_LIGHTGREY);
        lcd.drawString("Touch screen to continue", 160, 400);
        return;
      }
    }
    
    // Device selection screen handling
    if (showDeviceSelection && !TouchLatch) {
      TouchLatch = true;
      
      const int devicesPerPage = 4;
      const int deviceYPositions[] = {120, 165, 210, 255};
      int startIdx = bleDevicePageNumber * devicesPerPage;
      
      // Check if any device button was touched (4 devices per page at y=120, 165, 210, 255)
      for (int i = 0; i < devicesPerPage && (startIdx + i) < bleDeviceCount; i++) {
        int yPos = deviceYPositions[i];
        if (x >= 20 && x <= 300 && y >= yPos && y <= yPos + 40) {
          int deviceIdx = startIdx + i;
          strncpy(selectedBLEDevice, bleDeviceList[deviceIdx], 20);
          selectedBLEDevice[20] = '\0';
          Serial.printf("Selected device: %s\n", selectedBLEDevice);
          drawBLEDeviceSelection();  // Redraw entire screen with new selection
          return;
        }
      }
      
      // Check "Use Default" button at y=300
      if (x >= 20 && x <= 300 && y >= 300 && y <= 340) {
        strncpy(selectedBLEDevice, "IOS-Vlink", 20);
        selectedBLEDevice[20] = '\0';
        Serial.println("Selected default device: IOS-Vlink");
        drawBLEDeviceSelection();  // Redraw entire screen with new selection
        return;
      }
      
      // PREV button (20-155, 350-390)
      int totalPages = (bleDeviceCount + devicesPerPage - 1) / devicesPerPage;
      if (totalPages > 1 && x >= 20 && x <= 155 && y >= 350 && y <= 390) {
        if (bleDevicePageNumber > 0) {
          bleDevicePageNumber--;
          Serial.printf("Previous page: %d\n", bleDevicePageNumber);
          drawBLEDeviceSelection();
        }
        return;
      }
      
      // NEXT button (165-300, 350-390)
      if (totalPages > 1 && x >= 165 && x <= 300 && y >= 350 && y <= 390) {
        if (bleDevicePageNumber < totalPages - 1) {
          bleDevicePageNumber++;
          Serial.printf("Next page: %d\n", bleDevicePageNumber);
          drawBLEDeviceSelection();
        }
        return;
      }
      
      // Scan Again button (20-110, 400-450)
      if (x >= 20 && x <= 110 && y >= 400 && y <= 450) {
        Serial.println("Scanning for BLE devices...");
        
        // Show scanning message
        lcd.fillScreen(TFT_BLACK);
        lcd.setFont(&FreeSans18pt7b);
        lcd.setTextColor(TFT_CYAN);
        lcd.drawString("Scanning...", 160, 200);
        lcd.setFont(&FreeSans12pt7b);
        lcd.setTextColor(TFT_LIGHTGREY);
        lcd.drawString("Looking for BLE devices", 160, 250);
        
        scanBLEDevices();
        bleDevicePageNumber = 0;  // Reset to first page after scan
        drawBLEDeviceSelection();
        
        return;
      }
      
      // Back button (115-205, 400-450)
      if (x >= 115 && x <= 205 && y >= 400 && y <= 450) {
        Serial.println("Back from device selection");
        showDeviceSelection = false;
        showOBD2FailScreen = true;
        bleDevicePageNumber = 0;  // Reset page
        
        // Redraw error screen
        drawBLEConnectionError(lcd);
        return;
      }
      
      // Connect/OK button (210-300, 400-450)
      if (x >= 210 && x <= 300 && y >= 400 && y <= 450) {
        if (selectedBLEDevice[0] == '\0') {
          Serial.println("No device selected!");
          return;
        }
        Serial.printf("Attempting connection to selected device: %s\n", selectedBLEDevice);
        Serial.println("Device will be saved to EEPROM only after successful connection");
        
        showDeviceSelection = false;
        
        // Redraw welcome screen
        drawWelcomeScreen();
        
        // Show WiFi status
        if (WiFi.status() == WL_CONNECTED) {
          lcd.setTextColor(TFT_GREEN);
          lcd.drawString("WiFi: Connected", 160, 310);
          
          // Show SSID and IP address
          lcd.setFont(&FreeSans9pt7b);
          lcd.setTextColor(TFT_DARKGREY);
          char ssidDisplay[64];
          snprintf(ssidDisplay, sizeof(ssidDisplay), "SSID: %s", WiFi.SSID().c_str());
          lcd.drawString(ssidDisplay, 160, 335);
          char ipDisplay[32];
          snprintf(ipDisplay, sizeof(ipDisplay), "IP: %s", WiFi.localIP().toString().c_str());
          lcd.drawString(ipDisplay, 160, 360);
          lcd.setFont(&FreeSans12pt7b);
        } else {
          lcd.setTextColor(TFT_RED);
          lcd.drawString("WiFi: Failed", 160, 310);
        }
        
        // Show connecting message
        lcd.setTextColor(TFT_YELLOW);
        lcd.drawString("OBD2: Connecting...", lcd.width() / 2, 410);
        Serial.println("...Connection to OBDII...");
        
        ConnectToOBD2(lcd);
        if (OBD2connected) {
          DrawBackground = true;
        }
        return;
      }
    }
    
    // Normal button handling    
          
    //Button 1 test
    if ((x >= btnAon.xStart && x <= btnAon.xStart + btnAon.xWidth) && (y >= btnAon.yStart && y <= btnAon.yStart + btnAon.yHeight)) {      
      TouchTime = (millis() - initTouchTime) / 1000;      
      if (TouchTime >= 2 & !TouchLatch){
        Serial.println("Button1 Long Press - Showing IP Address");
        TouchLatch = true;
        showWiFiInfo = true;
        
        // Display WiFi info screen
        lcd.fillScreen(TFT_BLACK);
        lcd.setFont(&FreeSans18pt7b);
        lcd.setTextColor(TFT_WHITE);
        lcd.drawString("WiFi Information", 160, 100);
        
        lcd.setFont(&FreeSans12pt7b);
        if (WiFi.status() == WL_CONNECTED) {
          lcd.setTextColor(TFT_GREEN);
          lcd.drawString("Status: Connected", 160, 160);
          lcd.setTextColor(TFT_CYAN);
          String ssidStr = "SSID: ";
          ssidStr += WiFi.SSID();
          lcd.drawString(ssidStr, 160, 200);
          String ipStr = "IP: ";
          ipStr += WiFi.localIP().toString();
          lcd.drawString(ipStr, 160, 240);
          char rssiStr[32];
          sprintf(rssiStr, "Signal: %d dBm", WiFi.RSSI());
          lcd.drawString(rssiStr, 160, 280);
        } else {
          lcd.setTextColor(TFT_RED);
          lcd.drawString("Status: Disconnected", 160, 200);
        }
        
        lcd.setFont(&FreeSans9pt7b);
        lcd.setTextColor(TFT_LIGHTGREY);
        lcd.drawString("Touch screen to continue", 160, 400);
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
        
    //Button 2 test
    if ((x >= btnBon.xStart && x <= btnBon.xStart + btnBon.xWidth) && (y >= btnBon.yStart && y <= btnBon.yStart + btnBon.yHeight)) {      
      TouchTime = (millis() - initTouchTime) / 1000;
      if (TouchTime >= 2 && !TouchLatch){
        TouchLatch = true;
        Serial.println("Button2 Long Press - Opening WiFi Settings");
        
        // Perform WiFi scan first
        Serial.println("Scanning for WiFi networks...");
        WiFi.mode(WIFI_STA);  // Ensure WiFi is in station mode
        wifiCount = WiFi.scanNetworks();
        Serial.printf("Found %d networks\n", wifiCount);
        for (int i = 0; i < wifiCount && i < 20; i++) {
          strncpy(wifiSSIDList[i], WiFi.SSID(i).c_str(), 16);
          wifiSSIDList[i][16] = '\0';
          wifiRSSI[i] = WiFi.RSSI(i);
        }
        Serial.println("WiFi scan complete");
        
        // Now switch to WiFi screen (don't set DrawBackground for WiFi screen)
        screenNbr = 4;  // Switch to WiFi selection screen
        wifiScreenPage = 0;  // Start with scan list
        Btn2SetON = false;  // Keep false so we can detect button release
        Btn1SetON = false;
        Btn3SetON = false;
        
        // Draw WiFi screen using helper function
        drawWiFiScreen();
      }
      else if (!Btn2SetON && !TouchLatch){            
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

    //Button 3 test
    if ((x >= btnCon.xStart && x <= btnCon.xStart + btnCon.xWidth) && (y >= btnCon.yStart && y <= btnCon.yStart + btnCon.yHeight)) {      
      TouchTime = (millis() - initTouchTime) / 1000;
      if (TouchTime >= 2 & !TouchLatch){
        TouchLatch = true;        
        Serial.println("Button3 Long Press - Showing save confirmation");
        showSaveConfirmation = true;
        
        // Draw confirmation dialog
        lcd.fillScreen(TFT_BLACK);
        lcd.setFont(&FreeSans18pt7b);
        lcd.setTextColor(TFT_WHITE);
        lcd.drawString("Save Trip Data?", 160, 100);
        
        lcd.setFont(&FreeSans12pt7b);
        lcd.setTextColor(TFT_LIGHTGREY);
        lcd.drawString("This will save current trip", 160, 160);
        lcd.drawString("data to EEPROM", 160, 190);
        
        // Yes button
        lcd.fillRoundRect(40, 300, 100, 60, 10, TFT_GREEN);
        lcd.setTextColor(TFT_WHITE);
        lcd.drawString("YES", 90, 330);
        
        // No button
        lcd.fillRoundRect(180, 300, 100, 60, 10, TFT_RED);
        lcd.drawString("NO", 230, 330);
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

    // WiFi Screen touch handling
    if (screenNbr == 4 && !TouchLatch) {
      TouchLatch = true;
      
      if (wifiScreenPage == 0) {
        // Handle WiFi list touches
        for (int i = 0; i < wifiCount && i < 8; i++) {
          int yPos = 100 + (i * 35);
          if (y >= yPos - 15 && y <= yPos + 15) {
            selectedWiFi = i;
            strncpy(enteredSSID, wifiSSIDList[i], 16);
            enteredSSID[16] = '\0';
            wifiScreenPage = 2;  // Go to password entry
            drawWiFiScreen();
            Serial.printf("Selected WiFi: %s\n", wifiSSIDList[i]);
            return;
          }
        }
        
        // Manual button
        if (x >= 10 && x <= 150 && y >= 420 && y <= 470) {
          wifiScreenPage = 1;  // Go to manual SSID entry
            strcpy(enteredSSID, "");
          drawWiFiScreen();
          return;
        }
        
        // Close button
        if (x >= 170 && x <= 310 && y >= 420 && y <= 470) {
          screenNbr = 0;  // Return to main screen
          DrawBackground = true;
          return;
        }
      }
      else if (wifiScreenPage == 1) {
        // Mode selection buttons (y=160-195)
        if (y >= 160 && y <= 195) {
          if (x >= 10 && x <= 100) {
            // abc mode
            keyboardMode = 0;
            currentKeyboard = keyboardLower;
            currentKeyboardSize = 26;
            keyboardIndex = 0;
            drawWiFiScreen();
            return;
          }
          else if (x >= 115 && x <= 205) {
            // ABC mode
            keyboardMode = 1;
            currentKeyboard = keyboardUpper;
            currentKeyboardSize = 26;
            keyboardIndex = 0;
            drawWiFiScreen();
            return;
          }
          else if (x >= 220 && x <= 310) {
            // 123 mode
            keyboardMode = 2;
            currentKeyboard = keyboardNum;
            currentKeyboardSize = 19;
            keyboardIndex = 0;
            drawWiFiScreen();
            return;
          }
        }
        
        // SSID keyboard handling - buttons at y=220 and y=290
        if (x >= 10 && x <= 150 && y >= 220 && y <= 270) {
          // Add character (cycles through current keyboard)
          if (strlen(enteredSSID) < 16) {
            size_t len = strlen(enteredSSID);
            if (len < 16) {
              enteredSSID[len] = currentKeyboard[keyboardIndex % currentKeyboardSize];
              enteredSSID[len + 1] = '\0';
            }
            keyboardIndex++;
            drawWiFiScreen();
          }
        }
        else if (x >= 170 && x <= 310 && y >= 220 && y <= 270) {
          // Delete character
          if (strlen(enteredSSID) > 0) {
            size_t len = strlen(enteredSSID);
            if (len > 0) enteredSSID[len - 1] = '\0';
            drawWiFiScreen();
          }
        }
        else if (x >= 10 && x <= 150 && y >= 290 && y <= 340) {
          // Next to password
          if (strlen(enteredSSID) > 0) {
            wifiScreenPage = 2;
            strcpy(enteredPassword, "");
            keyboardIndex = 0;  // Reset keyboard index
            drawWiFiScreen();
          }
        }
        else if (x >= 170 && x <= 310 && y >= 290 && y <= 340) {
          // Cancel
          wifiScreenPage = 0;
          drawWiFiScreen();
        }
      }
      else if (wifiScreenPage == 2) {
        // Mode selection buttons (y=160-195)
        if (y >= 160 && y <= 195) {
          if (x >= 10 && x <= 100) {
            // abc mode
            keyboardMode = 0;
            currentKeyboard = keyboardLower;
            currentKeyboardSize = 26;
            keyboardIndex = 0;
            drawWiFiScreen();
            return;
          }
          else if (x >= 115 && x <= 205) {
            // ABC mode
            keyboardMode = 1;
            currentKeyboard = keyboardUpper;
            currentKeyboardSize = 26;
            keyboardIndex = 0;
            drawWiFiScreen();
            return;
          }
          else if (x >= 220 && x <= 310) {
            // 123 mode
            keyboardMode = 2;
            currentKeyboard = keyboardNum;
            currentKeyboardSize = 19;
            keyboardIndex = 0;
            drawWiFiScreen();
            return;
          }
        }
        
        // Password keyboard handling - buttons at y=220, y=290, and y=360
        if (x >= 10 && x <= 150 && y >= 220 && y <= 270) {
          // Add character
          if (strlen(enteredPassword) < 16) {
            size_t len = strlen(enteredPassword);
            if (len < 16) {
              enteredPassword[len] = currentKeyboard[keyboardIndex % currentKeyboardSize];
              enteredPassword[len + 1] = '\0';
            }
            keyboardIndex++;
            drawWiFiScreen();
          }
        }
        else if (x >= 170 && x <= 310 && y >= 220 && y <= 270) {
          // Delete character
          if (strlen(enteredPassword) > 0) {
            size_t len = strlen(enteredPassword);
            if (len > 0) enteredPassword[len - 1] = '\0';
            drawWiFiScreen();
          }
        }
        else if (x >= 10 && x <= 75 && y >= 290 && y <= 340) {
          // Up button - cycle forward
          keyboardIndex++;
          drawWiFiScreen();
        }
        else if (x >= 85 && x <= 150 && y >= 290 && y <= 340) {
          // Down button - cycle backward
          keyboardIndex--;
          if (keyboardIndex < 0) keyboardIndex = currentKeyboardSize - 1;
          drawWiFiScreen();
        }
        else if (x >= 170 && x <= 310 && y >= 290 && y <= 340) {
          // Back to SSID entry
          wifiScreenPage = 1;
          drawWiFiScreen();
        }
        else if (x >= 10 && x <= 310 && y >= 360 && y <= 410) {
          // Connect
          Serial.printf("Connecting to WiFi: %s\n", enteredSSID);
          
          // Save credentials to EEPROM for future use
          saveWiFiCredentials(enteredSSID, enteredPassword);
          
          // Connect to the new WiFi
          WiFi.begin(enteredSSID, enteredPassword);
          screenNbr = 0;
          DrawBackground = true;
        }
      }
    }
    
    // WiFi Info Screen touch handling - dismiss on any touch
    if (showWiFiInfo && !TouchLatch) {
      TouchLatch = true;
      Serial.println("Dismissing WiFi info screen");
      showWiFiInfo = false;
      
      // If OBD2 is not connected, show the OBD2 fail screen again
      if (!OBD2connected) {
        showOBD2FailScreen = true;
        
        // Redraw OBD2 fail screen
        drawBLEConnectionError(lcd);
      } else {
        DrawBackground = true;
      }
      return;
    }
    
    // Save Confirmation Dialog touch handling
    if (showSaveConfirmation && !TouchLatch) {
      TouchLatch = true;
      
      // YES button (40, 300, 100x60)
      if (x >= 40 && x <= 140 && y >= 300 && y <= 360) {
        Serial.println("Save confirmed - calling save_lost('P')");
        showSaveConfirmation = false;
        save_lost('P');
        
        // Show success message briefly
        lcd.fillScreen(TFT_BLACK);
        lcd.setFont(&FreeSans18pt7b);
        lcd.setTextColor(TFT_GREEN);
        lcd.drawString("Data Saved!", 160, 240);
        delay(1500);
        
        // Return to previous screen
        DrawBackground = true;
        return;
      }
      
      // NO button (180, 300, 100x60)
      if (x >= 180 && x <= 280 && y >= 300 && y <= 360) {
        Serial.println("Save cancelled");
        showSaveConfirmation = false;
        
        // Return to previous screen
        DrawBackground = true;
        return;
      }
    }
    
    //Button 4 test
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

    //Button 5 test
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

    //Button 6 test
    if (x >= 0 && x <= 320 && y >= 0 && y <= 60 && screenNbr == 2) {
      TouchTime = (millis() - initTouchTime) / 1000;
      if (!TouchLatch && TouchTime >= 2) {            
        Serial.println("Screen Touched");
        TouchLatch = true;        
        InitRst = true;            
        PrevSoC = 0;                
      }      
    }

    //Button 7 test
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

    //Button 8 test
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


//--------------------------------------------------------------------------------------------
//                        Button Draw function
//--------------------------------------------------------------------------------------------

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

//--------------------------------------------------------------------------------------------
//                   Format Displays in Pages
//--------------------------------------------------------------------------------------------

void DisplayPage() {
  if (DrawBackground) {
    lcd.fillScreen(TFT_BLACK);
    lcd.setTextPadding(160);
    lcd.setTextSize(1);
    lcd.setFreeFont(&FreeSans12pt7b);        
    lcd.setTextColor(TFT_WHITE, TFT_BLUE);    
    
    // Draw parameter names
    for (int i = 0; i < 10; i++) {  
      if (i < 5) { // update left colunm
        lcd.drawString(titre[i], lcd.width() / 4, textLvl[i]);
      }
      else { // update right colunm
        lcd.drawString(titre[i], 3 * (lcd.width() / 4), textLvl[i]);
      }
    }

    // Draw frame lines
    lcd.drawLine(1,textLvl[0] - 13,319,textLvl[0] - 13,TFT_DARKGREY);
    lcd.drawLine(lcd.width() / 2,textLvl[0] - 13,lcd.width() / 2,drawLvl[4] + 24,TFT_DARKGREY);
    lcd.drawLine(1,drawLvl[4] + 24,319,drawLvl[4] + 24,TFT_DARKGREY);

    // Initialize previous values to empty value so value will be updated when background is redrawn
    for (int i = 0; i < 10; i++) {
      strcpy(prev_value[i], "");
    }
    
    // button state to display
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
        
      case 4:  // WiFi Selection Screen
        lcd.fillScreen(TFT_BLACK);
        lcd.setTextSize(1);
        lcd.setFreeFont(&FreeSans12pt7b);
        lcd.setTextColor(TFT_WHITE, TFT_BLUE);
        lcd.drawString("WiFi Settings", lcd.width() / 2, 22);
        
        if (wifiScreenPage == 0) {
          // Display WiFi network list
          lcd.setFreeFont(&FreeSans9pt7b);
          lcd.setTextColor(TFT_WHITE, TFT_BLACK);
          lcd.drawString("Available Networks:", 10, 70);
          
          for (int i = 0; i < wifiCount && i < 8; i++) {
            int yPos = 100 + (i * 35);
            lcd.setTextColor(TFT_GREEN, TFT_BLACK);
            lcd.drawString(wifiSSIDList[i], 20, yPos);
            lcd.setTextColor(TFT_CYAN, TFT_BLACK);
            char rssiStr[16];
            sprintf(rssiStr, "%ddBm", wifiRSSI[i]);
            lcd.drawString(rssiStr, 250, yPos);
            lcd.drawLine(10, yPos + 15, 310, yPos + 15, TFT_DARKGREY);
          }
          
          // Draw buttons
          lcd.fillRoundRect(10, 420, 140, 50, 8, TFT_DARKGREY);
          lcd.setTextColor(TFT_WHITE, TFT_DARKGREY);
          lcd.drawString("Manual", 80, 445);
          
          lcd.fillRoundRect(170, 420, 140, 50, 8, TFT_RED);
          lcd.setTextColor(TFT_WHITE, TFT_RED);
          lcd.drawString("Close", 240, 445);
        }
        else if (wifiScreenPage == 1) {
          // SSID entry keyboard
          lcd.setFreeFont(&FreeSans9pt7b);
          lcd.setTextColor(TFT_WHITE, TFT_BLACK);
          lcd.drawString("Enter SSID:", 10, 70);
          lcd.fillRect(10, 90, 300, 40, TFT_DARKGREY);
          lcd.setTextColor(TFT_YELLOW, TFT_DARKGREY);
          lcd.drawString(enteredSSID, 20, 110);
          
          // Draw simple keyboard
          lcd.fillRoundRect(10, 150, 140, 50, 8, TFT_BLUE);
          lcd.setTextColor(TFT_WHITE, TFT_BLUE);
          lcd.drawString("Add Char", 80, 175);
          
          lcd.fillRoundRect(170, 150, 140, 50, 8, TFT_BLUE);
          lcd.drawString("Delete", 240, 175);
          
          lcd.fillRoundRect(10, 220, 300, 50, 8, TFT_GREEN);
          lcd.setTextColor(TFT_WHITE, TFT_GREEN);
          lcd.drawString("Next (Password)", 160, 245);
          
          lcd.fillRoundRect(10, 290, 300, 50, 8, TFT_RED);
          lcd.setTextColor(TFT_WHITE, TFT_RED);
          lcd.drawString("Cancel", 160, 315);
        }
        else if (wifiScreenPage == 2) {
          // Password entry keyboard
          lcd.setFreeFont(&FreeSans9pt7b);
          lcd.setTextColor(TFT_WHITE, TFT_BLACK);
          lcd.drawString("Enter Password:", 10, 70);
          lcd.fillRect(10, 90, 300, 40, TFT_DARKGREY);
          lcd.setTextColor(TFT_YELLOW, TFT_DARKGREY);
          String maskedPassword = "";
          for (unsigned int i = 0; i < strlen(enteredPassword); i++) {
            maskedPassword += "*";
          }
          lcd.drawString(maskedPassword, 20, 110);
          
          // Draw simple keyboard
          lcd.fillRoundRect(10, 150, 140, 50, 8, TFT_BLUE);
          lcd.setTextColor(TFT_WHITE, TFT_BLUE);
          lcd.drawString("Add Char", 80, 175);
          
          lcd.fillRoundRect(170, 150, 140, 50, 8, TFT_BLUE);
          lcd.drawString("Delete", 240, 175);
          
          lcd.fillRoundRect(10, 220, 300, 50, 8, TFT_GREEN);
          lcd.setTextColor(TFT_WHITE, TFT_GREEN);
          lcd.drawString("Connect", 160, 245);
          
          lcd.fillRoundRect(10, 290, 300, 50, 8, TFT_RED);
          lcd.setTextColor(TFT_WHITE, TFT_RED);
          lcd.drawString("Back", 160, 315);
        }                
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
  
  // Skip normal value display for WiFi settings screen
  if (screenNbr == 4) {
    return;
  }
  
  lcd.setTextSize(1);
  lcd.setFreeFont(&FreeSans18pt7b);  

  // test for negative values and set negative flag
  for (int i = 0; i < 10; i++) {  
    if (value_float[i] < 0) {    
      negative_flag[i] = true;
    }
    else {
      negative_flag[i] = false;
    }
    
    dtostrf(value_float[i], 3, nbr_decimal[i], value[i]);    
    
    //if value changes update values (use strcmp for proper string comparison)
    if ((strcmp(value[i], prev_value[i]) != 0) && (i < 5)) { // update left column
      // Erase old value by drawing over it with black text
      lcd.setTextColor(TFT_BLACK, TFT_BLACK);
      lcd.drawString(prev_value[i], lcd.width() / 4, drawLvl[i]);
      
      // Draw new value with proper color
      if (negative_flag[i]) {
        lcd.setTextColor(TFT_ORANGE, TFT_BLACK);
      } 
      else {
        lcd.setTextColor(TFT_GREEN, TFT_BLACK);
      }
      lcd.drawString(value[i], lcd.width() / 4, drawLvl[i]);
      strcpy(prev_value[i], value[i]);
    }
    else if (strcmp(value[i], prev_value[i]) != 0) { // update right column
      // Erase old value by drawing over it with black text
      lcd.setTextColor(TFT_BLACK, TFT_BLACK);
      lcd.drawString(prev_value[i], 3 * (lcd.width() / 4), drawLvl[i]);
      
      // Draw new value with proper color
      if (negative_flag[i]) {
        lcd.setTextColor(TFT_ORANGE, TFT_BLACK);
      } 
      else {
        lcd.setTextColor(TFT_GREEN, TFT_BLACK);
      }
      lcd.drawString(value[i], 3 * (lcd.width() / 4), drawLvl[i]);
      strcpy(prev_value[i], value[i]);
    }  
  }
}

//-------------------------------------------------------------------------------------
//             Start of Pages content definition
//-------------------------------------------------------------------------------------

/*///////////////// Display Page 1 //////////////////////*/
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
  
  // set number of decimals for each value to display
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
/*///////////////// End of Display Page 1 //////////////////////*/

/*///////////////// Display Page 2 //////////////////////*/
void page2() {

  strcpy(titre[0], "SoC");
  strcpy(titre[1], "Full Ah");
  strcpy(titre[2], "MAXcellv");
  strcpy(titre[3], "SOH");
  strcpy(titre[4], "AuxBatt_SoC");
  strcpy(titre[5], "BmsSoC");
  strcpy(titre[6], "BATTv");
  strcpy(titre[7], "MINcellv");
  strcpy(titre[8], "Cell Vdiff");
  strcpy(titre[9], "AuxBattV");
  value_float[0] = SoC;
  value_float[1] = EstFull_Ah;
  value_float[2] = MAXcellv;
  value_float[3] = SOH;
  value_float[4] = AuxBatt_SoC;
  value_float[5] = BmsSoC;
  value_float[6] = BATTv;
  value_float[7] = MINcellv;
  value_float[8] = CellVdiff;
  value_float[9] = AuxBattV;
  
  // set number of decimals for each value to display
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
/*///////////////// End of Display Page 2 //////////////////////*/

/*///////////////// Display Page 3 //////////////////////*/
void page3() {

  strcpy(titre[0], "Power");
  strcpy(titre[1], "MIN Temp");
  strcpy(titre[2], "PID kWh");
  strcpy(titre[3], "kWh Used");
  strcpy(titre[4], "SoC");
  strcpy(titre[5], "Max Pwr");
  strcpy(titre[6], "MAX Temp");
  strcpy(titre[7], "Int. Energ");
  strcpy(titre[8], "degrad_ratio");
  strcpy(titre[9], "Chauf. Batt.");  
  value_float[0] = Power;
  value_float[1] = BattMinT;
  value_float[2] = Net_kWh2;
  value_float[3] = used_kWh;
  value_float[4] = SoC;
  value_float[5] = Max_Pwr;
  value_float[6] = BattMaxT;
  value_float[7] = acc_energy;
  value_float[8] = degrad_ratio;
  value_float[9] = Heater;

  // set number of decimals for each value to display
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
/*///////////////// End of Display Page 3 //////////////////////*/

/*///////////////// Display Page 4 //////////////////////*/
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
  
  // set number of decimals for each value to display
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
/*///////////////// End of Display Page 4 //////////////////////*/

/*///////////////////////////////////////////////////////////////////////*/
/*                     START OF LOOP                                     */
/*///////////////////////////////////////////////////////////////////////*/

void loop()
{
  // Handle web server requests
  if (WiFi.status() == WL_CONNECTED) {
    server.handleClient();
  }
  
  /*/////// Read each OBDII PIDs /////////////////*/     
  if ((OBDscanning || ResetOn) && OBD2connected){
    pid_counter++;
    read_data();    
  }
  else if (((millis() - read_timer) > read_data_interval) && OBD2connected){ // if BMS is not On, only scan OBD2 at some intervals
    read_data();
    read_timer = millis();            
  }
  
  /*/////// Check if touch buttons are pressed /////////////////*/
  button();
    
  /*/////// SD card logging every 10 seconds /////////////////*/
  if (sending_data) {
    // Get timestamp for SD card logging
    timeinfo = getTime();
    Serial.print("Local time: ");
    Serial.printf("%02d-%02d-%04d %02d:%02d:%02d\n", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    
    sprintf(EventTime, "%02d-%02d-%04d %02d:%02d:%02d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    
    // Save event code first (if any) before regular data
    saveEventCodeToSD(EventTime);
    
    // Save to SD card
    saveToSD(EventTime);
    
    // Save to second file when SoC decreases
    if (SoC_decreased) {
      saveEventCodeToSD_SoC(EventTime);  // Save event code to SoC file first
      saveToSD_SoCDecrease(EventTime);   // Then save regular data
      SoC_decreased = false;  // Reset flag after saving
    }
    
    // Auto-save every 25km
    if(dist_save >= 25 && !save_sent){
      save_sent = true;
      save_lost('P');
      init_distsave = Trip_dist;      
    }
    
    sending_data = false;
    
    // Set flag for LED indicator
    sdCardSaved = true;
    SDsaveTimer = millis();
  }
  
  /*/////// Display SD card save indicator /////////////////*/
  // Show brief green flash when data is saved to SD card
  if (sdCardSaved) {
    lcd.fillCircle(20, 20, 6, TFT_GREEN);
    if (millis() - SDsaveTimer >= 500) {
      sdCardSaved = false;
    }
  }
  else {
    lcd.fillCircle(20, 20, 6, TFT_BLACK);
  }  /*/////// Display Page Number /////////////////*/

  // OBD2 fail screen takes highest priority
  if (showOBD2FailScreen) {
    return;
  }

  // Device selection screen takes priority
  if (showDeviceSelection) {
    return;
  }

  // Save confirmation dialog takes priority - don't update display while showing
  if (showSaveConfirmation) {
    return;
  }

  // WiFi info screen takes priority - don't update display while showing
  if (showWiFiInfo) {
    // WiFi info screen is displayed, wait for user to dismiss
    return;
  }
  
  // WiFi screen can be shown anytime, regardless of OBD2 connection
  if (screenNbr == 4) {
    // WiFi settings screen is active - keep it displayed
    // Nothing to do here as drawWiFiScreen() is called from button handler
  }
  else if (OBD2connected && SoC != 0) {    
    Serial.println(" OBD2 connected - displaying data");
    if (display_off){
      Serial.println("Turning Display ON");
      lcd.setBrightness(128); // Switch on the display      
      display_off = false;
      SoC_saved = false;
      
      Serial.println("Display going ON");
      DrawBackground = true;
    }

    if (StartWifi) {  // If wifi is configured then display wifi status led
      if (WiFi.status() == WL_CONNECTED){
        lcd.fillCircle(300, 20, 6,TFT_GREEN);
      }
      else{
        lcd.fillCircle(300, 20, 6,TFT_RED);
      }
    }
     else{
      lcd.fillCircle(300, 20, 6,TFT_BLACK);
    }
	
        
    switch (screenNbr) {  // select page to display
      case 0: page1(); break;
      case 1: page2(); break;
      case 2: page3(); break;
      case 3: page4(); break;
    }
  }

  /*/////// Turn off display when BMS is off /////////////////*/
  else {    
    //lcd.setBrightness(0); // Switch off the display    
    display_off = true;    
  }
  
  ResetCurrTrip();  // Check if condition are met
}