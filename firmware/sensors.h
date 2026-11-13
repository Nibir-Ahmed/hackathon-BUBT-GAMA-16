// sensors.h — Environmental Sensors (DHT11, Flame, Touch Panic Pad)
// Level 1 & Level 3 Compliant: Validity Range, Heat Index, 10-Reading Trend, ESP32-S3 Touch Calibration

#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <DHT.h>
#include "config.h"

enum HeatTrend {
    TREND_STABLE = 0,
    TREND_RISING,
    TREND_FALLING
};

struct SensorReadings {
    float temperature;
    float humidity;
    float heatIndex;
    bool dhtValid;
    bool qualityGood;
    HeatTrend trend;
    bool flameDetected;
    int touchRaw;
    int touchBaseline;
    unsigned long lastReadTime;
};

class SensorManager {
private:
    DHT dht;
    SensorReadings currentReadings;
    unsigned long lastDhtMillis;

    // 10-reading ring buffer for heat trend
    float tempHistory[10];
    int historyCount;
    int historyIndex;

    // Touch panic pad state
    int touchBaseline;
    int touchThresholdDelta;
    unsigned long touchStartMillis;
    bool panicFiredForTouch;

    float computeHeatIndex(float tempC, float rh) const {
        // Steadman simple formula for < 27°C, Rothfusz regression for >= 27°C
        float hiSimple = 0.5f * (tempC + 61.0f + ((tempC - 68.0f) * 1.2f) + (rh * 0.094f));
        if (tempC < 27.0f) {
            // Convert to F for Steadman or return tempC when cool
            float tf = tempC * 1.8f + 32.0f;
            float hif = 0.5f * (tf + 61.0f + ((tf - 68.0f) * 1.2f) + (rh * 0.094f));
            if (hif < 80.0f) {
                return (hif - 32.0f) / 1.8f;
            }
        }
        float c1 = -8.78469475556f;
        float c2 = 1.61139411f;
        float c3 = 2.33854883889f;
        float c4 = -0.14611605f;
        float c5 = -0.012308094f;
        float c6 = -0.0164248277778f;
        float c7 = 0.002211732f;
        float c8 = 0.00072546f;
        float c9 = -0.000003582f;
        return c1 + (c2 * tempC) + (c3 * rh) + (c4 * tempC * rh) +
               (c5 * tempC * tempC) + (c6 * rh * rh) +
               (c7 * tempC * tempC * rh) + (c8 * tempC * rh * rh) +
               (c9 * tempC * tempC * rh * rh);
    }

    void pushTempHistory(float tempC) {
        tempHistory[historyIndex] = tempC;
        historyIndex = (historyIndex + 1) % 10;
        if (historyCount < 10) historyCount++;

        if (historyCount < 4) {
            currentReadings.trend = TREND_STABLE;
            return;
        }

        // Compare average of newer half vs older half across available history (up to 10)
        int half = historyCount / 2;
        float oldSum = 0.0f;
        float newSum = 0.0f;
        int startIdx = (historyIndex - historyCount + 10) % 10;
        for (int i = 0; i < historyCount; i++) {
            int idx = (startIdx + i) % 10;
            if (i < half) oldSum += tempHistory[idx];
            else newSum += tempHistory[idx];
        }
        float oldAvg = oldSum / half;
        float newAvg = newSum / (historyCount - half);
        float diff = newAvg - oldAvg;

        if (diff >= 0.35f) currentReadings.trend = TREND_RISING;
        else if (diff <= -0.35f) currentReadings.trend = TREND_FALLING;
        else currentReadings.trend = TREND_STABLE;
    }

public:
    SensorManager() :
        dht(PIN_DHT11, DHT11),
        lastDhtMillis(0),
        historyCount(0),
        historyIndex(0),
        touchBaseline(0),
        touchThresholdDelta(3000),
        touchStartMillis(0),
        panicFiredForTouch(false) {
        currentReadings.temperature = 0.0f;
        currentReadings.humidity = 0.0f;
        currentReadings.heatIndex = 0.0f;
        currentReadings.dhtValid = false;
        currentReadings.qualityGood = false;
        currentReadings.trend = TREND_STABLE;
        currentReadings.flameDetected = false;
        currentReadings.touchRaw = 0;
        currentReadings.touchBaseline = 0;
        currentReadings.lastReadTime = 0;
    }

