# ESP32-S3-N16R8 CAM Hardware Pin Configuration & Wiring Guide
**Project:** AI-Powered Smart Classroom (FASTROBOX 1.0 Techathon Grand Finale — BUBT)  
**Microcontroller:** ESP32-S3-N16R8 Dual Type-C Board (CH340 TTL + OTG)  
**Strict Rule:** No Breadboard Allowed (Direct Jumper Connections)

---

## 1. Master Pin Configuration Table (Based on Board Diagram)

All external modules are connected exclusively to the **RIGHT HEADER** to guarantee **ZERO conflict** with the Camera (Left Header) and PSRAM pins!

| External Component | Component Pin | ESP32-S3 Board Pin | Header Location | Voltage Level | Role in System |
|---|---|---|---|---|---|
| **RC522 RFID Reader** | **VCC** | **3V3** | **Left Header, Pin 1** | **3.3V ONLY ⚠️** | Power Supply (Never connect to 5V!) |
| | **GND** | **GND** | **Right Header, Pin 20** | 0V | Common Ground |
| | **RST** | **GPIO 2** | **Right Header, Pin 4** | 3.3V Logic | Hardware Reset |
| | **SDA (SS)** | **GPIO 42** | **Right Header, Pin 5** | 3.3V Logic | SPI Chip Select (MTMS) |
| | **MOSI** | **GPIO 41** | **Right Header, Pin 6** | 3.3V Logic | SPI Master Out (MTDI) |
| | **MISO** | **GPIO 40** | **Right Header, Pin 7** | 3.3V Logic | SPI Master In (SD DATA / MTDO) |
| | **SCK** | **GPIO 39** | **Right Header, Pin 8** | 3.3V Logic | SPI Serial Clock (SD CLK / MTCK) |
| **DHT11 Sensor** | **VCC** | **5V / 3V3** | **Left Header, Pin 20 (or 1)**| 3.3V - 5.0V | Power Supply |
| | **GND** | **GND** | **Right Header, Pin 20** | 0V | Common Ground |
| | **DATA** | **GPIO 21** | **Right Header, Pin 17** | Digital Input | Temperature & Humidity Data Line |
| **IR Flame Sensor** | **VCC** | **5V / 3V3** | **Left Header, Pin 20 (or 1)**| 3.3V - 5.0V | Power Supply |
| | **GND** | **GND** | **Right Header, Pin 20** | 0V | Common Ground |
| | **DO** | **GPIO 47** | **Right Header, Pin 16** | Digital Input | Active LOW Digital Flame Output |
| **Active Buzzer** | **(+) Long Leg**| **GPIO 38** | **Right Header, Pin 9** | 3.3V - 5.0V | Audio Alarm Trigger (HIGH = Beep) |
| | **(-) Short Leg**| **GND** | **Right Header, Pin 20** | 0V | Common Ground |
| **Touch Panic Pad** | **Bare Jumper** | **GPIO 1 (T1)** | **Right Header, Pin 3** | Capacitive | Touch Channel 1 (Touch drops value) |
| **Onboard RGB LED** | Internal | **GPIO 48** | Right Header, Pin 15 | Built-in | WS2812 NeoPixel (Table 4 Signals) |
| **BOOT Button** | Internal | **GPIO 0** | Right Header, Pin 13 | Built-in | Hold 5s for Wi-Fi Factory Reset |

---

## 2. Visual Pinout Diagram (Top View)

