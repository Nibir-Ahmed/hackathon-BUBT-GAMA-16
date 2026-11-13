// rfid_access.h — RC522 13.56MHz RFID Reader on Custom SPI Pins
// Level 1 & Level 3 Compliant: Global SPI remap, NVS Teacher/Student lists, once-per-tap debounce

#ifndef RFID_ACCESS_H
#define RFID_ACCESS_H

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Preferences.h>
#include <vector>
#include "config.h"

enum CardRole {
    ROLE_UNKNOWN = 0,
    ROLE_STUDENT,
    ROLE_TEACHER
};

struct RFIDScanResult {
    bool hasCard;
    String uidString;
    CardRole role;
    const char* roleName;
};

class RFIDManager {
private:
    MFRC522 rfid;
    Preferences prefs;
    String lastScannedUID;
    String lastTapUID;
    unsigned long lastTapMillis;
    std::vector<String> teacherTags;
    std::vector<String> studentCards;

    void loadLists() {
        teacherTags.clear();
        studentCards.clear();
        prefs.begin("rfid_cfg", true);
        int tCnt = prefs.getInt("t_cnt", 0);
        for (int i = 0; i < tCnt; i++) {
            String key = "t_" + String(i);
            String u = prefs.getString(key.c_str(), "");
            if (u.length() > 0) teacherTags.push_back(u);
        }
        int sCnt = prefs.getInt("s_cnt", 0);
        for (int i = 0; i < sCnt; i++) {
            String key = "s_" + String(i);
            String u = prefs.getString(key.c_str(), "");
            if (u.length() > 0) studentCards.push_back(u);
        }
        prefs.end();
    }

    void saveTeacherList() {
        prefs.begin("rfid_cfg", false);
        prefs.putInt("t_cnt", (int)teacherTags.size());
        for (size_t i = 0; i < teacherTags.size(); i++) {
            String key = "t_" + String(i);
            prefs.putString(key.c_str(), teacherTags[i]);
        }
        prefs.end();
    }

    void saveStudentList() {
        prefs.begin("rfid_cfg", false);
        prefs.putInt("s_cnt", (int)studentCards.size());
        for (size_t i = 0; i < studentCards.size(); i++) {
            String key = "s_" + String(i);
            prefs.putString(key.c_str(), studentCards[i]);
        }
        prefs.end();
    }

public:
    RFIDManager() :
        rfid(PIN_RC522_SS, PIN_RC522_RST),
        lastScannedUID("-"),
        lastTapUID(""),
        lastTapMillis(0) {}

    void begin() {
        // Global SPI must be initialized with custom ESP32-S3 pins for MFRC522(SS, RST)
        SPI.begin(PIN_RC522_SCK, PIN_RC522_MISO, PIN_RC522_MOSI, PIN_RC522_SS);
        rfid.PCD_Init();
        delay(10);
        rfid.PCD_SetAntennaGain(MFRC522::RxGain_max);
        loadLists();
        Serial.printf("[RFID] RC522 initialized (SCK=%d MISO=%d MOSI=%d SS=%d RST=%d) | Teachers=%d Students=%d\n",
                      PIN_RC522_SCK, PIN_RC522_MISO, PIN_RC522_MOSI, PIN_RC522_SS, PIN_RC522_RST,
                      (int)teacherTags.size(), (int)studentCards.size());
    }

    bool addTeacherTag(String uid) {
        uid.trim();
        uid.toUpperCase();
        if (uid.length() == 0) return false;
        for (const auto& t : teacherTags) {
            if (t == uid) return true;
        }
        teacherTags.push_back(uid);
        saveTeacherList();
        return true;
    }

    void clearTeacherTags() {
        teacherTags.clear();
        saveTeacherList();
    }

    bool addStudentCard(String uid) {
        uid.trim();
        uid.toUpperCase();
        if (uid.length() == 0) return false;
        for (const auto& s : studentCards) {
            if (s == uid) return true;
        }
        studentCards.push_back(uid);
        saveStudentList();
        return true;
    }

    void clearStudentCards() {
        studentCards.clear();
        saveStudentList();
    }

    bool isTeacherTag(const String& uid) const {
        for (const auto& t : teacherTags) {
            if (t.equalsIgnoreCase(uid)) return true;
        }
        return false;
    }

    bool isStudentCard(const String& uid) const {
        for (const auto& s : studentCards) {
            if (s.equalsIgnoreCase(uid)) return true;
        }
        return false;
    }

    const std::vector<String>& getTeacherTags() const { return teacherTags; }
    const std::vector<String>& getStudentCards() const { return studentCards; }

    RFIDScanResult poll() {
        RFIDScanResult result;
        result.hasCard = false;
        result.role = ROLE_UNKNOWN;
        result.roleName = "UNKNOWN";
        result.uidString = "";

        if (!rfid.PICC_IsNewCardPresent()) {
            return result;
        }
        if (!rfid.PICC_ReadCardSerial()) {
            return result;
        }

        String uidStr = "";
        for (byte i = 0; i < rfid.uid.size; i++) {
            if (rfid.uid.uidByte[i] < 0x10) uidStr += "0";
            uidStr += String(rfid.uid.uidByte[i], HEX);
            if (i < rfid.uid.size - 1) uidStr += ":";
        }
        uidStr.toUpperCase();

        // Halt PICC immediately so holding the card on the reader does not re-trigger
        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();

        unsigned long now = millis();
        if (uidStr == lastTapUID && (now - lastTapMillis < 800)) {
            return result;
        }

        lastTapUID = uidStr;
        lastTapMillis = now;
        lastScannedUID = uidStr;

        result.hasCard = true;
        result.uidString = uidStr;

        if (isTeacherTag(uidStr)) {
            result.role = ROLE_TEACHER;
            result.roleName = "TEACHER";
        } else if (isStudentCard(uidStr)) {
            result.role = ROLE_STUDENT;
            result.roleName = "STUDENT";
        } else {
            result.role = ROLE_UNKNOWN;
            result.roleName = "UNKNOWN";
        }

        return result;
    }

    String getLastUID() const {
        return lastScannedUID;
    }

    bool isReaderConnected() {
        byte v = rfid.PCD_ReadRegister(MFRC522::VersionReg);
        return (v != 0x00 && v != 0xFF);
    }
};

extern RFIDManager rfidManager;

#endif // RFID_ACCESS_H
