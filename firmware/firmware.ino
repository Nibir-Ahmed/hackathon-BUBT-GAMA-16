// AI-Powered Smart Classroom: Safety, Access and Vision
// FastRobox 1.0 Techathon Grand Finale (BUBT) — Levels 1, 2 & 3
// Microcontroller: ESP32-S3-N16R8 CAM

#include <Arduino.h>
#include <esp_task_wdt.h>
#include <esp_system.h>
#include <esp_idf_version.h>
#include <soc/soc.h>
#include <soc/rtc_cntl_reg.h>
#include <LittleFS.h>
#include "config.h"
#include "device_signals.h"
#include "door_control.h"
#include "sensors.h"
#include "rfid_access.h"
#include "wifi_portal.h"
#include "cloud_client.h"
#include "local_webserver.h"

// Global Controller Instances
DeviceSignalsManager signals;
DoorController door;
SensorManager sensors;
RFIDManager rfidManager;
WiFiPortalManager wifiPortal;
CloudClientManager cloud;
LocalWebServerManager localServer;

TaskHandle_t taskCore0Handle = NULL;
TaskHandle_t taskCore1Handle = NULL;
TaskHandle_t taskSignalHandle = NULL;

enum SafetyStatus {
    SAFETY_SAFE = 0,
    SAFETY_WARNING,
    SAFETY_DANGER
};

// System State
unsigned long bootTimestamp = 0;
bool deliberateHang = false;
RTC_NOINIT_ATTR uint32_t rtcWdtHangMagic;
String cachedBootReason = "POWER_ON";

// Level 3 Safety State
SafetyStatus currentSafetyStatus = SAFETY_SAFE;
SafetyStatus pendingSafetyStatus = SAFETY_SAFE;
unsigned long pendingSafetySince = 0;

bool flamePresent = false;
unsigned long flameContinuousSince = 0;
bool fireSilenced = false;
unsigned long panicBannerUntil = 0;
unsigned long silenceRefusedUntil = 0;

// Sensor Health Tracking
unsigned long lastDhtSuccess = 0;
unsigned long lastRfidSuccess = 0;
bool dhtErrorActive = false;
bool rfidErrorActive = false;

const char* safetyStatusName(SafetyStatus s) {
    switch (s) {
        case SAFETY_WARNING: return "WARNING";
        case SAFETY_DANGER:  return "DANGER";
        case SAFETY_SAFE:
        default:             return "SAFE";
    }
}

const char* getSafetyStatusString() {
    if (door.isFireLatched() || flamePresent) return "FIRE";
    if (dhtErrorActive || rfidErrorActive) return "SENSOR_ERROR";
    return safetyStatusName(currentSafetyStatus);
}

bool isPanicBannerActive() {
    return millis() < panicBannerUntil;
}

bool isSilenceRefusedActive() {
    return millis() < silenceRefusedUntil;
}

unsigned long getUptimeSeconds() {
    return (millis() - bootTimestamp) / 1000UL;
}

float getChipTemperature() {
#ifdef ESP32
    return temperatureRead();
#else
    return 41.5f;
#endif
}

void performFireReset(const char* source) {
    if (flamePresent) {
        Serial.println("[DOOR] Cannot reset fire while flame is still present!");
        return;
    }
    fireSilenced = false;
    door.setFireLatched(false);
    signals.setFire(false, false);
    cloud.enqueueEvent("FIRE_CLEARED", String("Fire reset by ") + source);
}

String getResetReasonString() {
    return cachedBootReason;
}

String computeResetReasonString() {
    esp_reset_reason_t reason = esp_reset_reason();
    if (rtcWdtHangMagic == 0x57445431UL && reason != ESP_RST_POWERON) {
        rtcWdtHangMagic = 0;
        return "WATCHDOG";
    }
    rtcWdtHangMagic = 0;
    switch (reason) {
        case ESP_RST_POWERON:  return "POWER_ON";
        case ESP_RST_SW:       return "SOFTWARE";
        case ESP_RST_TASK_WDT:
        case ESP_RST_INT_WDT:
        case ESP_RST_WDT:      return "WATCHDOG";
        case ESP_RST_PANIC:    return "CRASH";
        case ESP_RST_BROWNOUT: return "BROWNOUT";
        default:               return "SOFTWARE";
    }
}

