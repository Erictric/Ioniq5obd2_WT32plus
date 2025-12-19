# Ioniq5 OBD2 WT32-SC01 Plus Project

ESP32-S3 (WT32-SC01-Plus) dashboard for Hyundai Ioniq 5 using BLE OBD (Vgate iCar Pro BT4.0), LovyanGFX display/touch, Google Sheets logging, and ELMDuino.

**Current Version: v2.3.1**

## Environment

- Framework: Arduino (ESP32 core 3.x) via PlatformIO
- Board: `esp32-s3-devkitm-1`
- PlatformIO partition file: `huge_app.csv` (set in `platformio.ini`) to accommodate large firmware size.
- Toolchains auto-installed by PlatformIO (`xtensa-esp32s3`, `riscv32-esp`).

## Dependencies (platformio.ini)

```ini
lib_deps =
  lovyan03/LovyanGFX@^1.2.7
  powerbroker2/ELMDuino@^3.4.1
  powerbroker2/SafeString@^4.1.42
  mobizt/ESP-Google-Sheet-Client@^1.4.12
  adafruit/Adafruit GFX Library@^1.12.3
```

Additional core libs used: WiFi, EEPROM, BLE (from ESP32 core).

## Building

In VS Code (PlatformIO extension):

1. Open the folder `C:\Projects\Ioniq5obd2_WT32plus` (avoid OneDrive for stability).

2. Ensure `platformio.ini` contains:

   ```ini
   [env:esp32-s3-devkitm-1]
   platform = espressif32
   board = esp32-s3-devkitm-1
   framework = arduino
   board_build.flash_mode = qio
   board_build.partitions = huge_app.csv
   ```

3. Build:

   ```bash
   pio run --environment esp32-s3-devkitm-1
   ```

4. Upload (USB connected WT32-SC01 Plus):

   ```bash
   pio run --target upload --environment esp32-s3-devkitm-1
   ```

5. Monitor serial (115200 baud):

   ```bash
   pio device monitor -b 115200
   ```

## BLE OBD Connection

- Adapter name expected: `IOS-Vlink` (set in `BLEClientSerial.cpp`).
- Connection sequence handled in `BT_communication.h::ConnectToOBD2()`.
- **BLE Signal Strength (RSSI)**: Displays signal strength during device scan and after connection for troubleshooting intermittent connections.
- **Improved Connection UI**: 4-second countdown during device scan with clear "Device Found" (green) or "Device not found" (red) status.
- **Device Selection**: Touch-based BLE device picker with pagination support (4 devices per page) and EEPROM persistence.

## SD Card Data Logging

- **Auto-detection**: SD card initializes in 1-bit mode on startup for WT32-SC01 Plus.
- **CSV format**: Logs to `/ioniq5_log.csv` with same fields as Google Sheets.
- **Automatic header**: Creates CSV header on first run if file doesn't exist.
- **No SD card**: System continues normally if no card is detected (logs to Google Sheets only).
- **Data**: Each row contains timestamp, SoC, power, temperatures, battery metrics, tire pressures, and all telemetry sent to Google Sheets.
- **File size management**: Automatically archives files when they exceed 1MB, creating timestamped backups.
- **12V Battery Monitoring**: Logs both new (PID 22E011) and legacy 12V voltage sources for comparison (AuxBattV and AuxBattV_old).

## Modified / Shim Files

- `include/driver/spi_common.h`: Minimal shim to satisfy LovyanGFX when using Arduino framework (provides ESP-IDF types).
- `include/sdkconfig.h`: Stub (only if needed by some libs; can be removed if no references).
- Added const-safe overload: `BLEClientSerial::begin(const char*)`.
- Button text and event strings defined as `const char*` for warning-free compilation.

## Common Warnings & Fixes

| Issue | Fix |
|-------|-----|
| ISO C++ forbids converting string constant to `char*` | Cast or add const overloads (implemented for BLE; ELM327 still expects mutable) |
| Undefined reference to `setup()` / `loop()` | Caused by OneDrive deleting intermediate `.ino.cpp`; resolved by moving project out of OneDrive |
| Program size exceeds default partition | Switched to `huge_app.csv` |

## Optional Size Optimizations

Add (only if near limits):

```ini
build_unflags = -fexceptions -fno-rtti
build_flags = -fno-exceptions -fno-rtti -g0 -DNDEBUG
```

(Verify runtime features needed before stripping RTTI/exceptions.)

## Troubleshooting

- If build intermittently fails with missing `Ioniq5obd2_WT32plus.ino.cpp`: ensure project not under OneDrive sync or pause sync.
- If BLE fails to connect: verify adapter powered, name matches, reduce scan window/time if needed.
- If Google Sheet logging stalls: check WiFi credentials, project ID, and API quota; verify `sending_data` logic.

## Extending

Wrap ELM queries with a helper for const safety:

```cpp
inline int queryPID_c(const char* pid) { return myELM327.queryPID((char*)pid); }
```

Replace existing `myELM327.queryPID((char*)"220101")` with `queryPID_c("220101")` for clarity.

## Flash & Partition

`huge_app.csv` normally provides a larger app slot. If custom partitioning required, copy an existing csv from ESP32 core `tools/partitions/` and adjust app size ensuring total < flash size.

## License / Attribution

- LovyanGFX (MIT), Adafruit GFX (BSD/MIT), ELMDuino (open source), SafeString (per project site), ESP-Google-Sheet-Client.

Verify each library's license for distribution compliance.

## Quick Commands

```bash
pio run
pio run -t upload
pio device monitor -b 115200
```

## Features

### Battery Monitoring
- **12V Battery Auto-Alert**: Automatically switches to Battery Info screen when 12V battery SoC drops below 70% (with 75% hysteresis to prevent oscillation).
- **Accurate 12V Voltage**: Corrected voltage extraction from PID 22E011 (Frame 2 Byte 6 + Frame 3 Byte 0).
- **Cell Voltage Tracking**: Displays MAX/MIN cell voltages, cell numbers, and voltage differential.
- **State of Health (SOH)**: Tracks battery degradation and estimated full capacity.

### Display Screens
- **Consumption Screen**: Real-time power usage, trip data, and efficiency metrics.
- **Battery Info**: Detailed battery status including cell voltages, temperatures, and health.
- **Energy Screen**: Energy flow visualization and statistics.
- **Battery Info 2**: Additional battery diagnostics.

### WiFi & Data
- **WiFi Configuration**: Touch-based WiFi setup with on-screen keyboard.
- **Google Sheets Integration**: Real-time data logging to Google Sheets.
- **Dual Logging**: Simultaneous SD card and cloud logging with automatic failover.

## Version History

### v2.3.1 (December 19, 2025)
- Added BLE signal strength (RSSI) display during connection and when connected
- Improved BLE connection sequence with 4-second countdown timer
- Fixed 12V battery voltage extraction from PID 22E011
- Added AuxBattV_old to track legacy voltage source for comparison
- Swapped Cell Vdiff and MINcellv positions in Battery Info screen
- Reduced BLE scan time from 5 to 4 seconds
- Removed repetitive connection attempt messages

## Next Ideas

- Add OTA update flow.
- Persist last trip metrics in NVS.
- Add optional PSRAM use for large display buffers.
- Implement trip history and statistics.

---

Generated README summarizing current configuration and workflow.
