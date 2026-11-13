# FastRobox 1.0 Techathon Grand Finale (BUBT) — Master Context Document

## 1. Executive Summary
- **Competition:** FastRobox 1.0 Techathon Grand Finale
- **Organizer:** Bangladesh University of Business and Technology (BUBT)
- **Duration:** 7 Hours continuous on-site competition
- **Total Marks:** 500 points (Distributed across 12 sequential levels, max 50 points/level, NO bonus marks)
- **Target Project:** "AI-Powered Smart Classroom: Safety, Access and Vision"
- **Primary Microcontroller:** 1x ESP32-S3-CAM board (Provided on-site)
- **Team Size:** 5 Members

---

## 2. Hardware Kit Inventory & Constraints

### Official Provided Hardware (ONLY these 6 items permitted)
| # | Item Name | Quantity | Role in the Build |
|---|-----------|----------|-------------------|
| 1 | **ESP32-S3-CAM** (Camera, Onboard RGB LED, BOOT Button) | 1 pcs | Main Controller, Camera Stream/Snapshot, Status Light, Wi-Fi & BLE |
| 2 | **RC522 RFID Reader** (13.56 MHz) + 1 White Card + 1 Blue Key-tag | 1 set | Student Attendance Check-in (White Card) & Teacher Administrative Actions (Blue Tag) |
| 3 | **DHT11 Sensor** | 1 pcs | Ambient Temperature & Humidity Monitoring |
| 4 | **IR Flame Sensor Module** (3-pin: VCC, GND, DO) | 1 pcs | Fire / Flame Detection (Active LOW DO) |
| 5 | **Active Buzzer** (2-pin) | 1 pcs | Audio Alarm & Feedback Beeper |
| 6 | **Jumper Wires** | 1 set | Wire-to-wire interconnects + Touch Panic Pad (one jumper connected to a Touch Pin with exposed metallic tip) |

### Strict Physical Restrictions:
- **NO Breadboard allowed!** Wire connections must be made directly using jumper wires (female-to-female or pin-to-pin).
- **NO 2nd microcontroller or extra electronic modules.**
- **Non-electronic supports allowed:** Books, tape, or cardboard boxes may be used to mount the board and point the camera stably.
- **Teams MUST bring their own:** Data-supported USB Cable (Type-C or Micro-USB depending on the board), laptops, phone hotspots, power strips, and cloud accounts.

---

## 3. Ground Rules & Judging Constraints

### 3.1 Judging Calls (MAXIMUM 4 Calls in 7 Hours)
- A team can call judges a maximum of **4 times** during the entire 7 hours.
- **Consecutive Levels Announcement:** In each call, you must declare consecutive levels (e.g., Call 1: Level 1, 2, 3).
- **Mid-Call Failure:** If Level 2 fails during a call for Levels 1, 2, 3, judging stops immediately. Level 3 will NOT be evaluated in that call.
- **2 Attempts per Level:**
  - 1st Attempt Pass: **100% full marks**.
  - 2nd Attempt Pass: **70% marks** (max 1 decimal place).
  - Both Attempts Fail: **Closure Rule** — That level and ALL subsequent levels are permanently CLOSED. The team cannot progress further!
- **Timing & 0.5s Tolerance:** Judges measure latency with a stopwatch. If a test exceeds the target time by $\le 0.5$ seconds, the judge grants an immediate re-test and takes the better time.

### 3.2 Non-Negotiable Technical Constraints
1. **No Hard-Coding:** Judges will supply on-the-spot Wi-Fi SSID, Password, Room ID, and Class Start Time. Hardcoded credentials will cause instant failure.
   - *Solution:* Firmware integrates a Captive Portal (WiFiManager) + local NVS storage so values can be input from a phone in 15 seconds.
2. **No Secrets in Code:** Passwords, API keys, and private tokens must never appear in firmware source, web code, or git repositories.
3. **Simulated Door:** No physical motor exists. Door state (`LOCKED` or `UNLOCKED`) must be managed in software and visibly reflected simultaneously across **5 display surfaces**:
   - Surface 1: Serial Monitor (Laptop)
   - Surface 2: Local Status Web Page (`http://<esp32-local-ip>`)
   - Surface 3: Live Web Dashboard (Public HTTPS)
   - Surface 4: Hardware Outputs (Onboard RGB LED + Buzzer)
   - Surface 5: Cloud Database Records (Firestore / Realtime DB)
4. **Public HTTPS Web Dashboard:** Must be accessible over the internet via HTTPS (hosted on Vercel, Netlify, or Firebase Hosting).
5. **Full Git History & Tags:** Code must have regular commits and git tags formatted as `level-01`, `level-02`, ..., `level-12`. `README.md` must be updated starting from Level 2.
6. **30-Minute Submission Deadline:** Within 30 minutes of completing judging, upload:
   - `<TeamID>_L<NN>_video.mp4` (720p+, max 10 mins, clock/stopwatch visible, Team ID & Level spoken).
   - `<TeamID>_L<NN>_writeup.pdf` (Max 2 pages with 5 mandatory headings).

---

## 4. Team Structure (5 People Strategy)

| Member | Role | Primary Responsibility |
|---|---|---|
| **Member 1** | **Hardware & Circuit Lead** | Jumper wire interconnects, sensor pin routing, camera mounting, physical stability without breadboard |
| **Member 2** | **Firmware & ESP32 Flasher** | Arduino IDE code flashing, Serial Monitor logging, live check demonstrations to judges |
| **Member 3** | **Cloud & Web Dashboard Lead** | Firebase console management, public HTTPS dashboard operation, remote config tweaks |
| **Member 4** | **Video Recorder & Timekeeper** | 720p+ video recording with visible stopwatch, vocal team ID announcements, video file exports |
| **Member 5** | **Evidence & Write-Up Specialist** | 2-page PDF report generation, Git commits & tagging (`level-NN`), final uploads within 30 mins |