void printStatusLine() {
    SensorReadings s = sensors.getLatest();
    unsigned long upSec = getUptimeSeconds();
    String rssiStr = wifiPortal.isConnected() ? String(wifiPortal.getRSSI()) : "-";
    String tStr = s.dhtValid ? String(s.temperature, 1) : (s.lastReadTime > 0 ? String(s.temperature, 1) : "-");
    String rhStr = s.dhtValid ? String((int)roundf(s.humidity)) : (s.lastReadTime > 0 ? String((int)roundf(s.humidity)) : "-");
    String hiStr = s.dhtValid ? String(s.heatIndex, 1) : (s.lastReadTime > 0 ? String(s.heatIndex, 1) : "-");

    Serial.printf("[STATUS] up=%lus T=%sC RH=%s%% HI=%sC flame=%d door=%s state=%s trend=%s rssi=%s heap=%uk psram=%uk chip=%.1fC fw=%s\n",
                  upSec,
                  tStr.c_str(),
                  rhStr.c_str(),
                  hiStr.c_str(),
                  s.flameDetected ? 1 : 0,
                  door.getStateString(),
                  getSafetyStatusString(),
                  sensors.getTrendString(),
                  rssiStr.c_str(),
                  ESP.getFreeHeap() / 1024,
                  ESP.getFreePsram() / 1024,
                  getChipTemperature(),
                  FIRMWARE_VERSION);
}

void handleSerialCommands() {
    static char cmdBuf[128];
    static size_t cmdLen = 0;

    while (Serial.available()) {
        char c = (char)Serial.read();
        if (c == '\r') continue;
        if (c == '\n') {
            cmdBuf[cmdLen] = '\0';
            String cmd = String(cmdBuf);
            cmd.trim();
            cmdLen = 0;
            if (cmd.length() == 0) return;

            if (cmd.equalsIgnoreCase("help")) {
                Serial.println("[CMD] Commands: help, status, health, tasks, ver, hang, reset, csi, teacher add <UID>, teacher list, teacher clear, student add <UID>, firereset");
            } else if (cmd.equalsIgnoreCase("status")) {
                Serial.print("[CMD] ");
                printStatusLine();
            } else if (cmd.equalsIgnoreCase("health")) {
                Serial.printf("[CMD] chip=%.1fC heap=%uKB psram=%uKB flash=%uMB cpu=%uMHz rssi=%d dBm up=%lus reset=%s fw=%s\n",
                              getChipTemperature(),
                              ESP.getFreeHeap() / 1024,
                              ESP.getFreePsram() / 1024,
                              ESP.getFlashChipSize() / (1024 * 1024),
                              ESP.getCpuFreqMHz(),
                              wifiPortal.getRSSI(),
                              getUptimeSeconds(),
                              getResetReasonString().c_str(),
                              FIRMWARE_VERSION);
            } else if (cmd.equalsIgnoreCase("tasks")) {
                Serial.printf("[CMD] NetWorker: Core=0 FreeStack=%u bytes\n",
                              taskCore0Handle ? (unsigned int)uxTaskGetStackHighWaterMark(taskCore0Handle) : 0);
                Serial.printf("[CMD] SignalWorker: Core=0 FreeStack=%u bytes\n",
                              taskSignalHandle ? (unsigned int)uxTaskGetStackHighWaterMark(taskSignalHandle) : 0);
                Serial.printf("[CMD] SensorWorker: Core=1 FreeStack=%u bytes\n",
                              taskCore1Handle ? (unsigned int)uxTaskGetStackHighWaterMark(taskCore1Handle) : 0);
            } else if (cmd.equalsIgnoreCase("ver")) {
                Serial.printf("[CMD] fw=%s build=%s\n", FIRMWARE_VERSION, FIRMWARE_BUILD_DATE);
            } else if (cmd.equalsIgnoreCase("hang")) {
                Serial.println("[CMD] Freezing SensorWorker task... Watchdog reset in <=8s");
                rtcWdtHangMagic = 0x57445431UL;
                deliberateHang = true;
                // Guaranteed hardware reset fallback at 8.5s if TWDT doesn't reset cleanly
                xTaskCreate([](void*){
                    vTaskDelay(pdMS_TO_TICKS(8500));
                    REG_SET_BIT(RTC_CNTL_OPTIONS0_REG, RTC_CNTL_SW_SYS_RST);
                    vTaskDelete(NULL);
                }, "HangFallback", 2048, NULL, 5, NULL);
                while (deliberateHang) {
                    delay(100);
                }
            } else if (cmd.equalsIgnoreCase("reset")) {
                Serial.println("[CMD] Clearing saved Wi-Fi details and restarting in setup mode...");
                wifiPortal.resetSettings();
            } else if (cmd.equalsIgnoreCase("csi")) {
                Serial.println("[CMD] CSI not built yet (Level 11)");
            } else if (cmd.startsWith("teacher add ")) {
                String uid = cmd.substring(12);
                rfidManager.addTeacherTag(uid);
                Serial.printf("[CMD] Teacher tag added: %s\n", uid.c_str());
            } else if (cmd.equalsIgnoreCase("teacher list")) {
                Serial.printf("[CMD] Teacher tags (%d):", (int)rfidManager.getTeacherTags().size());
                for (const auto& t : rfidManager.getTeacherTags()) Serial.printf(" %s", t.c_str());
                Serial.println();
            } else if (cmd.equalsIgnoreCase("teacher clear")) {
                rfidManager.clearTeacherTags();
                Serial.println("[CMD] Teacher tags cleared");
            } else if (cmd.startsWith("student add ")) {
                String uid = cmd.substring(12);
                rfidManager.addStudentCard(uid);
                Serial.printf("[CMD] Student card added: %s\n", uid.c_str());
            } else if (cmd.equalsIgnoreCase("firereset")) {
                performFireReset("Serial CMD");
                Serial.println("[CMD] Fire reset requested");
            } else {
                Serial.printf("[CMD] Unknown command: %s\n", cmd.c_str());
            }
        } else if (cmdLen < sizeof(cmdBuf) - 1) {
            cmdBuf[cmdLen++] = c;
        }
    }
}

