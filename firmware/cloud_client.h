// cloud_client.h — Secure HTTPS Telemetry & Event Queue Client (Core 0 Only)
// Level 2 & Level 3 Compliant: TLS cert verification, monotonic NVS seq, NTP ts, non-blocking queue

#ifndef CLOUD_CLIENT_H
#define CLOUD_CLIENT_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <time.h>
#include "config.h"
#include "wifi_portal.h"
#include "sensors.h"
#include "door_control.h"

// Concatenated Root CAs: ISRG Root X1 (Let's Encrypt) + GTS Root R1 (Google Trust Services)
static const char* ROOT_CA_PEM =
"-----BEGIN CERTIFICATE-----\n"
"MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw\n"
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4\n"
"WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu\n"
"ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY\n"
"MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc\n"
"h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+\n"
"0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U\n"
"A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW\n"
"T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH\n"
"B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC\n"
"B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv\n"
"KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn\n"
"OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn\n"
"jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw\n"
"qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI\n"
"rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV\n"
"HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq\n"
"hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL\n"
"ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ\n"
"3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK\n"
"NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5\n"
"ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur\n"
"TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC\n"
"jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc\n"
"oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq\n"
"4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA\n"
"mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d\n"
"emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=\n"
"-----END CERTIFICATE-----\n"
"-----BEGIN CERTIFICATE-----\n"
"MIIFWjCCA0KgAwIBAgIQbkepxUtHDA3sM9CJuRz04TANBgkqhkiG9w0BAQwFADBH\n"
"MQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExM\n"
"QzEUMBIGA1UEAxMLR1RTIFJvb3QgUjEwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIy\n"
"MDAwMDAwWjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNl\n"
"cnZpY2VzIExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjEwggIiMA0GCSqGSIb3DQEB\n"
"AQUAA4ICDwAwggIKAoICAQC2EQKLHuOhd5s73L+UPreVp0A8of2C+X0yBoJx9vaM\n"
"f/vo27xqLpeXo4xL+Sv2sfnOhB2x+cWX3u+58qPpvBKJXqeqUqv4IyfLpLGcY9vX\n"
"mX7wCl7raKb0xlpHDU0QM+NOsROjyBhsS+z8CZDfnWQpJSMHobTSPS5g4M/SCYe7\n"
"zUjwTcLCeoiKu7rPWRnWr4+wB7CeMfGCwcDfLqZtbBkOtdh+JhpFAz2weaSUKK0P\n"
"fyblqAj+lug8aJRT7oM6iCsVlgmy4HqMLnXWnOunVmSPlk9orj2XwoSPwLxAwAtc\n"
"vfaHszVsrBhQf4TgTM2S0yDpM7xSma8ytSmzJSq0SPly4cpk9+aCEI3oncKKiPo4\n"
"Zor8Y/kB+Xj9e1x3+naH+uzfsQ55lVe0vSbv1gHR6xYKu44LtcXFilWr06zqkUsp\n"
"zBmkMiVOKvFlRNACzqrOSbTqn3yDsEB750Orp2yjj32JgfpMpf/VjsPOS+C12LOO\n"
"Rc92wO1AK/1TD7Cn1TsNsYqiA94xrcx36m97PtbfkSIS5r762DL8EGMUUXLeXdYW\n"
"k70paDPvOmbsB4om3xPXV2V4J95eSRQAogB/mqghtqmxlbCluQ0WEdrHbEg8QOB+\n"
"DVrNVjzRlwW5y0vtOUucxD/SVRNuJLDWcfr0wbrM7Rv1/oFB2ACYPTrIrnqYNxgF\n"
"lQIDAQABo0IwQDAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0TAQH/BAUwAwEB/zAdBgNV\n"
"HQ4EFgQU5K8rJnEaK0gnhS9SZizv8IkTcT4wDQYJKoZIhvcNAQEMBQADggIBADiW\n"
"Cu49tJYeX++dnAsznyvgyv3SjgofQXSlfKqE1OXyHuY3UjKcC9FhHb8owbZEKTV1\n"
"d5iyfNm9dKyKaOOpMQkpAWBz40d8U6iQSifvS9efk+eCNs6aaAyC58/UEBZvXw6Z\n"
"XPYfcX3v73svfuo21pdwCxXu11xWajOl40k4DLh9+42FpLFZXvRq4d2h9mREruZR\n"
"gyFmxhE+885H7pwoHyXa/6xmld01D1zvICxi/ZG6qcz8WpyTgYMpl0p8WnK0OdC3\n"
"d8t5/Wk6kjftbjhlRn7pYL15iJdfOBL07q9bgsiG1eGZbYwE8na6SfZu6W0eX6Dv\n"
"J4J2QPim01hcDyxC2kLGe4g0x8HYRZvBPsVhHdljUEn2NIVq4BjFbkerQUIpm/Zg\n"
"DdIx02OYI5NaAIFItO/Nis3Jz5nu2Z6qNuFoS3FJFDYoOj0dzpqPJeaAcWErtXvM\n"
"+SUWgeExX6GjfhaknBZqlxi9dnKlC54dNuYvoS++cJEPqOba+MSSQGwlfnuzCdyy\n"
"F62ARPBopY+Udf90WuioAnwMCeKpSwughQtiue+hMZL77/ZRBIls6Kl0obsXs7X9\n"
"SQ98POyDGCBDTtWTurQ0sR8WNh8M5mQ5Fkzc4P4dyKliPUDqysU0ArSuiYgzNdws\n"
"E3PYJ/HQcu51OyLemGhmW/HGY0dVHLqlCFF1pkgl\n"
"-----END CERTIFICATE-----\n";

