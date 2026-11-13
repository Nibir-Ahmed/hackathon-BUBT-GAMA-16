// wifi_portal.h — Non-Blocking Wi-Fi Setup AP, Exponential Backoff & mDNS
// Level 2 Compliant: No hard-coded creds, 8-char random AP pass, 15s setup error fallback

#ifndef WIFI_PORTAL_H
#define WIFI_PORTAL_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <time.h>
#include "config.h"
#include "device_signals.h"

enum WifiState {
    WIFI_STATE_SETUP_AP = 0,
    WIFI_STATE_CONNECTING_INITIAL,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_RECONNECTING
};

class WiFiPortalManager {
private:
    Preferences prefs;
    WifiState state;
    String savedSsid;
    String savedPass;
    String savedRoomId;
    String savedServerUrl;
    String savedDeviceToken;

    String apSsid;
    String apPassword;
    String lastSetupError;

    unsigned long connectStartMs;
    unsigned long lastReconnectAttemptMs;
    unsigned long reconnectBackoffMs;
    unsigned long lastScanMs;
    bool mdnsStarted;
    bool ntpStarted;

    String generateRandomPassword8() {
        const char charset[] = "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghjkmnpqrstuvwxyz23456789";
        String pwd = "";
        for (int i = 0; i < 8; i++) {
            uint32_t r = esp_random() % (sizeof(charset) - 1);
            pwd += charset[r];
        }
        return pwd;
    }

    void startSetupAP() {
        WiFi.disconnect(true);
        delay(50);
        WiFi.mode(WIFI_AP_STA);
        IPAddress apIP(192, 168, 4, 1);
        IPAddress subnet(255, 255, 255, 0);
        WiFi.softAPConfig(apIP, apIP, subnet);

        apSsid = String(SETUP_AP_PREFIX) + String(TEAM_ID);
        if (apPassword.length() != 8) {
            apPassword = generateRandomPassword8();
        }
        WiFi.softAP(apSsid.c_str(), apPassword.c_str());

        // Start async scan for nearby networks
        WiFi.scanNetworks(true);
        lastScanMs = millis();

        state = WIFI_STATE_SETUP_AP;
        signals.setWifiSetupOrOffline(true);

        Serial.println("========================================================");
        Serial.printf("[NET] Setup AP Started: %s\n", apSsid.c_str());
        Serial.printf("[NET] Setup AP Password: %s\n", apPassword.c_str());
        Serial.println("[NET] Setup Page: http://192.168.4.1");
        Serial.println("========================================================");
    }

    void startServicesOnConnect() {
        if (!mdnsStarted) {
            String host = "smartclass-" + String(TEAM_ID);
            host.toLowerCase();
            if (MDNS.begin(host.c_str())) {
                MDNS.addService("http", "tcp", 80);
                mdnsStarted = true;
                Serial.printf("[NET] mDNS active: http://%s.local\n", host.c_str());
            } else {
                Serial.println("[NET] mDNS failed to start");
            }
        }
        if (!ntpStarted) {
            // UTC for telemetry ts; Asia/Dhaka formatting handled in helper
            configTime(0, 0, "pool.ntp.org", "time.nist.gov");
            ntpStarted = true;
            Serial.println("[NET] NTP time sync started");
        }
    }

public:
    WiFiPortalManager() :
        state(WIFI_STATE_SETUP_AP),
        savedSsid(""),
        savedPass(""),
        savedRoomId(""),
        savedServerUrl(""),
        savedDeviceToken(""),
        apSsid(""),
        apPassword(""),
        lastSetupError(""),
        connectStartMs(0),
        lastReconnectAttemptMs(0),
        reconnectBackoffMs(2000),
        lastScanMs(0),
        mdnsStarted(false),
        ntpStarted(false) {}

    void begin() {
        prefs.begin("sys_cfg", true);
        savedSsid = prefs.getString("wifi_ssid", "");
        savedPass = prefs.getString("wifi_pass", "");
        savedRoomId = prefs.getString("room_id", "");
        savedServerUrl = prefs.getString("srv_url", "");
        savedDeviceToken = prefs.getString("dev_tok", "");
        prefs.end();

        if (savedSsid.length() > 0) {
            WiFi.mode(WIFI_STA);
            WiFi.setAutoReconnect(false);
            WiFi.begin(savedSsid.c_str(), savedPass.c_str());
            connectStartMs = millis();
            lastReconnectAttemptMs = millis();
            reconnectBackoffMs = 2000;
            state = WIFI_STATE_RECONNECTING;
            signals.setWifiSetupOrOffline(true);
            Serial.printf("[NET] Connecting to saved Wi-Fi '%s'...\n", savedSsid.c_str());
        } else {
            startSetupAP();
        }
    }

