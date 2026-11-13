// FASTROBOX 1.0 TECHATHON GRAND FINALE — BUBT
// AI-Powered Smart Classroom: Safety, Access and Vision
// Official Question Paper & Hardware Pinout (ESP32-S3-N16R8 CAM)

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// 1. TEAM & DEVICE IDENTIFICATION
#define TEAM_ID                   "16"
#define DEVICE_ID                 "16-D1"
#define FIRMWARE_VERSION          "1.0.0"
#define FIRMWARE_BUILD_DATE       __DATE__ " " __TIME__

// Set to 0 when judging Level 1 standalone (keeps LED Green 2Hz instead of Blue AP mode)
// Set to 1 for Level 2+ (enables Wi-Fi Setup AP, mDNS, Local Web Server, HTTPS Telemetry)
#define ENABLE_LEVEL_2_WIFI       1

// 2. PIN DEFINITIONS MATCHING ESP32-S3-N16R8 BOARD DIAGRAM
// --- RC522 RFID Reader (SPI) ---
#define PIN_RC522_SCK             39
#define PIN_RC522_MISO            40
#define PIN_RC522_MOSI            41
#define PIN_RC522_SS              42
#define PIN_RC522_RST             2

// --- Environmental & Safety Sensors ---
#define PIN_DHT11                 1     // DHT11 Data (wired per Master Pin Table)
#define PIN_FLAME_DO              3     // Flame Sensor DO (Active LOW)
#define PIN_BUZZER                38
#define PIN_TOUCH_PANIC           4     // Capacitive Touch Channel 4 (T4)

// --- Onboard Hardware ---
#define PIN_RGB_NEOPIXEL          48    // Built-in WS2812 RGB LED
#define PIN_BOOT_BTN              0     // Built-in BOOT Button (Active LOW)

// 3. LEVEL 3 SAFETY THRESHOLDS & HYSTERESIS (Question Paper Spec)
#define TEMP_WARN_DEFAULT         32.0f // Enter Warning >= 32°C or >= 85% RH
#define HUMID_WARN_DEFAULT        85.0f
#define TEMP_DANGER_DEFAULT       40.0f // Enter Danger >= 40°C
#define TEMP_HYSTERESIS           2.0f  // Leave Danger at 38°C, Warning at 30°C
#define HUMID_HYSTERESIS          5.0f  // Leave Warning at <= 80% RH

#define DHT_VALID_TEMP_MIN        0.0f
#define DHT_VALID_TEMP_MAX        50.0f
#define DHT_VALID_HUMID_MIN       20.0f
#define DHT_VALID_HUMID_MAX       90.0f

#define STATUS_HOLD_MS            3000  // New status counts only after holding 3 s
#define FLAME_HOLD_MS             2000  // Flame continuous 2 s -> FIRE (no 3s hold)
#define TOUCH_HOLD_TIME_MS        2000  // Touch 2 s -> PANIC (ignore < 1s)
#define DOOR_UNLOCK_TIME_SEC      5     // Door unlocks for 5 s on valid tap
#define BOOT_RESET_HOLD_MS        5000  // Hold BOOT 5 s -> clear creds & setup mode

// 4. POLLING & TELEMETRY INTERVALS (Level 1 & Level 2 Spec)
#define DHT_READ_INTERVAL_MS      5000  // DHT11 every 5 s
#define FLAME_READ_INTERVAL_MS    100   // Flame every 100 ms
#define RFID_POLL_INTERVAL_MS     200   // RC522 every 200 ms
#define SENSOR_DHT_ERR_MS         12000 // DHT11 error within 12 s
#define SENSOR_RFID_ERR_MS        3000  // RFID error within 3 s
#define TELEMETRY_INTERVAL_MS     5000  // Telemetry every 5 ± 1 s
#define WIFI_SETUP_TIMEOUT_MS     13000 // Wrong password -> back to setup <= 15 s
#define WDT_TIMEOUT_MS            8000  // Task Watchdog <= 10 s (8s gives margin for USB CDC)

// 5. SETUP AP & MDNS
#define SETUP_AP_PREFIX           "SmartClass-"
#define MDNS_HOSTNAME             "smartclass-t07"

#endif // CONFIG_H