void TaskCore0(void *pvParameters) {
    esp_task_wdt_add(NULL);
    for (;;) {
        if (!deliberateHang) {
            esp_task_wdt_reset();
        }
#if ENABLE_LEVEL_2_WIFI
        wifiPortal.update();
        localServer.handleClient();
        cloud.update(getSafetyStatusString(), getUptimeSeconds(), getChipTemperature());
#endif
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void TaskSignal(void *pvParameters) {
    esp_task_wdt_add(NULL);
    for (;;) {
        if (!deliberateHang) {
            esp_task_wdt_reset();
        }
        signals.update();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void TaskCore1(void *pvParameters) {
    esp_task_wdt_add(NULL);

    unsigned long lastStatusPrint = 0;
    unsigned long lastFlameRead = 0;
    unsigned long lastRfidPoll = 0;
    unsigned long lastDhtRead = 0;
    unsigned long lastTouchPoll = 0;
    unsigned long bootBtnDownSince = 0;

    for (;;) {
        if (!deliberateHang) {
            esp_task_wdt_reset();
        }
        unsigned long now = millis();

        // 1. Door auto-lock (signals.update() runs in dedicated SignalWorker task)
        door.update();

        // 2. Non-blocking Serial CLI
        handleSerialCommands();

        // 3. BOOT button 5s hold -> clear saved Wi-Fi & restart in setup mode
        if (digitalRead(PIN_BOOT_BTN) == LOW) {
            if (bootBtnDownSince == 0) bootBtnDownSince = now;
            else if (now - bootBtnDownSince >= BOOT_RESET_HOLD_MS) {
                Serial.println("[NET] BOOT held 5s -> clearing credentials");
                wifiPortal.resetSettings();
            }
        } else {
            bootBtnDownSince = 0;
        }

        // 4. Touch panic pad every 100ms (2s hold -> PANIC, <1s ignored, one-shot per touch)
        if (now - lastTouchPoll >= 100) {
            lastTouchPoll = now;
            if (sensors.pollTouchPanic()) {
                panicBannerUntil = now + 10000UL;
                signals.triggerPanic();
                cloud.enqueueEvent("PANIC", "Touch panic pad held 2s");
            }
        }

        // 5. Flame sensor every 100 ms (2s continuous -> FIRE)
        if (now - lastFlameRead >= FLAME_READ_INTERVAL_MS) {
            lastFlameRead = now;
            bool isFlame = sensors.readFlame();

            if (isFlame) {
                if (flameContinuousSince == 0) flameContinuousSince = now;
                if (!flamePresent && (now - flameContinuousSince >= FLAME_HOLD_MS)) {
                    flamePresent = true;
                    fireSilenced = false;
                    door.setFireLatched(true);
                    signals.setFire(true, false);
                    cloud.enqueueEvent("FIRE", "Flame continuous 2s -> Door UNLOCKED");
                }
            } else {
                flameContinuousSince = 0;
                if (flamePresent) {
                    flamePresent = false;
                    cloud.enqueueEvent("FIRE_CLEARED", "Flame gone (door stays UNLOCKED until Reset fire)");
                }
            }
        }

        // 6. RFID reader every 200 ms
        if (now - lastRfidPoll >= RFID_POLL_INTERVAL_MS) {
            lastRfidPoll = now;

            if (rfidManager.isReaderConnected()) {
                lastRfidSuccess = now;
                if (rfidErrorActive) {
                    rfidErrorActive = false;
                    Serial.println("[INFO] SENSOR RECOVERED: RFID");
                    signals.setSensorError(dhtErrorActive);
                }
            } else if (now - lastRfidSuccess >= 2000) {
                static unsigned long lastRfidErrPrint = 0;
                if (!rfidErrorActive) {
                    rfidErrorActive = true;
                    lastRfidErrPrint = now;
                    Serial.println("[ERROR] SENSOR ERROR: RFID");
                    cloud.enqueueEvent("SENSOR_ERROR", "RFID");
                    signals.setSensorError(true);
                } else if (now - lastRfidErrPrint >= 5000) {
                    lastRfidErrPrint = now;
                    Serial.println("[ERROR] SENSOR ERROR: RFID");
                }
            }

            RFIDScanResult card = rfidManager.poll();
            if (card.hasCard) {
                Serial.printf("[CARD] uid=%s role=%s\n", card.uidString.c_str(), card.roleName);
                cloud.enqueueEvent("CARD_TAP", "uid=" + card.uidString);

                if (door.isFireLatched()) {
                    if (card.role == ROLE_TEACHER) {
                        if (flamePresent) {
                            silenceRefusedUntil = now + 4000UL;
                            Serial.println("[EVENT] SILENCE_REFUSED CANNOT SILENCE - FLAME PRESENT");
                        } else if (!fireSilenced) {
                            fireSilenced = true;
                            signals.setFire(true, true);
                            cloud.enqueueEvent("SILENCED", "Teacher tag " + card.uidString);
                        }
                    } else {
                        Serial.println("[EVENT] SILENCE_REFUSED Student/unknown card cannot silence fire");
                    }
                } else {
#if ENABLE_LEVEL_2_WIFI
                    if (card.role == ROLE_TEACHER || card.role == ROLE_STUDENT) {
                        door.unlock(card.roleName);
                        signals.triggerAccessGranted();
                        cloud.enqueueEvent("ENTRY_GRANTED", String(card.roleName) + " " + card.uidString);
                    } else {
                        signals.triggerAccessDenied();
                        cloud.enqueueEvent("ENTRY_DENIED", "UNKNOWN_CARD " + card.uidString);
                    }
#endif
                }
            }
        }

        // 7. DHT11 every 5 seconds (Level 3 status, hysteresis, quality, trend)
        if (now - lastDhtRead >= DHT_READ_INTERVAL_MS || lastDhtRead == 0) {
            lastDhtRead = now;
            bool ok = sensors.readDHT();

            if (ok) {
                lastDhtSuccess = now;
                if (dhtErrorActive) {
                    dhtErrorActive = false;
                    Serial.println("[INFO] SENSOR RECOVERED: DHT11");
                    signals.setSensorError(rfidErrorActive);
                }

                SensorReadings s = sensors.getLatest();
                SafetyStatus target = currentSafetyStatus;

                if (currentSafetyStatus == SAFETY_SAFE) {
                    if (s.temperature >= TEMP_DANGER_DEFAULT) target = SAFETY_DANGER;
                    else if (s.temperature >= TEMP_WARN_DEFAULT || s.humidity >= HUMID_WARN_DEFAULT) target = SAFETY_WARNING;
                } else if (currentSafetyStatus == SAFETY_WARNING) {
                    if (s.temperature >= TEMP_DANGER_DEFAULT) target = SAFETY_DANGER;
                    else if (s.temperature <= (TEMP_WARN_DEFAULT - TEMP_HYSTERESIS) &&
                             s.humidity <= (HUMID_WARN_DEFAULT - HUMID_HYSTERESIS)) {
                        target = SAFETY_SAFE;
                    }
                } else if (currentSafetyStatus == SAFETY_DANGER) {
                    if (s.temperature <= (TEMP_DANGER_DEFAULT - TEMP_HYSTERESIS)) {
                        target = SAFETY_WARNING;
                    }
                }

                if (target != currentSafetyStatus) {
                    if (pendingSafetyStatus != target) {
                        pendingSafetyStatus = target;
                        pendingSafetySince = now;
                    } else if (now - pendingSafetySince >= STATUS_HOLD_MS) {
                        SafetyStatus oldStatus = currentSafetyStatus;
                        currentSafetyStatus = target;
                        pendingSafetySince = 0;
                        signals.setHeatStatus(currentSafetyStatus == SAFETY_WARNING,
                                              currentSafetyStatus == SAFETY_DANGER);
                        cloud.enqueueEvent("STATUS",
                            String(safetyStatusName(oldStatus)) + "->" + String(safetyStatusName(currentSafetyStatus)));
                    }
                } else {
                    pendingSafetyStatus = currentSafetyStatus;
                    pendingSafetySince = 0;
                }
            } else {
                // Bad/impossible reading: keep last status, reset pending transition
                pendingSafetyStatus = currentSafetyStatus;
                pendingSafetySince = 0;
                if (now - lastDhtSuccess >= 9000) {
                    if (!dhtErrorActive) {
                        dhtErrorActive = true;
                        Serial.println("[ERROR] SENSOR ERROR: DHT11");
                        cloud.enqueueEvent("SENSOR_ERROR", "DHT11");
                        signals.setSensorError(true);
                    } else {
                        Serial.println("[ERROR] SENSOR ERROR: DHT11");
                    }
                }
            }
        }

        // 8. Print [STATUS] every 1 second
        if (now - lastStatusPrint >= 1000) {
            lastStatusPrint = now;
            printStatusLine();
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Hardware-reset the ESP32-S3 cleanly when Task Watchdog fires (avoids abort() halting on USB-CDC)
extern "C" void esp_task_wdt_isr_user_handler(void) {
    REG_SET_BIT(RTC_CNTL_OPTIONS0_REG, RTC_CNTL_SW_SYS_RST);
}

void setup() {
    bootTimestamp = millis();
    lastDhtSuccess = bootTimestamp;
    lastRfidSuccess = bootTimestamp;

    Serial.begin(115200);
    unsigned long serialWaitStart = millis();
    while (!Serial && (millis() - serialWaitStart < 2000)) {
        delay(10);
    }
    delay(200);

    pinMode(PIN_BOOT_BTN, INPUT_PULLUP);

    if (!LittleFS.begin(true)) {
        Serial.println("[ERROR] LittleFS mount failed!");
    }

    signals.begin();
    door.begin();
    sensors.begin();
    rfidManager.begin();
    cloud.begin();

    cachedBootReason = computeResetReasonString();
    String bootReason = getResetReasonString();
    Serial.printf("[EVENT] BOOT reason=%s\n", bootReason.c_str());
    cloud.enqueueEvent("BOOT", "reason=" + bootReason);

    // Boot flame check & NVS fire-latch restore
    bool bootFlame = sensors.readFlame();
    if (bootFlame || door.isFireLatched()) {
        flamePresent = bootFlame;
        door.setFireLatched(true);
        signals.setFire(true, false);
        if (bootFlame) {
            cloud.enqueueEvent("FIRE", "Flame at startup -> Door UNLOCKED");
        } else {
            Serial.println("[DOOR] Restored FIRE latch from flash -> Door UNLOCKED");
        }
    } else {
        door.lock("Boot Safe");
    }

#if ENABLE_LEVEL_2_WIFI
    wifiPortal.begin();
    localServer.begin();
#endif

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = WDT_TIMEOUT_MS,
        .idle_core_mask = 0,
        .trigger_panic = false
    };
    if (esp_task_wdt_reconfigure(&twdt_config) != ESP_OK) {
        esp_task_wdt_init(&twdt_config);
    }
#else
    esp_task_wdt_init(WDT_TIMEOUT_MS / 1000, false);
#endif

    xTaskCreatePinnedToCore(TaskCore0, "NetWorker", 8192, NULL, 1, &taskCore0Handle, 0);
    xTaskCreatePinnedToCore(TaskSignal, "SignalWorker", 4096, NULL, 2, &taskSignalHandle, 0);
    xTaskCreatePinnedToCore(TaskCore1, "SensorWorker", 8192, NULL, 2, &taskCore1Handle, 1);

    Serial.println("[SYSTEM] Dual-core FreeRTOS tasks started.");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
