// device_signals.h — Single-Owner Priority-Based RGB LED & Buzzer Controller
// Implements Official Device Signals Table (Priority 1 = highest)

#ifndef DEVICE_SIGNALS_H
#define DEVICE_SIGNALS_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"

enum DeviceSignalState {
    STATE_FIRE = 1,                 // Red 5 Hz, buzzer 200ms on/off continuous
    STATE_PANIC = 2,                // Red & Blue alternating, buzzer 100ms on/off 10s
    STATE_ACCESS_GRANTED = 3,       // Solid green 5s, 1 short beep
    STATE_ACCESS_DENIED = 4,        // 3 quick red flashes, 2 short beeps
    STATE_HEAT_DANGER = 5,          // Red 2 Hz heartbeat, 1 short beep every 2s
    STATE_SENSOR_ERROR = 6,         // Magenta 2 Hz heartbeat
    STATE_CAMERA_FAULT = 7,         // Cyan 2 Hz heartbeat
    STATE_WEATHER_DANGER = 8,       // Purple 2 Hz heartbeat, 3 beeps once
    STATE_WARNING = 9,              // Yellow 2 Hz heartbeat
    STATE_WIFI_SETUP_OFFLINE = 10,  // Blue 2 Hz heartbeat
    STATE_SAFE = 11                 // Green 2 Hz heartbeat (250ms on / 250ms off)
};

class DeviceSignalsManager {
private:
    Adafruit_NeoPixel pixel;
    uint8_t buzzerPin;
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

    // Independent condition flags
    bool fireActive;
    bool fireSilenced;
    unsigned long panicUntil;
    unsigned long accessGrantedUntil;
    unsigned long accessDeniedUntil;
    bool heatDanger;
    bool sensorError;
    bool cameraFault;
    bool weatherDanger;
    bool weatherBeeped;
    bool heatWarning;
    bool wifiSetupOrOffline;

    // Active rendering state
    DeviceSignalState activeState;
    unsigned long stateStartTime;
    unsigned long lastLedToggle;
    unsigned long lastBuzzerToggle;
    bool ledOn;
    bool buzzerOn;
    int beepStep;
    uint32_t lastAppliedColor;

    DeviceSignalState computeHighestPriority(unsigned long now) const {
        if (fireActive) return STATE_FIRE;
        if (now < panicUntil) return STATE_PANIC;
        if (now < accessGrantedUntil) return STATE_ACCESS_GRANTED;
        if (now < accessDeniedUntil) return STATE_ACCESS_DENIED;
        if (heatDanger) return STATE_HEAT_DANGER;
        if (sensorError) return STATE_SENSOR_ERROR;
        if (cameraFault) return STATE_CAMERA_FAULT;
        if (weatherDanger) return STATE_WEATHER_DANGER;
        if (heatWarning) return STATE_WARNING;
        if (wifiSetupOrOffline) return STATE_WIFI_SETUP_OFFLINE;
        return STATE_SAFE;
    }

    void setPixelColor(uint32_t color) {
        if (color != lastAppliedColor) {
            lastAppliedColor = color;
            pixel.setPixelColor(0, color);
            pixel.show();
        }
    }

    void handleHeartbeat(uint32_t color, unsigned long now) {
        if (now - lastLedToggle >= 250) {
            lastLedToggle = now;
            ledOn = !ledOn;
            setPixelColor(ledOn ? color : pixel.Color(0, 0, 0));
        }
    }

public:
    DeviceSignalsManager() :
        pixel(1, PIN_RGB_NEOPIXEL, NEO_GRB + NEO_KHZ800),
        buzzerPin(PIN_BUZZER),
        fireActive(false),
        fireSilenced(false),
        panicUntil(0),
        accessGrantedUntil(0),
        accessDeniedUntil(0),
        heatDanger(false),
        sensorError(false),
        cameraFault(false),
        weatherDanger(false),
        weatherBeeped(false),
        heatWarning(false),
        wifiSetupOrOffline(false),
        activeState(STATE_SAFE),
        stateStartTime(0),
        lastLedToggle(0),
        lastBuzzerToggle(0),
        ledOn(false),
        buzzerOn(false),
        beepStep(0),
        lastAppliedColor(0xFFFFFFFF) {}