    void begin() {
        dht.begin();
        pinMode(PIN_FLAME_DO, INPUT_PULLUP);
        calibrateTouchBaseline();
    }

    void calibrateTouchBaseline() {
        long sum = 0;
        int validSamples = 0;
        const int samples = 10;
        for (int i = 0; i < samples; i++) {
            int v = touchRead(PIN_TOUCH_PANIC);
            if (v > 0 && v < 4000000) {
                sum += v;
                validSamples++;
            }
            delay(20);
        }
        if (validSamples > 0) {
            touchBaseline = (int)(sum / validSamples);
            int pctDelta = (int)(touchBaseline * 0.25f);
            touchThresholdDelta = (pctDelta > 3000) ? pctDelta : 3000;
            currentReadings.touchBaseline = touchBaseline;
            Serial.printf("[SENSORS] Touch baseline calibrated: %d (trigger > %d)\n",
                          touchBaseline, touchBaseline + touchThresholdDelta);
        } else {
            touchBaseline = 0;
            currentReadings.touchBaseline = 0;
            Serial.println("[SENSORS] Touch baseline skipped (pin floating/timeout on GPIO 1)");
        }
    }

    bool readFlame() {
        bool flame = (digitalRead(PIN_FLAME_DO) == LOW);
        currentReadings.flameDetected = flame;
        return flame;
    }

    // Returns true ONCE per >=2s touch hold; ignores <1s touches
    bool pollTouchPanic() {
        unsigned long now = millis();
        int val = touchRead(PIN_TOUCH_PANIC);
        currentReadings.touchRaw = val;

        bool touched = (touchBaseline > 0) && (val > (touchBaseline + touchThresholdDelta));
        if (touched) {
            if (touchStartMillis == 0) {
                touchStartMillis = now;
            } else if (!panicFiredForTouch && (now - touchStartMillis >= TOUCH_HOLD_TIME_MS)) {
                panicFiredForTouch = true;
                return true;
            }
        } else {
            touchStartMillis = 0;
            panicFiredForTouch = false;
        }
        return false;
    }

    bool readDHT() {
        unsigned long now = millis();
        lastDhtMillis = now;
        float t = dht.readTemperature();
        float h = dht.readHumidity();

        bool valid = !isnan(t) && !isnan(h) &&
                     t >= DHT_VALID_TEMP_MIN && t <= DHT_VALID_TEMP_MAX &&
                     h >= DHT_VALID_HUMID_MIN && h <= DHT_VALID_HUMID_MAX;

        if (valid) {
            currentReadings.temperature = t;
            currentReadings.humidity = h;
            currentReadings.heatIndex = computeHeatIndex(t, h);
            currentReadings.dhtValid = true;
            currentReadings.qualityGood = true;
            currentReadings.lastReadTime = now;
            pushTempHistory(t);
            return true;
        } else {
            // Re-init pin pullup in case sensor was unplugged/replugged
            dht.begin();
            Serial.printf("[SENSORS] DHT11 invalid/NaN read (T=%.1f, RH=%.1f) on GPIO %d\n", t, h, PIN_DHT11);
            // Keep last valid temperature/humidity/status; mark quality bad
            currentReadings.dhtValid = false;
            currentReadings.qualityGood = false;
            return false;
        }
    }

    const SensorReadings& getLatest() const {
        return currentReadings;
    }

    const char* getTrendString() const {
        switch (currentReadings.trend) {
            case TREND_RISING:  return "RISING";
            case TREND_FALLING: return "FALLING";
            case TREND_STABLE:
            default:            return "STABLE";
        }
    }
};

extern SensorManager sensors;

#endif // SENSORS_H
