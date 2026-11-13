// local_webserver.h — Local Web Server (Setup Portal in AP mode + LittleFS Status Page)
// Level 2 & Level 3 Compliant

#ifndef LOCAL_WEBSERVER_H
#define LOCAL_WEBSERVER_H

#include <Arduino.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <time.h>
#include "config.h"
#include "door_control.h"
#include "sensors.h"
#include "rfid_access.h"
#include "wifi_portal.h"
#include "cloud_client.h"

// Shared state callbacks provided by firmware.ino
extern const char* getSafetyStatusString();
extern bool isPanicBannerActive();
extern bool isSilenceRefusedActive();
extern void performFireReset(const char* source);
extern float getChipTemperature();
extern unsigned long getUptimeSeconds();

class LocalWebServerManager {
private:
    WebServer server;
    bool serverStarted;

    String getDhakaTimeString() const {
        time_t now = time(nullptr);
        if (now < 1700000000) return "Syncing NTP...";
        time_t dhaka = now + (6 * 3600); // Asia/Dhaka = UTC+6
        struct tm tmInfo;
        gmtime_r(&dhaka, &tmInfo);
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S +06", &tmInfo);
        return String(buf);
    }

    void handleSetupPage() {
        String err = wifiPortal.getLastSetupError();
        int n = WiFi.scanComplete();
        String optionsHtml = "";
        if (n > 0) {
            for (int i = 0; i < n; i++) {
                String ssid = WiFi.SSID(i);
                int rssi = WiFi.RSSI(i);
                optionsHtml += "<option value=\"" + ssid + "\">" + ssid + " (" + String(rssi) + " dBm)</option>";
            }
        }

        String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
                      "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                      "<title>SmartClass-" TEAM_ID " Setup</title>"
                      "<style>body{font-family:sans-serif;background:#f1f5f9;padding:20px;color:#0f172a;}"
                      ".box{max-width:440px;margin:0 auto;background:#fff;padding:20px;border-radius:12px;box-shadow:0 2px 10px rgba(0,0,0,0.08);}"
                      "label{display:block;font-size:13px;font-weight:600;margin-top:12px;}"
                      "input,select{width:100%;padding:9px;margin-top:4px;border:1px solid #cbd5e1;border-radius:8px;box-sizing:border-box;}"
                      "button{width:100%;margin-top:16px;padding:11px;background:#2563eb;color:#fff;border:none;border-radius:8px;font-weight:700;cursor:pointer;}"
                      ".err{background:#fee2e2;color:#991b1b;padding:10px;border-radius:8px;margin-bottom:12px;font-size:13px;font-weight:600;}"
                      "</style></head><body><div class='box'><h2>SmartClass-" TEAM_ID " Setup</h2>";
        if (err.length() > 0) {
            html += "<div class='err'>" + err + "</div>";
        }
        html += "<form method='POST' action='/setup'>";
        if (optionsHtml.length() > 0) {
            html += "<label>Nearby Wi-Fi Networks</label><select onchange=\"document.getElementById('ssid').value=this.value\">"
                    "<option value=''>-- Select network --</option>" + optionsHtml + "</select>";
        }
        html += "<label>Wi-Fi SSID</label><input id='ssid' name='ssid' required>"
                "<label>Wi-Fi Password</label><input name='pass' type='password'>"
                "<label>Room ID (e.g. R-305)</label><input name='room_id' required>"
                "<label>Server Base URL (https://...)</label><input name='server_url' placeholder='https://your-server.onrender.com' required>"
                "<label>Device Token</label><input name='token' required>"
                "<button type='submit'>Save & Connect</button></form></div></body></html>";

        server.send(200, "text/html", html);
    }

    void handleSetupSubmit() {
        String ssid = server.arg("ssid");
        String pass = server.arg("pass");
        String roomId = server.arg("room_id");
        String serverUrl = server.arg("server_url");
        String token = server.arg("token");

        server.send(200, "text/html",
            "<html><body style='font-family:sans-serif;padding:24px;'>"
            "<h3>Connecting to " + ssid + "...</h3>"
            "<p>Check Serial Monitor for IP address, or reconnect to this AP if connection fails within 15s.</p>"
            "</body></html>");

        wifiPortal.submitSetup(ssid, pass, roomId, serverUrl, token);
    }

