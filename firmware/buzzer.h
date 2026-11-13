// buzzer.h — Non-Blocking Buzzer Controller with Distinct Audio Patterns

#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>
#include "config.h"

enum BuzzerPattern {
    BUZZ_OFF = 0,
    BUZZ_SINGLE_BEEP,      // 1 short beep (RFID card accepted / door action)
    BUZZ_DOUBLE_BEEP,      // 2 short beeps (Door unlocked)
    BUZZ_DENIED_ALARM,     // 3 harsh rapid beeps (Unauthorized card / denied)
    BUZZ_FIRE_SIREN,       // Continuous rapid siren (200ms on / 200ms off)
    BUZZ_PANIC_SOS         // SOS Morse pattern (... --- ...)
};

class BuzzerController {
private:
    uint8_t pin;
    BuzzerPattern currentPattern;
    bool enabled;
    unsigned long patternStartTime;
    unsigned long lastToggleTime;
    bool pinState;
    uint8_t stepIndex;

public:
    BuzzerController(uint8_t buzzerPin) : 
        pin(buzzerPin), currentPattern(BUZZ_OFF), enabled(true),
        patternStartTime(0), lastToggleTime(0), pinState(false), stepIndex(0) {}

    void begin() {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }

    void setEnabled(bool isEnabled) {
        enabled = isEnabled;
        if (!enabled) {
            stop();
        }
    }

    bool isEnabled() const {
        return enabled;
    }

    void play(BuzzerPattern pattern) {
        if (!enabled && pattern != BUZZ_FIRE_SIREN && pattern != BUZZ_PANIC_SOS) {
            // Never mute safety life-critical sirens
            return;
        }
        currentPattern = pattern;
        patternStartTime = millis();
        lastToggleTime = millis();
        stepIndex = 0;
        pinState = true;
        digitalWrite(pin, HIGH);
    }

    void stop() {
        currentPattern = BUZZ_OFF;
        pinState = false;
        digitalWrite(pin, LOW);
    }

    void update() {
        if (currentPattern == BUZZ_OFF) return;
        unsigned long now = millis();

        switch (currentPattern) {
            case BUZZ_SINGLE_BEEP:
                if (now - patternStartTime >= 100) {
                    stop();
                }
                break;

            case BUZZ_DOUBLE_BEEP:
                // 80ms on -> 60ms off -> 80ms on -> off
                if (stepIndex == 0 && now - lastToggleTime >= 80) {
                    digitalWrite(pin, LOW);
                    pinState = false;
                    lastToggleTime = now;
                    stepIndex = 1;
                } else if (stepIndex == 1 && now - lastToggleTime >= 60) {
                    digitalWrite(pin, HIGH);
                    pinState = true;
                    lastToggleTime = now;
                    stepIndex = 2;
                } else if (stepIndex == 2 && now - lastToggleTime >= 80) {
                    stop();
                }
                break;

            case BUZZ_DENIED_ALARM:
                // 3 quick 60ms beeps with 50ms pause
                if (now - lastToggleTime >= 60) {
                    pinState = !pinState;
                    digitalWrite(pin, pinState ? HIGH : LOW);
                    lastToggleTime = now;
                    stepIndex++;
                    if (stepIndex >= 6) {
                        stop();
                    }
                }
                break;

            case BUZZ_FIRE_SIREN:
                // Continuous emergency alarm: 200ms ON / 200ms OFF
                if (now - lastToggleTime >= 200) {
                    pinState = !pinState;
                    digitalWrite(pin, pinState ? HIGH : LOW);
                    lastToggleTime = now;
                }
                break;

            case BUZZ_PANIC_SOS:
                // Repeatable SOS pulse
                if (now - lastToggleTime >= 150) {
                    pinState = !pinState;
                    digitalWrite(pin, pinState ? HIGH : LOW);
                    lastToggleTime = now;
                    stepIndex++;
                    if (stepIndex >= 18) {
                        stop();
                    }
                }
                break;

            default:
                stop();
                break;
        }
    }
};

extern BuzzerController buzzer;

#endif // BUZZER_H