```
                       ESP32-S3-N16R8 BOARD
                      ┌───────────────────┐
                      │    [ CAMERA ]     │
                      │      OV2640       │
                      │                   │
    [Pin 1]  3V3 ────┤ [1]           [1] ├──── GPIO 43 (U0TXD - Serial TX)
             RST ────┤ [2]           [2] ├──── GPIO 44 (U0RXD - Serial RX)
(CAM)      GPIO 4 ───┤ [3]           [3] ├──── GPIO 1  (T1) ──────── TOUCH PANIC PAD (Bare Wire)
(CAM)      GPIO 5 ───┤ [4]           [4] ├──── GPIO 2  (T2) ──────── RC522 RST
(CAM)      GPIO 6 ───┤ [5]           [5] ├──── GPIO 42 (MTMS) ────── RC522 SDA (SS)
(CAM)      GPIO 7 ───┤ [6]           [6] ├──── GPIO 41 (MTDI) ────── RC522 MOSI
(CAM)      GPIO 15 ──┤ [7]           [7] ├──── GPIO 40 (MTDO) ────── RC522 MISO
(CAM)      GPIO 16 ──┤ [8]           [8] ├──── GPIO 39 (MTCK) ────── RC522 SCK
(CAM)      GPIO 17 ──┤ [9]           [9] ├──── GPIO 38 ───────────── BUZZER (+) Long Leg
(CAM)      GPIO 18 ──┤ [10]         [10] ├──── GPIO 37 (PSRAM - DO NOT USE!)
(CAM)      GPIO 8 ───┤ [11]         [11] ├──── GPIO 36 (PSRAM - DO NOT USE!)
(JTAG)     GPIO 3 ───┤ [12]         [12] ├──── GPIO 35 (PSRAM - DO NOT USE!)
(LOG)      GPIO 46 ──┤ [13]         [13] ├──── GPIO 0  (BOOT Button - Built-in)
(CAM)      GPIO 9 ───┤ [14]         [14] ├──── GPIO 45 (Free VSPI)
(CAM)      GPIO 10 ──┤ [15]         [15] ├──── GPIO 48 (WS2812 RGB LED - Built-in)
(CAM)      GPIO 11 ──┤ [16]         [16] ├──── GPIO 47 ───────────── FLAME SENSOR (DO)
(CAM)      GPIO 12 ──┤ [17]         [17] ├──── GPIO 21 ───────────── DHT11 (DATA)
(CAM)      GPIO 13 ──┤ [18]         [18] ├──── GPIO 20 (USB D-)
(CAM)      GPIO 14 ──┤ [19]         [19] ├──── GPIO 19 (USB D+)
    [Pin 20] 5V  ────┤ [20]         [20] ├──── GND ───────────────── COMMON GROUND (All Sensors)
                      └───────────────────┘
                           [OTG] [TTL]
```

---

## 3. Critical Hardware Safety Rules

### ⚠️ RULE 1: STRICT 3.3V POWER FOR RC522 RFID
* **RC522 VCC must strictly connect to Pin 1 (3V3) on the Left Header.**
* Never connect RC522 VCC to 5V! 5V will instantly fry the MFRC522 chip.
* DHT11 and IR Flame sensor can take either 3.3V or 5V (Pin 20 on Left Header).

### ⚠️ RULE 2: GROUND & POWER SPLITTING (DAISY-CHAINING WITHOUT BREADBOARD)
* You have only one GND pin (Right Header Pin 20).
* **Daisy-Chain Jumper Technique:** Take the GND jumper wires from RC522, DHT11, Flame sensor, and Buzzer (-), twist their male ends tightly together, insert them into a single female jumper wire, insulate the joint with electrical tape, and plug the female end into the board's `GND` pin (Right Header Pin 20).
* Do the same for the power lines.

### ⚠️ RULE 3: TOUCH PANIC PAD SETUP
* Plug one jumper wire into **GPIO 1** (Right Header, Pin 3, marked `T1`).
* Leave the other metallic pin end completely bare and exposed.
* Touching this exposed tip with a human finger drops the touch reading below threshold, immediately triggering the **Level 3 Panic SOS Siren**!

### ⚠️ RULE 4: ZERO CONFLICT WITH CAMERA & PSRAM
* Notice that **GPIO 35, 36, 37** are internal PSRAM pins — they are left completely unconnected.
* Notice that all pins on the Left Header from **GPIO 4 to GPIO 14** are camera DVP bus pins — they are left completely unconnected.
* This ensures that when the camera captures images (Levels 6, 7, 8, 9), it will **never glitch, hang, or conflict with RFID or sensors**!