    void handleRoot() {
        if (wifiPortal.isSetupMode()) {
            handleSetupPage();
            return;
        }
        if (LittleFS.exists("/index.html")) {
            File f = LittleFS.open("/index.html", "r");
            if (f) {
                server.streamFile(f, "text/html");
                f.close();
                return;
            }
        }
        server.send(500, "text/plain", "LittleFS /index.html not found. Upload LittleFS data folder.");
    }

    void handleApiStatus() {
        SensorReadings s = sensors.getLatest();
        StaticJsonDocument<512> doc;
        doc["device_id"] = DEVICE_ID;
        doc["room_id"] = wifiPortal.getRoomID();
        doc["local_time"] = getDhakaTimeString();
        doc["temp_c"] = roundf(s.temperature * 10.0f) / 10.0f;
        doc["rh"] = (int)roundf(s.humidity);
        doc["heat_index_c"] = roundf(s.heatIndex * 10.0f) / 10.0f;
        doc["heat_trend"] = sensors.getTrendString();
        doc["quality"] = s.qualityGood ? "good" : "bad";
        doc["status"] = getSafetyStatusString();
        doc["flame"] = s.flameDetected;
        doc["fire_latched"] = door.isFireLatched();
        doc["panic_active"] = isPanicBannerActive();
        doc["silence_refused"] = isSilenceRefusedActive();
        doc["door"] = door.getStateString();
        doc["last_card_uid"] = rfidManager.getLastUID();
        doc["rssi"] = wifiPortal.getRSSI();
        doc["chip_temp_c"] = roundf(getChipTemperature() * 10.0f) / 10.0f;
        doc["fw"] = FIRMWARE_VERSION;
        doc["uptime_s"] = getUptimeSeconds();
        doc["conn_state"] = cloud.isServerReachable() ? "LIVE" : "OFFLINE";

        String json;
        serializeJson(doc, json);
        server.send(200, "application/json", json);
    }

public:
    LocalWebServerManager() : server(80), serverStarted(false) {}

