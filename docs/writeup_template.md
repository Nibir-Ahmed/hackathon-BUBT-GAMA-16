# FastRobox 1.0 Techathon Grand Finale — Write-Up Report Template

> **Mandatory Rule:** Each level requires a **maximum 2-page PDF** submitted within **30 minutes** of judging completion.
> **File naming convention:** `<TeamID>_L<NN>_writeup.pdf` (e.g. `T07_L03_writeup.pdf`).
> **Video file naming:** `<TeamID>_L<NN>_video.mp4` (e.g. `T07_L03_video.mp4`).

---

## Team & Level Information
- **Team ID:** `[Insert Team ID, e.g. T07]`
- **Level Evaluated:** `Level [NN] — [Level Name]`
- **Evaluation Attempt:** `[1st Attempt (100%) / 2nd Attempt (70%)]`
- **Git Commit Hash:** `[Insert Git Commit Hash]`
- **Git Tag:** `level-[NN]`

---

## (1) Design
### 1.1 Architecture & Implementation Summary
*Briefly describe what was engineered in this level, which sensors/modules were activated, and the software logic flow.*

```
[Insert ASCII or Mermaid Flowchart of this Level's Logic]
Example:
RFID Tap (RC522) ──> ESP32-S3 SPI ──> Validate UID ──> Update Door State (UNLOCKED)
                                                      ├──> NeoPixel LED (Solid Green)
                                                      ├──> Active Buzzer (Single Beep)
                                                      └──> Cloud REST Push (Firestore)
```

### 1.2 Hardware Connections & Pin Map
- **ESP32-S3-CAM Pins Used:** `[List GPIO pins active in this level]`
- **Power Configuration:** `[3.3V / 5V jumper split]`

---

## (2) Where it shows
*Insert actual screenshots captured immediately during or after the live judge inspection.*

| Surface | Description of Visible Data | Screenshot Placeholder |
|---|---|---|
| **1. Serial Monitor** | 115200 baud terminal output with timestamps | `[Paste Serial Screenshot Here]` |
| **2. Local Status Page** | Browser window at `http://<esp32-ip>/` showing state | `[Paste Local Web Screenshot Here]` |
| **3. Web Dashboard** | Live HTTPS URL (`https://...vercel.app`) | `[Paste Dashboard Screenshot Here]` |
| **4. Database Records** | Firebase Firestore / RTDB JSON record | `[Paste Firebase Console Screenshot]` |
| **5. Hardware Output** | NeoPixel RGB LED + Buzzer physical photo | `[Paste Photo of ESP32 Board Here]` |

---

## (3) Test results
*Summarize all measured timings and values evaluated by judges with a stopwatch.*

| Test Item Description | Video Timestamp (`MM:SS`) | Measured Value | Allowed Limit | Result |
|---|---|---|---|---|
| **Wi-Fi Portal Connect Latency** | `00:45` | `3.2s` | `< 15.0s` | **PASS** |
| **Door State Change Latency** | `01:20` | `0.4s` | `< 1.0s` | **PASS** |
| **Flame Sensor Alarm Trigger** | `02:10` | `0.6s` | `< 2.0s` | **PASS** |
| **Firebase Cloud Sync Latency** | `03:05` | `1.1s` | `< 3.0s` | **PASS** |
| **Touch Panic Wire Response** | `04:15` | `0.2s` | `< 0.8s` | **PASS** |

*Note: All tests satisfied the strict stopwatch timing tolerance ($0.5$s margin).*

---

## (4) Problems and fixes
*Document real technical issues faced during this level and the exact engineering solution applied.*

1. **Problem 1 (Pin Conflict / Camera Interference):**
   - *Symptom:* Initial SPI clock pin interfered with camera initialization.
   - *Fix:* Reassigned RC522 SCK to `GPIO 39` and MISO to `GPIO 40`, fully clearing the internal OV2640 DVP bus.

2. **Problem 2 (Wi-Fi Credentials Reset under Judge Test):**
   - *Symptom:* Hardcoded network failed when judges tested on-the-spot Wi-Fi SSID.
   - *Fix:* Implemented `WiFiManager` captive portal saving SSID and Room ID dynamically to Flash NVS via `Preferences.h`.

---

## (5) Extra item
*Address any level-specific extra question or special requirement stated in the judging rubric.*

- **Resilience / Security Consideration:** All credentials and dynamic values are persisted across power interruptions in non-volatile flash storage. Zero secrets or API keys are committed to the public Git repository.

