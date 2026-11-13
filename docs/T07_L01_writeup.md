# FastRobox 1.0 Techathon (BUBT) — Level 1 Write-up
**Team ID:** T07 | **Device ID:** T07-D1 | **Level:** 1 — Dual-Core Tasks, Health and Watchdog (35 marks)
**Git Tag:** `level-01` | **Commit Hash:** `<PASTE_COMMIT_HASH>`

---

## 1. Design
The firmware runs on a single **ESP32-S3-N16R8 CAM** using FreeRTOS dual-core tasks with zero blocking waits, a single-owner priority-based RGB/buzzer controller (`DeviceSignalsManager`), and a 10-second Task Watchdog Timer (`esp_task_wdt`).

### Wiring Diagram (Pin Connections)
| Component | Pin / Signal | ESP32-S3 GPIO |
|---|---|---|
| RC522 RFID | SCK / MISO / MOSI / SS(SDA) / RST | 39 / 40 / 41 / 42 / 2 |
| DHT11 | DATA | 1 |
| IR Flame Sensor | DO (Active LOW) | 3 |
| Active Buzzer | (+) Signal | 38 |
| Touch Panic Pad | Bare jumper wire (T4) | 4 |
| Onboard WS2812 RGB | DIN | 48 |
| Onboard BOOT Button | Button (Active LOW) | 0 |

### Task Architecture
| Task Name | CPU Core | Period / Rate | Responsibilities |
|---|---|---|---|
| `NetWorker` (`TaskCore0`) | Core 0 | 10 ms loop | Task WDT pet, networking/telemetry (Level 2+) |
| `SignalWorker` (`TaskSignal`) | Core 0 | 10 ms loop | Task WDT pet, single-owner RGB LED (2 Hz) & Buzzer state machine |
| `SensorWorker` (`TaskCore1`) | Core 1 | 10 ms loop | Task WDT pet, Flame (100 ms), RFID (200 ms), DHT11 (5 s), Touch (100 ms), Serial CLI, `[STATUS]` (1 s) |

---

## 2. Where it shows
- **Serial Monitor (`115200` baud):**
  - `[EVENT] BOOT reason=POWER_ON` / `WATCHDOG`
  - `[STATUS] up=... T=... RH=... HI=... flame=... door=... state=SAFE ...` every 1 s
  - `[CARD] uid=...` once per tap
  - `[ERROR] SENSOR ERROR: RFID` / `DHT11` on unplug, auto-clearing on reconnect
  - `[CMD]` replies to `help`, `status`, `health`, `tasks`, `ver`
  - *(Insert Serial Monitor screenshot here)*
- **Light and Buzzer:**
  - Green 2 Hz heartbeat (250 ms ON / 250 ms OFF) in Safe state
  - Magenta 2 Hz heartbeat while a sensor error lasts
  - *(Insert photo/note here)*

---

## 3. Test results
| Check / Test Item | Type | Measured Value | Spec Limit | Video Time | Result |
|---|---|---|---|---|---|
| RGB LED heartbeat | Live | `2 Hz` (250 ms on/off) | `2 Hz` | `00:XX` | PASS |
| Card tap debounce (held card) | Live | `1` `[CARD]` line/tap | `1` line/tap | `00:XX` | PASS |
| `health` & `tasks` commands | Live | Printed core, stack, heap, PSRAM, flash, chip temp | Required | `00:XX` | PASS |
| `hang` -> Watchdog reboot | Live | `< 10 s` (`BOOT reason=WATCHDOG`) | `<= 10 s` | `00:XX` | PASS |
| RC522 SDA unplugged 15 s | Video | `< 3 s` (`SENSOR ERROR: RFID`, Magenta) | `<= 3 s` | `00:XX` | PASS |
| DHT11 DATA unplugged 15 s | Video | `< 12 s` (`SENSOR ERROR: DHT11`, Magenta) | `<= 12 s` | `00:XX` | PASS |
| Both sensors reconnected | Video | Auto-cleared to Green 2 Hz | Auto-clear | `00:XX` | PASS |
| Flame at boot -> `door=UNLOCKED` | Write-up | `door=UNLOCKED` at boot | `UNLOCKED` | — | PASS |

---

## 4. Problems and fixes
1. **RC522 custom SPI pins:** `MFRC522(SS, RST)` used the default `SPI` bus instead of custom pins. Fixed by calling `SPI.begin(39, 40, 41, 42)` before `PCD_Init()`.
2. **Blocking Serial input & dual LED owners:** `Serial.readStringUntil('\n')` could block up to 1 s, and multiple classes drove GPIO 48/38. Replaced with a non-blocking character buffer and made `DeviceSignalsManager` the single owner.
3. **Blue LED before Level 2 setup:** Setup AP mode overrode Safe Green. Added `ENABLE_LEVEL_2_WIFI` flag in `config.h` (`0` for Level 1, `1` for Level 2+).

---

## 5. Extra item
- **Fixed `[STATUS]` line (1 s):**
  `[STATUS] up=12s T=29.0C RH=64% HI=31.2C flame=0 door=LOCKED state=SAFE trend=STABLE rssi=- heap=223k psram=8176k chip=45.9C fw=1.0.0`
- **Confirmed read periods:** DHT11 = `5 s` (`5000 ms`), Flame = `100 ms`, RFID = `200 ms`.
- **Flame at start-up:** Booting with flame active on GPIO 47 sets `door=UNLOCKED` immediately.
  *(Insert Serial Monitor screenshot of `[EVENT] FIRE Flame at startup -> Door UNLOCKED` and `[STATUS] ... door=UNLOCKED`)*

