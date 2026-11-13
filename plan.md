# Operational Game Plan & Feature Reasoning — FastRobox 1.0

## 1. Feature Inclusion Reasoning Matrix
Every widget on the dashboard and feature in the firmware maps directly to points in the 12 levels:

| Component / Feature | Included? | Points / Level Mapping | Reasoning & Competition Justification |
|---|---|---|---|
| **Off-White Background (`#FAF9F6`)** | ✅ YES | Presentation / UI Score | Provides high-contrast, clean, eye-friendly layout under intense competition lighting. |
| **Shadow-Separated Cards (`shadow-lg`)** | ✅ YES | Presentation / UI Score | Creates clear visual depth and clean separation without harsh lines or messy borders. |
| **Simulated Door Card (`LOCKED` / `UNLOCKED`)** | ✅ YES | Levels 2, 5, 6, 8, 10 | Primary classroom access element. Shows live door status, toggle button, and time since last change. |
| **Sensor Gauges (Temp, Humidity, Flame, RSSI)** | ✅ YES | Levels 2, 3, 4, 7 | Core environmental monitoring required by mission overview. Instant red alarm when flame goes LOW. |
| **Emergency Alert Banner** | ✅ YES | Levels 7, 8, 12 | Top banner that flashes during fire, panic touch, or intruder alert. Includes audio mute button. |
| **Attendance Check-in Table** | ✅ YES | Levels 5, 6, 9 | Displays student check-in, teacher events, timestamps, and an **Export to CSV** button for evidence. |
| **Searchable Event Log with Filters** | ✅ YES | Level 9 | Allows judges to search by date, event type (`fire_alarm`, `rfid_student`, `panic_pad`), or card UID. |
| **Remote Config Panel** | ✅ YES | Level 10 | Allows remote updating of temp alert threshold, buzzer enable/disable, and auto-lock timeout. |
| **Camera Vision Snapshot Viewer** | ✅ YES | Levels 6, 8 | Displays latest captured photo from ESP32-CAM (student check-in or intruder capture). |
| **Display Surfaces Readiness Checker** | ✅ YES | Rule 3.3 | Live checklist verifying Serial Monitor, Local Page, Dashboard, Hardware, and DB are ready for judges. |
| **Dark Mode Switch** | ❌ NO | None | Adds zero marks, increases bundle size and introduces potential styling bugs. Strictly omitted. |
| **Complex User Auth / Sign-in** | ❌ NO | None | No login requirement exists in the rulebook; authentication adds friction during judge evaluations. |

---

## 2. Master 4-Call Judging Strategy (500 Marks Optimization)

Remember: **You are allowed a maximum of 4 judging calls in 7 hours!** If you fail one level during a call, all subsequent levels in that call are canceled. Therefore, **each call must be rehearsed 3 times with a phone stopwatch before raising your hand.**

```mermaid
gantt
    title 7-Hour Competition Timeline & 4 Judging Calls
    dateFormat X
    axisFormat %s hr

    section Foundation & Setup
    Kit inspection, wiring, WiFi portal test :0, 60
    Rehearse Call 1 (Levels 1, 2, 3)         :60, 90
    Call 1 Judging (Levels 1, 2, 3)          :90, 110

    section Cloud & Access
    Test Firebase sync, RFID, Door locking   :110, 180
    Rehearse Call 2 (Levels 4, 5, 6)         :180, 205
    Call 2 Judging (Levels 4, 5, 6)          :205, 230

    section Safety & Intelligence
    Test Flame alarm, Touch panic, Event log :230, 290
    Rehearse Call 3 (Levels 7, 8, 9)         :290, 315
    Call 3 Judging (Levels 7, 8, 9)          :315, 340

    section Resilience & Final Wrap-up
    Test Remote config, Offline queue, Crash :340, 380
    Rehearse Call 4 (Levels 10, 11, 12)       :380, 400
    Call 4 Judging (Levels 10, 11, 12)       :400, 420
```

### Call 1 (Target: Hour 1.5) — Foundation Group (Levels 1, 2, 3)
* **Level 1:** Board boot, Serial Monitor welcome output, Captive portal opens when unconfigured, connects to judge's Wi-Fi network smoothly without hardcoding.
* **Level 2:** Local web server (`http://<ip>/`) loads cleanly on phone or laptop, displaying device IP, RSSI, and simulated door status.
* **Level 3:** DHT11 temperature/humidity values and flame sensor status live updating on Serial Monitor and Local Web Page.

### Call 2 (Target: Hour 3.5) — Cloud & Access Group (Levels 4, 5, 6)
* **Level 4:** Sensor telemetry syncing to Firebase Firestore and appearing on the public HTTPS web dashboard.
* **Level 5:** White RFID card tap $\rightarrow$ Buzzer beeps, onboard NeoPixel turns GREEN, door status changes to `UNLOCKED`, logs to Firestore.
* **Level 6:** Camera capture upon access or button press; attendance record logged with card UID and timestamp.

### Call 3 (Target: Hour 5.2) — Safety & Intelligence Group (Levels 7, 8, 9)
* **Level 7:** Fire detection (lighter/flame near sensor) triggers continuous buzzer alarm and dashboard emergency banner within $<3$ seconds. Touch wire contact triggers panic alarm.
* **Level 8:** Unregistered RFID tag or unrecognized trigger logs an `unauthorized_access` alert, flashes RED, and sounds warning beeps.
* **Level 9:** Event log viewer on dashboard filterable by date, type, and card ID with working CSV export.

### Call 4 (Target: Hour 6.5) — Resilience & Advanced Scenarios (Levels 10, 11, 12)
* **Level 10:** Remote configuration change on dashboard (e.g. adjust temperature threshold or toggle buzzer mute) received and applied by ESP32 within 10 seconds.
* **Level 11:** Wi-Fi disconnect test: router/hotspot turned off $\rightarrow$ RFID taps and door toggles continue working locally $\rightarrow$ Hotspot re-enabled $\rightarrow$ ESP32 reconnects and batch-uploads buffered events to Firestore without data loss.
* **Level 12:** Full multi-event simulation: simultaneous temperature rise + flame detection + RFID access under power interruption recovery.

---

## 3. Team Member Allocation Checklist (During the 7 Hours)

- **Member 1 (Hardware Lead):** Keeps wires tight, guards against accidental disconnects, positions board and camera with steady non-electronic support.
- **Member 2 (Firmware & Serial Driver):** Manages Arduino IDE, handles USB connection, leads judge communication and Serial Monitor demonstration.
- **Member 3 (Web & Cloud Operator):** Keeps the public HTTPS dashboard active, demonstrates remote config, triggers cloud tests.
- **Member 4 (Timekeeper & Videographer):** Holds phone on tripod, checks 720p resolution, announces Team ID and Level clearly, records stopwatch tests.
- **Member 5 (Evidence & Submission Lead):** Takes screenshots of all 5 surfaces immediately after each pass, fills the 2-page write-up PDF template, creates `git tag level-XX`, pushes to remote repository.