    void submitSetup(const String& ssid, const String& pass, const String& roomId,
                     const String& serverUrl, const String& token) {
        savedSsid = ssid;
        savedPass = pass;
        savedRoomId = roomId;
        savedServerUrl = serverUrl;
        savedServerUrl.trim();
        while (savedServerUrl.endsWith("/")) {
            savedServerUrl.remove(savedServerUrl.length() - 1);
        }
        savedDeviceToken = token;
        lastSetupError = "";

        Serial.printf("[NET] Attempting connection to '%s' (timeout 13s)...\n", savedSsid.c_str());
        WiFi.begin(savedSsid.c_str(), savedPass.c_str());
        connectStartMs = millis();
        state = WIFI_STATE_CONNECTING_INITIAL;
    }

    void update() {
        unsigned long now = millis();

        switch (state) {
            case WIFI_STATE_SETUP_AP:
                // Refresh async scan every 15 seconds while in setup mode
                if (now - lastScanMs >= 15000 && WiFi.scanComplete() != WIFI_SCAN_RUNNING) {
                    WiFi.scanDelete();
                    WiFi.scanNetworks(true);
                    lastScanMs = now;
                }
                break;

            case WIFI_STATE_CONNECTING_INITIAL:
                if (WiFi.status() == WL_CONNECTED) {
                    // Save credentials to NVS
                    prefs.begin("sys_cfg", false);
                    prefs.putString("wifi_ssid", savedSsid);
                    prefs.putString("wifi_pass", savedPass);
                    prefs.putString("room_id", savedRoomId);
                    prefs.putString("srv_url", savedServerUrl);
                    prefs.putString("dev_tok", savedDeviceToken);
                    prefs.end();

                    WiFi.softAPdisconnect(true);
                    WiFi.mode(WIFI_STA);
                    state = WIFI_STATE_CONNECTED;
                    signals.setWifiSetupOrOffline(false);
                    reconnectBackoffMs = 2000;
                    Serial.printf("[NET] Connected! IP=%s RSSI=%d dBm\n",
                                  WiFi.localIP().toString().c_str(), WiFi.RSSI());
                    startServicesOnConnect();
                } else if (now - connectStartMs >= WIFI_SETUP_TIMEOUT_MS) {
                    lastSetupError = "Could not connect to '" + savedSsid + "' (wrong password or unreachable).";
                    Serial.printf("[NET] Setup error: %s\n", lastSetupError.c_str());
                    startSetupAP();
                }
                break;

            case WIFI_STATE_CONNECTED:
                if (WiFi.status() != WL_CONNECTED) {
                    Serial.println("[NET] Wi-Fi disconnected! Starting backoff reconnect...");
                    state = WIFI_STATE_RECONNECTING;
                    signals.setWifiSetupOrOffline(true);
                    lastReconnectAttemptMs = now;
                    reconnectBackoffMs = 2000;
                }
                break;

            case WIFI_STATE_RECONNECTING:
                if (WiFi.status() == WL_CONNECTED) {
                    state = WIFI_STATE_CONNECTED;
                    signals.setWifiSetupOrOffline(false);
                    reconnectBackoffMs = 2000;
                    Serial.printf("[NET] Wi-Fi reconnected! IP=%s RSSI=%d dBm\n",
                                  WiFi.localIP().toString().c_str(), WiFi.RSSI());
                    startServicesOnConnect();
                } else if (now - lastReconnectAttemptMs >= reconnectBackoffMs) {
                    lastReconnectAttemptMs = now;
                    Serial.printf("[NET] Reconnecting to '%s' (next retry in %lus)...\n",
                                  savedSsid.c_str(), reconnectBackoffMs / 1000);
                    WiFi.disconnect();
                    WiFi.begin(savedSsid.c_str(), savedPass.c_str());
                    reconnectBackoffMs = min(reconnectBackoffMs * 2, 30000UL);
                }
                break;
        }
    }

    void resetSettings() {
        prefs.begin("sys_cfg", false);
        prefs.clear();
        prefs.end();
        WiFi.disconnect(true, true);
        Serial.println("[NET] Saved Wi-Fi & server credentials cleared. Restarting in setup mode...");
        delay(300);
        ESP.restart();
    }

    bool isSetupMode() const { return state == WIFI_STATE_SETUP_AP || state == WIFI_STATE_CONNECTING_INITIAL; }
    bool isConnected() const { return WiFi.status() == WL_CONNECTED; }
    WifiState getState() const { return state; }
    String getRoomID() const { return savedRoomId.length() > 0 ? savedRoomId : "-"; }
    String getServerUrl() const { return savedServerUrl; }
    String getDeviceToken() const { return savedDeviceToken; }
    String getLastSetupError() const { return lastSetupError; }
    int getRSSI() const { return isConnected() ? WiFi.RSSI() : 0; }
    String getLocalIP() const {
        if (isConnected()) return WiFi.localIP().toString();
        if (state == WIFI_STATE_SETUP_AP) return "192.168.4.1 (AP)";
        return "0.0.0.0 (Offline)";
    }
};

extern WiFiPortalManager wifiPortal;

#endif // WIFI_PORTAL_H