    void begin() {
        pixel.begin();
        pixel.setBrightness(45);
        pinMode(buzzerPin, OUTPUT);
        digitalWrite(buzzerPin, LOW);
        setPixelColor(pixel.Color(0, 0, 0));
    }

    void setFire(bool active, bool silenced) {
        portENTER_CRITICAL(&mux);
        fireActive = active;
        fireSilenced = silenced;
        portEXIT_CRITICAL(&mux);
    }

    void triggerPanic() {
        portENTER_CRITICAL(&mux);
        panicUntil = millis() + 10000UL;
        if (activeState == STATE_PANIC) {
            stateStartTime = millis();
        }
        portEXIT_CRITICAL(&mux);
    }

    void triggerAccessGranted() {
        portENTER_CRITICAL(&mux);
        accessGrantedUntil = millis() + (DOOR_UNLOCK_TIME_SEC * 1000UL);
        // Force re-trigger of one-shot beep even if already granted
        if (activeState == STATE_ACCESS_GRANTED) {
            stateStartTime = millis();
            beepStep = 0;
        }
        portEXIT_CRITICAL(&mux);
    }

    void triggerAccessDenied() {
        portENTER_CRITICAL(&mux);
        accessDeniedUntil = millis() + 900UL; // 3 quick red flashes (~900ms)
        if (activeState == STATE_ACCESS_DENIED) {
            stateStartTime = millis();
            beepStep = 0;
        }
        portEXIT_CRITICAL(&mux);
    }

    void setHeatStatus(bool warning, bool danger) {
        portENTER_CRITICAL(&mux);
        heatWarning = warning;
        heatDanger = danger;
        portEXIT_CRITICAL(&mux);
    }

    void setSensorError(bool err) {
        portENTER_CRITICAL(&mux);
        sensorError = err;
        portEXIT_CRITICAL(&mux);
    }

    void setCameraFault(bool fault) {
        portENTER_CRITICAL(&mux);
        cameraFault = fault;
        portEXIT_CRITICAL(&mux);
    }

    void setWeatherDanger(bool danger) {
        portENTER_CRITICAL(&mux);
        if (danger && !weatherDanger) weatherBeeped = false;
        weatherDanger = danger;
        portEXIT_CRITICAL(&mux);
    }

    void setWifiSetupOrOffline(bool offline) {
        portENTER_CRITICAL(&mux);
        wifiSetupOrOffline = offline;
        portEXIT_CRITICAL(&mux);
    }

    DeviceSignalState getState() const {
        return activeState;
    }