struct QueuedDeviceEvent {
    char type[24];
    char detail[96];
    uint64_t ts;
};

class CloudClientManager {
private:
    Preferences seqPrefs;
    QueueHandle_t eventQueue;
    uint32_t currentSeq;
    unsigned long lastTelemetrySync;
    bool serverReachable;
    bool everConnected;

    uint32_t nextSeq() {
        currentSeq++;
        seqPrefs.begin("seq_cfg", false);
        seqPrefs.putUInt("seq", currentSeq);
        seqPrefs.end();
        return currentSeq;
    }

    uint64_t getUnixUtc() const {
        time_t now = time(nullptr);
        return (now > 1700000000) ? (uint64_t)now : 0;
    }

    void updateServerState(bool ok) {
        if (ok && !serverReachable) {
            serverReachable = true;
            if (!everConnected) {
                everConnected = true;
                Serial.println("[NET] Server connected (LIVE)");
            } else {
                Serial.println("[NET] Server reconnected (LIVE)");
            }
        } else if (!ok && serverReachable) {
            serverReachable = false;
            Serial.println("[NET] Server disconnected");
        }
    }

    bool postJson(const String& path, const String& payload) {
        String baseUrl = wifiPortal.getServerUrl();
        if (baseUrl.length() == 0) return false;

        WiFiClientSecure client;
        if (baseUrl.startsWith("https://")) {
            client.setCACert(ROOT_CA_PEM);
        }

        HTTPClient http;
        http.setTimeout(3500);
        String url = baseUrl + path;
        bool began = baseUrl.startsWith("https://")
            ? http.begin(client, url)
            : http.begin(url);
        if (!began) return false;

        http.addHeader("Content-Type", "application/json");
        String tok = wifiPortal.getDeviceToken();
        if (tok.length() > 0) {
            http.addHeader("Authorization", "Bearer " + tok);
        }

        int code = http.POST(payload);
        http.end();

        bool ok = (code >= 200 && code < 300) || (code == 409); // 409 = replay rejected by server
        updateServerState(ok);
        return (code >= 200 && code < 300);
    }

public:
    CloudClientManager() :
        eventQueue(NULL),
        currentSeq(0),
        lastTelemetrySync(0),
        serverReachable(false),
        everConnected(false) {}

    void begin() {
        seqPrefs.begin("seq_cfg", false);
        currentSeq = seqPrefs.getUInt("seq", 0);
        seqPrefs.end();
        eventQueue = xQueueCreate(32, sizeof(QueuedDeviceEvent));
        Serial.printf("[NET] Cloud client initialized (seq=%lu)\n", (unsigned long)currentSeq);
    }

    bool isServerReachable() const {
        return serverReachable && wifiPortal.isConnected();
    }

    // Thread-safe, non-blocking: can be called from Core 1 or setup()
    void enqueueEvent(const char* type, const String& detail) {
        Serial.printf("[EVENT] %s %s\n", type, detail.c_str());
        if (!eventQueue) return;

        QueuedDeviceEvent evt;
        memset(&evt, 0, sizeof(evt));
        strncpy(evt.type, type, sizeof(evt.type) - 1);
        strncpy(evt.detail, detail.c_str(), sizeof(evt.detail) - 1);
        evt.ts = getUnixUtc();
        xQueueSend(eventQueue, &evt, 0);
    }

    // Called ONLY from Core 0 (NetWorker)
    void update(const char* safetyStatusStr, unsigned long uptimeSec, float chipTempC) {
        if (!wifiPortal.isConnected()) {
            updateServerState(false);
            return;
        }

        // 1. Drain event queue immediately
        QueuedDeviceEvent evt;
        while (eventQueue && xQueueReceive(eventQueue, &evt, 0) == pdTRUE) {
            StaticJsonDocument<384> doc;
            doc["device_id"] = DEVICE_ID;
            doc["room_id"] = wifiPortal.getRoomID();
            doc["seq"] = nextSeq();
            doc["ts"] = (evt.ts > 0) ? evt.ts : getUnixUtc();
            doc["type"] = evt.type;
            doc["detail"] = evt.detail;

            String payload;
            serializeJson(doc, payload);
            postJson("/api/events", payload);
        }

        // 2. Periodic telemetry every 5s
        unsigned long now = millis();
        if (now - lastTelemetrySync >= TELEMETRY_INTERVAL_MS) {
            lastTelemetrySync = now;

            SensorReadings s = sensors.getLatest();
            StaticJsonDocument<512> doc;
            doc["device_id"] = DEVICE_ID;
            doc["room_id"] = wifiPortal.getRoomID();
            doc["seq"] = nextSeq();
            doc["ts"] = getUnixUtc();
            doc["uptime_s"] = uptimeSec;
            doc["temp_c"] = roundf(s.temperature * 10.0f) / 10.0f;
            doc["rh"] = (int)roundf(s.humidity);
            doc["heat_index_c"] = roundf(s.heatIndex * 10.0f) / 10.0f;
            doc["flame"] = s.flameDetected;
            doc["door"] = door.getStateString();
            doc["status"] = safetyStatusStr;
            doc["heat_trend"] = sensors.getTrendString();
            doc["rssi"] = wifiPortal.getRSSI();
            doc["heap_kb"] = ESP.getFreeHeap() / 1024;
            doc["chip_temp_c"] = roundf(chipTempC * 10.0f) / 10.0f;
            doc["fw"] = FIRMWARE_VERSION;
            doc["quality"] = s.qualityGood ? "good" : "bad";

            String payload;
            serializeJson(doc, payload);
            postJson("/api/telemetry", payload);
        }
    }
};

extern CloudClientManager cloud;

#endif // CLOUD_CLIENT_H
