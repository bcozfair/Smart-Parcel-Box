#include "app.h"
#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

// OLED 128x64 I2C
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);

String lastStatus = "";
int lastCount = -1;

// Global variable
ScreenState currentScreen = SCREEN_MAIN;
unsigned long triggerStartTime = 0;
const unsigned long TIMEOUT_MS = 10000;

void refreshDisplay() {
    if(currentScreen != SCREEN_MAIN) return;

    display.clearBuffer();
    display.setFont(u8g2_font_etl16thai_t);

    display.drawUTF8(0, 15, ("WiFi: " + lastStatus).c_str());
    display.drawUTF8(0, 35, ("Parcel: " + String(lastCount)).c_str());
    display.drawUTF8(0, 55, ("Fill: " + String(lastFillPercent) + "%").c_str());

    display.sendBuffer();
}


void initDisplay(){
    display.begin();
    lastStatus = "Connected";
    lastCount = 0;
    refreshDisplay();
}

void updateDisplayStatus(const String &status){
    if(status == lastStatus) return;
    lastStatus = status;
    refreshDisplay();
    Serial.println("WiFi: " + status);
}

void updateDisplayCount(int count){
    if(count == lastCount) return;
    lastCount = count;
    refreshDisplay();
}

void updateDisplayFill(int fill) {
    if (fill == lastFillPercent) return;  
    lastFillPercent = fill;
    refreshDisplay();
}

void showTriggerMessage() {
    if(currentScreen == SCREEN_WAIT_PACKAGE) return; // ไม่ซ้ำ
    currentScreen = SCREEN_WAIT_PACKAGE;

    display.clearBuffer();
    display.setFont(u8g2_font_etl24thai_t);

    // บรรทัดแรก "Place"
    int16_t x1 = (128 - display.getUTF8Width("Place")) / 2;
    display.drawUTF8(x1, 28, "Place");

    // บรรทัดสอง "Parcel"
    int16_t x2 = (128 - display.getUTF8Width("Parcel")) / 2;
    display.drawUTF8(x2, 60, "Parcel");

    display.sendBuffer();

    triggerStartTime = millis(); // เริ่มจับเวลา
    Serial.println("📦 วางพัสดุ");
}

void backToMainScreen() {
    if(currentScreen == SCREEN_MAIN) return; // กันซ้ำ
    currentScreen = SCREEN_MAIN;
    refreshDisplay();
    Serial.println("กลับไปหน้าจอหลัก");
}


void logEvent(const String &eventMsg){
    Serial.println("Event: " + eventMsg);
}
