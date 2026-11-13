# 12-Level Execution Playbook (Pass All 12 Levels in 4 Calls)

## Call 1: Foundation Group (Levels 1, 2, 3) — Target: Hour 1.5

### Level 1: Board Boot & Dynamic Wi-Fi Connection
- **Requirements:** Board boots up, outputs banner to Serial Monitor, connects to judge's Wi-Fi network without hardcoding.
- **Judge Test Action:** Judge provides an arbitrary Wi-Fi SSID and Password.
- **How to Execute:**
  1. Boot ESP32-S3. If unconfigured, the AP `SmartClassroom-Setup-XXXX` appears.
  2. Member 2 connects via phone, enters judge's SSID, Password, and Room ID.
  3. Serial Monitor prints: `[WIFI] Connected Successfully! IP: 192.168.x.x, RSSI: -xx dBm`.
  4. Video must show: Serial Monitor output and phone screen completing portal setup.

### Level 2: Local Status Web Server
- **Requirements:** ESP32 hosts an embedded web page displaying system IP, RSSI, and simulated door state (`LOCKED`/`UNLOCKED`).
- **Judge Test Action:** Judge opens `http://<esp32-ip>/` in browser and verifies status.
- **How to Execute:**
  1. Open the board's IP address on laptop or phone.
  2. Show simulated door badge, auto-refresh every 3s, and `/api/status` JSON output.

### Level 3: Environmental Sensors (DHT11 + Flame)
- **Requirements:** Live temperature, humidity, and flame sensor states reflected on Serial Monitor and Local Web Page.
- **Judge Test Action:** Judge checks ambient temperature reading and brings a lighter/match near flame sensor.
- **How to Execute:**
  1. DHT11 shows room temp and humidity.
  2. Bring flame near IR Flame sensor: Serial prints `[ALERT] FIRE DETECTED!`, Local Page shows red warning.

---

## Call 2: Cloud & Access Group (Levels 4, 5, 6) — Target: Hour 3.5

### Level 4: Live HTTPS Web Dashboard Sync
- **Requirements:** Telemetry updates sync to public HTTPS web dashboard via Firebase within target latency ($<3$s).
- **Judge Test Action:** Judge views the public URL on their phone or laptop.
- **How to Execute:**
  1. Show Vercel / Netlify dashboard updating temperature and flame state in real time.

### Level 5: RFID Access Control & Simulated Door
- **Requirements:** Tap White Card $\rightarrow$ Door changes to `UNLOCKED`, NeoPixel turns GREEN, buzzer chirps once. Tap Blue Tag $\rightarrow$ Administrative action.
- **Judge Test Action:** Judge watches stopwatch as White Card is tapped on RC522.
- **How to Execute:**
  1. Tap White Card: Door unlocks in $<0.5$s.
  2. Serial Monitor, Local Page, Dashboard, and NeoPixel LED all reflect `UNLOCKED` simultaneously.

### Level 6: Camera Vision Snapshot & Attendance
- **Requirements:** Camera captures photo upon access verification; attendance record logged.
- **Judge Test Action:** Judge watches member tap card and camera snap.
- **How to Execute:**
  1. Member taps card $\rightarrow$ Camera captures VGA frame $\rightarrow$ Attendance row appears on dashboard with timestamp.

---

## Call 3: Safety & Intelligence Group (Levels 7, 8, 9) — Target: Hour 5.2

### Level 7: Emergency Safety Alarms (Fire & Touch Panic)
- **Requirements:** Flame detection triggers evacuation siren and auto-opens door. Touch wire triggers panic SOS alarm.
- **Judge Test Action:** Flame sensor triggered or jumper wire tip touched.
- **How to Execute:**
  1. Flame detected $\rightarrow$ Continuous 200ms siren on buzzer + door switches to `EMERGENCY_OPEN` + red banner on dashboard ($<2$s).
  2. Touch wire pad touched $\rightarrow$ SOS pattern beeps + panic alert logged.

### Level 8: Intruder & Unauthorized Access Detection
- **Requirements:** Unregistered RFID card or unrecognized attempt rejected with alarm beeps and visual intruder snapshot.
- **Judge Test Action:** Judge provides an unregistered card / NFC phone.
- **How to Execute:**
  1. Unregistered card tapped $\rightarrow$ 3 rapid denied beeps, NeoPixel flashes Red, camera captures intruder snapshot, event logged as `intruder`.

### Level 9: Searchable Event History Log
- **Requirements:** Web dashboard provides filterable event history (by type, date, card UID) with CSV export.
- **Judge Test Action:** Judge asks to search for "fire_alarm" or filter by a specific card UID.
- **How to Execute:**
  1. Open Event Log on Dashboard, apply filter, click **Export to CSV**.

---

## Call 4: Resilience & Grand Finale (Levels 10, 11, 12) — Target: Hour 6.5

### Level 10: Remote Config Updates (OTA Settings)
- **Requirements:** Dashboard parameter change applied by ESP32 without reboot.
- **Judge Test Action:** Judge changes High Temp threshold or toggles buzzer mute on dashboard.
- **How to Execute:**
  1. Adjust slider on dashboard $\rightarrow$ ESP32 polls and applies new setting within 8s $\rightarrow$ Verified on Serial Monitor.

### Level 11: Network Failure & Offline Resilience
- **Requirements:** System continues working when Wi-Fi is cut; buffered events batch-upload upon reconnection.
- **Judge Test Action:** Wi-Fi hotspot turned off. Member taps card and toggles door. Hotspot turned back on.
- **How to Execute:**
  1. Turn off Wi-Fi $\rightarrow$ RFID taps continue to unlock door and beep locally. Events saved to LittleFS queue (`offline_events.jsonl`).
  2. Turn on Wi-Fi $\rightarrow$ ESP32 reconnects and batch-uploads all buffered events with original timestamps.

### Level 12: Multi-Event Stress & Power Interruption Recovery
- **Requirements:** Simultaneous events (fire alert + RFID tap) under power recovery.
- **Judge Test Action:** Power cable unplugged and reconnected while events occur.
- **How to Execute:**
  1. Power interrupted $\rightarrow$ Reconnects in 3s $\rightarrow$ NVS loads last door state $\rightarrow$ Sensors resume without deadlock.