    void begin() {
        if (!LittleFS.exists("/index.html")) {
            File f = LittleFS.open("/index.html", "w");
            if (f) {
                f.print(F("<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>Smart Classroom</title>"
                          "<style>body{font-family:sans-serif;background:#f8fafc;padding:16px;color:#0f172a}.c{max-width:640px;margin:0 auto}.card{background:#fff;border-radius:12px;padding:14px;margin-bottom:12px;box-shadow:0 2px 6px rgba(0,0,0,.06)}"
                          ".g{display:grid;grid-template-columns:repeat(auto-fit,minmax(120px,1fr));gap:8px}.s{background:#f8fafc;padding:10px;border-radius:8px;text-align:center}.v{font-size:19px;font-weight:700}.l{font-size:11px;color:#64748b}"
                          ".b{padding:4px 10px;border-radius:999px;font-size:12px;font-weight:700;background:#e2e8f0}.live{background:#dcfce7;color:#166534}.off{background:#fee2e2;color:#991b1b}"
                          ".ban{display:none;padding:12px;border-radius:10px;background:#dc2626;color:#fff;font-weight:700;text-align:center;margin-bottom:12px}.show{display:block}"
                          ".row{display:flex;justify-content:space-between;padding:4px 0;border-bottom:1px solid #f1f5f9;font-size:13px}</style></head>"
                          "<body><div class='c'><div style='display:flex;justify-content:space-between;align-items:center;margin-bottom:12px'>"
                          "<h2 style='margin:0'>Smart Classroom (<span id='device_id'>-</span>)</h2><div><span class='b' id='room_id'>-</span> <span class='b' id='conn_state'>OFFLINE</span></div></div>"
                          "<div id='fire_b' class='ban'>FIRE - EVACUATE</div><div id='panic_b' class='ban' style='background:#ea580c'>PANIC ACTIVATED</div><div id='sil_b' class='ban' style='background:#ea580c'>CANNOT SILENCE - FLAME PRESENT</div>"
                          "<div class='card'><div class='g'><div class='s'><div class='v' id='status'>SAFE</div><div class='l'>Status</div></div>"
                          "<div class='s'><div class='v' id='door'>LOCKED</div><div class='l'>Door</div></div><div class='s'><div class='v' id='flame'>NO</div><div class='l'>Flame</div></div>"
                          "<div class='s'><div class='v' id='quality'>good</div><div class='l'>Quality</div></div></div>"
                          "<div id='rst' style='display:none;margin-top:10px;text-align:center'><button onclick=\"fetch('/api/fire/reset',{method:'POST'}).then(r)\" style='padding:8px 14px;background:#dc2626;color:#fff;border:none;border-radius:8px;font-weight:700'>Reset Fire</button></div></div>"
                          "<div class='card'><div class='g'><div class='s'><div class='v'><span id='temp_c'>-</span> C</div><div class='l'>Temp</div></div>"
                          "<div class='s'><div class='v'><span id='rh'>-</span> %</div><div class='l'>Humidity</div></div><div class='s'><div class='v'><span id='heat_index_c'>-</span> C</div><div class='l'>Heat Index</div></div>"
                          "<div class='s'><div class='v' id='heat_trend'>STABLE</div><div class='l'>Trend</div></div></div></div>"
                          "<div class='card'><div class='row'><span>Local Time (Asia/Dhaka)</span><strong id='local_time'>-</strong></div>"
                          "<div class='row'><span>Last Card UID</span><strong id='last_card_uid'>-</strong></div>"
                          "<div class='row'><span>Wi-Fi RSSI</span><strong><span id='rssi'>-</span> dBm</strong></div>"
                          "<div class='row'><span>Chip Temp</span><strong><span id='chip_temp_c'>-</span> C</strong></div>"
                          "<div class='row'><span>Firmware</span><strong id='fw'>-</strong></div>"
                          "<div class='row'><span>Uptime</span><strong><span id='uptime_s'>0</span> s</strong></div></div></div>"
                          "<script>async function r(){try{const d=await(await fetch('/api/status')).json();"
                          "['device_id','room_id','status','door','quality','rh','heat_trend','local_time','last_card_uid','rssi','fw','uptime_s'].forEach(k=>document.getElementById(k).textContent=d[k]);"
                          "document.getElementById('temp_c').textContent=d.temp_c.toFixed(1);document.getElementById('heat_index_c').textContent=d.heat_index_c.toFixed(1);"
                          "document.getElementById('chip_temp_c').textContent=d.chip_temp_c.toFixed(1);document.getElementById('flame').textContent=d.flame?'DETECTED':'NO';"
                          "const c=document.getElementById('conn_state');c.textContent=d.conn_state;c.className='b '+(d.conn_state==='LIVE'?'live':'off');"
                          "document.getElementById('fire_b').className='ban'+(d.fire_latched?' show':'');document.getElementById('panic_b').className='ban'+(d.panic_active?' show':'');"
                          "document.getElementById('sil_b').className='ban'+(d.silence_refused?' show':'');document.getElementById('rst').style.display=d.fire_latched?'block':'none';}catch(e){}}"
                          "r();setInterval(r,1500);</script></body></html>"));
                f.close();
                Serial.println("[STORAGE] Seeded /index.html into LittleFS");
            }
        }
        server.on("/", HTTP_GET, [this]() { this->handleRoot(); });
        server.on("/setup", HTTP_POST, [this]() { this->handleSetupSubmit(); });
        server.on("/api/status", HTTP_GET, [this]() { this->handleApiStatus(); });

        server.on("/api/fire/reset", HTTP_POST, [this]() {
            performFireReset("Local Web (Teacher/Admin)");
            this->server.send(200, "application/json", "{\"ok\":true}");
        });

        server.on("/api/teacher/add", HTTP_POST, [this]() {
            String uid = this->server.arg("uid");
            bool ok = rfidManager.addTeacherTag(uid);
            this->server.send(200, "application/json",
                              "{\"ok\":" + String(ok ? "true" : "false") + ",\"uid\":\"" + uid + "\"}");
        });

        server.onNotFound([this]() {
            if (wifiPortal.isSetupMode()) {
                this->handleSetupPage();
            } else {
                this->server.send(404, "text/plain", "404 Not Found");
            }
        });

        server.begin();
        serverStarted = true;
        Serial.println("[NET] Local HTTP server started on port 80");
    }

    void handleClient() {
        if (serverStarted) {
            server.handleClient();
        }
    }
};

extern LocalWebServerManager localServer;

#endif // LOCAL_WEBSERVER_H
