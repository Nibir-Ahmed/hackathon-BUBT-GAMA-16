# Required Arduino IDE Libraries & Board Setup

Follow these exact steps to prepare your Arduino IDE for the **FASTROBOX 1.0 Techathon Grand Finale**:

---

## 1. Arduino IDE Board Package Installation

1. Open **Arduino IDE** (version 2.x recommended).
2. Go to **File -> Preferences**.
3. In the **Additional boards manager URLs** field, paste:
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
4. Open **Tools -> Board -> Boards Manager...**, search for `esp32` by **Espressif Systems**, and install the latest version (version $\ge 2.0.14$ or $3.0+$).
5. Select Board:
   - **Tools -> Board -> esp32 -> ESP32S3 Dev Module** (or `Freenove ESP32S3-CAM` if available in the board list).
   - **PSRAM:** `OPI PSRAM` (or `Enabled`).
   - **Flash Size:** `8MB` or `16MB`.
   - **Upload Speed:** `921600`.
   - **USB CDC On Boot:** `Enabled` (Crucial for Serial Monitor on ESP32-S3).

---

## 2. Required Libraries (Install via Library Manager)

Open **Tools -> Manage Libraries...** (or `Ctrl+Shift+I` / `Cmd+Shift+I`) and install each of the following:

| # | Library Name | Author | Version | Purpose in Hackathon |
|---|---|---|---|---|
| 1 | **WiFiManager** | tzapu | Latest ($\ge 2.0.16$) | Captive Portal for dynamic Wi-Fi SSID, Password & Room ID (Level 1) |
| 2 | **MFRC522** | GithubCommunity / miguelbalboa | Latest | RC522 13.56MHz RFID Reader driver (Levels 5, 8) |
| 3 | **DHT sensor library** | Adafruit | Latest ($\ge 1.4.4$) | DHT11 Temperature & Humidity sensor (Level 3) |
| 4 | **Adafruit Unified Sensor** | Adafruit | Latest | Dependency for DHT sensor library |
| 5 | **Adafruit NeoPixel** | Adafruit | Latest | Built-in WS2812 RGB LED status color driver (GPIO 48) |
| 6 | **ArduinoJson** | Benoit Blanchon | Version 6.x or 7.x | JSON serialization for API, Firebase, & LittleFS buffer |
| 7 | **LittleFS** | Built-in | Included with ESP32 core | Flash storage for offline queue resilience (Level 11) |
| 8 | **esp32-camera** | Espressif Systems | Built-in with ESP32 core | OV2640 camera driver & image capture (Levels 6, 8) |

---

## 3. How to Open and Compile
1. In Arduino IDE, go to **File -> Open...**
2. Browse to:
   ```
   /Users/mdnibirahmed/Desktop/smart_classroom_hackathon/firmware/firmware.ino
   ```
3. Connect your **ESP32-S3-CAM** using your **Data-capable USB Cable**.
4. Select the matching COM / Serial port under **Tools -> Port**.
5. Click **Verify (Checkmark button)** to ensure everything compiles cleanly.
6. Click **Upload (Arrow button)** to flash the firmware.
7. Open **Tools -> Serial Monitor** at **115200 baud** to view real-time logs.