    void update() {
        unsigned long now = millis();

        portENTER_CRITICAL(&mux);
        DeviceSignalState nextState = computeHighestPriority(now);
        bool silenced = fireSilenced;
        bool needWeatherBeep = (nextState == STATE_WEATHER_DANGER && !weatherBeeped);
        portEXIT_CRITICAL(&mux);

        if (nextState != activeState) {
            activeState = nextState;
            stateStartTime = now;
            lastLedToggle = now;
            lastBuzzerToggle = now;
            ledOn = true;
            buzzerOn = false;
            beepStep = 0;
            digitalWrite(buzzerPin, LOW);
        }

        switch (activeState) {
            case STATE_FIRE:
                // Red, fast flashing (5 Hz = 100ms on / 100ms off)
                if (now - lastLedToggle >= 100) {
                    lastLedToggle = now;
                    ledOn = !ledOn;
                    setPixelColor(ledOn ? pixel.Color(255, 0, 0) : pixel.Color(0, 0, 0));
                }
                if (silenced) {
                    digitalWrite(buzzerPin, LOW);
                    buzzerOn = false;
                } else if (now - lastBuzzerToggle >= 200) {
                    lastBuzzerToggle = now;
                    buzzerOn = !buzzerOn;
                    digitalWrite(buzzerPin, buzzerOn ? HIGH : LOW);
                }
                break;

            case STATE_PANIC:
                // Red and blue alternating
                if (now - lastLedToggle >= 150) {
                    lastLedToggle = now;
                    ledOn = !ledOn;
                    setPixelColor(ledOn ? pixel.Color(255, 0, 0) : pixel.Color(0, 0, 255));
                }
                // Buzzer: 100 ms on / 100 ms off for 10 seconds
                if (now - lastBuzzerToggle >= 100) {
                    lastBuzzerToggle = now;
                    buzzerOn = !buzzerOn;
                    digitalWrite(buzzerPin, buzzerOn ? HIGH : LOW);
                }
                break;

            case STATE_ACCESS_GRANTED:
                // Solid green for 5 seconds
                setPixelColor(pixel.Color(0, 255, 0));
                // One short beep (100ms)
                digitalWrite(buzzerPin, (now - stateStartTime < 100) ? HIGH : LOW);
                break;

            case STATE_ACCESS_DENIED: {
                // Three quick red flashes (150ms on/off x3 = 900ms)
                if (now - lastLedToggle >= 150) {
                    lastLedToggle = now;
                    ledOn = !ledOn;
                    setPixelColor(ledOn ? pixel.Color(255, 0, 0) : pixel.Color(0, 0, 0));
                }
                // Two short beeps (80ms on, 70ms off, 80ms on)
                unsigned long dt = now - stateStartTime;
                bool b = (dt < 80) || (dt >= 150 && dt < 230);
                digitalWrite(buzzerPin, b ? HIGH : LOW);
                break;
            }

            case STATE_HEAT_DANGER:
                // Red, 2 Hz heartbeat
                handleHeartbeat(pixel.Color(255, 0, 0), now);
                // One short beep (100ms) every 2 seconds
                if (now - lastBuzzerToggle >= 2000) {
                    lastBuzzerToggle = now;
                }
                digitalWrite(buzzerPin, (now - lastBuzzerToggle < 100) ? HIGH : LOW);
                break;

            case STATE_SENSOR_ERROR:
                // Magenta, 2 Hz heartbeat
                handleHeartbeat(pixel.Color(255, 0, 255), now);
                digitalWrite(buzzerPin, LOW);
                break;

            case STATE_CAMERA_FAULT:
                // Cyan, 2 Hz heartbeat
                handleHeartbeat(pixel.Color(0, 255, 255), now);
                digitalWrite(buzzerPin, LOW);
                break;

            case STATE_WEATHER_DANGER:
                // Purple, 2 Hz heartbeat
                handleHeartbeat(pixel.Color(160, 32, 240), now);
                if (needWeatherBeep && beepStep < 6) {
                    if (now - lastBuzzerToggle >= 100) {
                        lastBuzzerToggle = now;
                        buzzerOn = !buzzerOn;
                        digitalWrite(buzzerPin, buzzerOn ? HIGH : LOW);
                        beepStep++;
                        if (beepStep >= 6) {
                            portENTER_CRITICAL(&mux);
                            weatherBeeped = true;
                            portEXIT_CRITICAL(&mux);
                            digitalWrite(buzzerPin, LOW);
                        }
                    }
                } else {
                    digitalWrite(buzzerPin, LOW);
                }
                break;

            case STATE_WARNING:
                // Yellow, 2 Hz heartbeat
                handleHeartbeat(pixel.Color(255, 200, 0), now);
                digitalWrite(buzzerPin, LOW);
                break;

            case STATE_WIFI_SETUP_OFFLINE:
                // Blue, 2 Hz heartbeat
                handleHeartbeat(pixel.Color(0, 0, 255), now);
                digitalWrite(buzzerPin, LOW);
                break;

            case STATE_SAFE:
            default:
                // Green, 2 Hz heartbeat
                handleHeartbeat(pixel.Color(0, 255, 0), now);
                digitalWrite(buzzerPin, LOW);
                break;
        }
    }
};

extern DeviceSignalsManager signals;

#endif // DEVICE_SIGNALS_H
