// offline_storage.h — LittleFS Offline Event Buffer & Flash Persistence

#ifndef OFFLINE_STORAGE_H
#define OFFLINE_STORAGE_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#define QUEUE_FILE_PATH "/offline_events.jsonl"
#define MAX_QUEUED_EVENTS 50

struct QueuedEvent {
    String type;
    String details;
    String cardUID;
    String personName;
    unsigned long timestamp;
};

class OfflineStorageManager {
private:
    bool fsReady;
    int pendingCount;

public:
    OfflineStorageManager() : fsReady(false), pendingCount(0) {}

    bool begin() {
        if (!LittleFS.begin(true)) {
            Serial.println("[STORAGE] LittleFS Mount Failed!");
            fsReady = false;
            return false;
        }
        fsReady = true;
        recountPending();
        Serial.printf("[STORAGE] LittleFS initialized. Pending offline events: %d\n", pendingCount);
        return true;
    }

    void appendEvent(const String& type, const String& details, const String& cardUID = "", const String& person = "") {
        if (!fsReady) return;

        File f = LittleFS.open(QUEUE_FILE_PATH, FILE_APPEND);
        if (!f) {
            Serial.println("[STORAGE] Failed to open queue file for writing!");
            return;
        }

        StaticJsonDocument<256> doc;
        doc["type"] = type;
        doc["details"] = details;
        doc["cardUID"] = cardUID;
        doc["person"] = person;
        doc["ts"] = millis();

        String line;
        serializeJson(doc, line);
        f.println(line);
        f.close();

        pendingCount++;
        Serial.printf("[STORAGE] Event buffered locally (Total pending: %d): %s\n", pendingCount, line.c_str());
    }

    int getPendingCount() const {
        return pendingCount;
    }

    bool hasPendingEvents() const {
        return pendingCount > 0;
    }

    void clearQueue() {
        if (!fsReady) return;
        LittleFS.remove(QUEUE_FILE_PATH);
        pendingCount = 0;
        Serial.println("[STORAGE] Offline queue successfully cleared.");
    }

    // Callback function type for flushing events
    typedef void (*EventFlushCallback)(const String& type, const String& details, const String& cardUID, const String& person, unsigned long ts);

    void flushToCallback(EventFlushCallback callback) {
        if (!fsReady || !LittleFS.exists(QUEUE_FILE_PATH)) return;

        File f = LittleFS.open(QUEUE_FILE_PATH, FILE_READ);
        if (!f) return;

        Serial.println("[STORAGE] Flushing buffered offline events...");
        while (f.available()) {
            String line = f.readStringUntil('\n');
            line.trim();
            if (line.length() == 0) continue;

            StaticJsonDocument<256> doc;
            DeserializationError err = deserializeJson(doc, line);
            if (!err) {
                String type = doc["type"].as<String>();
                String details = doc["details"].as<String>();
                String cardUID = doc["cardUID"].as<String>();
                String person = doc["person"].as<String>();
                unsigned long ts = doc["ts"].as<unsigned long>();

                if (callback) {
                    callback(type, details, cardUID, person, ts);
                }
            }
        }
        f.close();
        clearQueue();
    }

private:
    void recountPending() {
        if (!fsReady || !LittleFS.exists(QUEUE_FILE_PATH)) {
            pendingCount = 0;
            return;
        }
        File f = LittleFS.open(QUEUE_FILE_PATH, FILE_READ);
        if (!f) {
            pendingCount = 0;
            return;
        }
        int count = 0;
        while (f.available()) {
            String line = f.readStringUntil('\n');
            line.trim();
            if (line.length() > 0) count++;
        }
        f.close();
        pendingCount = count;
    }
};

extern OfflineStorageManager offlineStorage;

#endif // OFFLINE_STORAGE_H

