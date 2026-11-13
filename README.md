# hackathon-BUBT-GAMA-16

# FastRobox 1.0 Techathon — Smart Classroom (Team 16: BUBT GAMA)

AI-Powered Smart Classroom: Safety, Access and Vision on ESP32-S3-N16R8 CAM.

## Repository Structure
- `firmware/` — ESP32-S3 Arduino firmware (`firmware.ino` + headers + LittleFS `data/index.html`)
- `server/` — Node.js + Express + SQLite (`node:sqlite`) HTTPS backend
- `dashboard/` — Web dashboard (Level 4+)

## Building & Flashing the Firmware (Arduino IDE)
1. Install **esp32** board package (v2.0.14+ or 3.x) via Boards Manager.
2. Select **ESP32S3 Dev Module** (PSRAM: `OPI PSRAM`, Flash Size: `16MB`, USB CDC On Boot: `Enabled`).
3. Install libraries: `MFRC522`, `DHT sensor library`, `Adafruit Unified Sensor`, `Adafruit NeoPixel`, `ArduinoJson` (v6/v7).
4. Upload `firmware/data/index.html` to LittleFS using the Arduino LittleFS Uploader plugin (`ESP32 Sketch Data Upload`).
5. Open `firmware/firmware.ino`, compile, and upload.
6. Open Serial Monitor at **115200 baud**.

### Initial Setup (Level 2)
1. On first boot (or after holding `BOOT` for 5s / typing `reset` in Serial), the board starts AP `SmartClass-16` with a random 8-character password printed on Serial.
2. Connect to `SmartClass-16` and open `http://192.168.4.1`.
3. Select/enter Wi-Fi SSID, Password, Room ID, your deployed HTTPS Server URL, and Device Token.
4. Once connected, the local status page is available at the printed IP and `http://smartclass-16.local`.

### Registering Teacher Key Tag (Level 3)
Tap the blue key tag on the RC522 reader to see its UID on Serial (`[CARD] uid=...`), then register it via Serial:
```
teacher add <UID>
```
Or via the **Teacher Tag Registration** box on the local status page.

## Running the Backend Server (`server/`)
Requires Node.js >= 22.5 (tested on v24.9.0).
```bash
cd server
cp .env.example .env
# Edit .env and set a strong DEVICE_TOKEN
npm install
npm start
```
Deploy `server/` to any public HTTPS host (Render, Railway, Fly.io) with `DEVICE_TOKEN` set in platform environment variables. Never commit `.env` or tokens to git.
