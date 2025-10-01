#include "app.h"
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

// ----------------------
// PIR Settings
// ----------------------
static int pirPin;
static String esp32CamURL;

// ---------- Motion control ----------
static unsigned long lastTriggerTime = 0;
static unsigned long lastHighTime = 0;

// ตั้งค่าตามสถานการณ์ส่งพัสดุ
static const unsigned long stableTime   = 50;     // ms ลดเพื่อ trigger ทันที
static const unsigned long motionHold   = 5000;   // ms ถือว่า motion ต่อเนื่อง
static const unsigned long cooldown     = 15000;  // ms กัน trigger ซ้ำ 30 วิ

// Debug raw signal
static int lastPirState = -1;

void initPIR(int pin, const String &cameraURL){
    pirPin = pin;
    esp32CamURL = cameraURL;
    pinMode(pirPin, INPUT);  
}

bool checkMotion(){
    int pirState = digitalRead(pirPin);
    unsigned long now = millis();

    // ---------- Debug raw signal ----------
    if(pirState != lastPirState) {
        Serial.printf("[DEBUG] PIR raw state: %d  millis: %lu\n", pirState, now);
        lastPirState = pirState;
    }
    // --------------------------------------

    // ถ้า HIGH → update lastHighTime
    if(pirState == HIGH){
        lastHighTime = now;
    }

    // ถือว่า motion ยัง active ถ้า HIGH ล่าสุด < motionHold
    bool motionActive = (now - lastHighTime) < motionHold;

    // Trigger ถ้า cooldown หมด และ motionActive
    if(motionActive && (now - lastTriggerTime > cooldown)){
        Serial.println("🚨 Motion detected! Sending request...");
        lastTriggerTime = now;

        // ส่ง HTTP request หรือถ่ายรูป
        if(WiFi.status() == WL_CONNECTED){
            HTTPClient http;
            http.begin(esp32CamURL);
            int httpCode = http.GET();
            if(httpCode > 0){
                Serial.printf("Request sent, response code: %d\n", httpCode);
            } else {
                Serial.printf("Failed to send request: %s\n", http.errorToString(httpCode).c_str());
            }
            http.end();
        }
        return true;
    }

    return false;
}
