// door_control.h — Simulated Classroom Door State Machine with NVS Fire Latch

#ifndef DOOR_CONTROL_H
#define DOOR_CONTROL_H

#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

enum DoorState {
    DOOR_LOCKED = 0,
    DOOR_UNLOCKED = 1
};

class DoorController {
private:
    DoorState currentState;
    unsigned long lastStateChangeTime;
    uint16_t autoLockSeconds;
    Preferences prefs;
    bool fireLatched;

public:
    DoorController() :
        currentState(DOOR_LOCKED),
        lastStateChangeTime(0),
        autoLockSeconds(DOOR_UNLOCK_TIME_SEC),
        fireLatched(false) {}

    void begin() {
        prefs.begin("door_cfg", false);
        fireLatched = prefs.getBool("fire_latched", false);
        autoLockSeconds = prefs.getUInt("autolock_sec", DOOR_UNLOCK_TIME_SEC);
        prefs.end();

        if (fireLatched) {
            currentState = DOOR_UNLOCKED;
        } else {
            currentState = DOOR_LOCKED;
        }
        lastStateChangeTime = millis();
    }

    bool isFireLatched() const {
        return fireLatched;
    }

    DoorState getState() const {
        return currentState;
    }

    const char* getStateString() const {
        return (currentState == DOOR_UNLOCKED) ? "UNLOCKED" : "LOCKED";
    }

    void unlock(const String& reason = "Access") {
        if (currentState != DOOR_UNLOCKED) {
            currentState = DOOR_UNLOCKED;
            Serial.printf("[DOOR] UNLOCKED (%s)\n", reason.c_str());
        }
        lastStateChangeTime = millis();
    }

    void lock(const String& reason = "AutoLock") {
        if (fireLatched) return; // Fire overrides every other door rule
        if (currentState != DOOR_LOCKED) {
            currentState = DOOR_LOCKED;
            lastStateChangeTime = millis();
            Serial.printf("[DOOR] LOCKED (%s)\n", reason.c_str());
        }
    }

    void setFireLatched(bool latched) {
        if (fireLatched == latched) return;
        fireLatched = latched;
        prefs.begin("door_cfg", false);
        prefs.putBool("fire_latched", fireLatched);
        prefs.end();

        if (fireLatched) {
            currentState = DOOR_UNLOCKED;
            lastStateChangeTime = millis();
            Serial.println("[DOOR] FIRE LATCHED -> Door UNLOCKED (persisted in flash)");
        } else {
            currentState = DOOR_LOCKED;
            lastStateChangeTime = millis();
            Serial.println("[DOOR] Fire reset -> Door LOCKED");
        }
    }

    void update() {
        if (!fireLatched && currentState == DOOR_UNLOCKED && autoLockSeconds > 0) {
            if (millis() - lastStateChangeTime >= (unsigned long)autoLockSeconds * 1000UL) {
                lock("AutoLock Timeout");
            }
        }
    }
};

extern DoorController door;

#endif // DOOR_CONTROL_H
