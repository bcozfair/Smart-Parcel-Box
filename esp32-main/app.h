#pragma once
#include <Arduino.h>  // ต้อง include ก่อนใช้งาน String
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <U8g2lib.h>
#include <ESP32Servo.h>

// --------------------
// Display Module
// --------------------
enum ScreenState {
  SCREEN_MAIN,
  SCREEN_WAIT_PACKAGE
};

extern ScreenState currentScreen;
extern unsigned long triggerStartTime;
extern const unsigned long TIMEOUT_MS;
extern int lastFillPercent;
extern String lastStatus;
extern int lastCount;
extern int lastParcelCount;
extern bool authorizedUnlock;
extern int theftBaselineFill;

void initDisplay();
void updateDisplayStatus(const String& status);
void updateDisplayCount(int count);
void updateDisplayFill(int fillPercent);
void showTriggerMessage();
void backToMainScreen();
void logEvent(const String& eventMsg);

// --------------------
// WiFi Module
// --------------------
void initWiFi(const char* ssid, const char* password);
bool isWiFiConnected();

// --------------------
// PIR Module
// --------------------
void initPIR(int pin, const String& cameraURL);
bool checkMotion();

// --------------------
// IR Module
// --------------------
void setupIR(int pin);
int getParcelCount();
void resetParcelCount();  // ← ต้องประกาศ สำหรับ manual/auto reset

// --------------------
// Telegram Module
// --------------------
void initTelegram(const char* botToken, const char* chatID);
void sendTelegramMessage(const String& msg);
void handleTelegramCommands();
void sendWelcomeMessage();  // ← ต้องมี (เพื่อให้ esp32.ino รู้จัก)
bool isCameraOnline();      // ← ถ้าจะใช้จากที่อื่นด้วย

// --------------------
// Ultrasonic Module
// --------------------
void initUltrasonic(int trigPin, int echoPin, int boxHeight);
long measureDistance();  // เพิ่มบรรทัดนี้
int getBoxFillPercent();
int getStableFillPercent();  // เพิ่มบรรทัดนี้

// --------------------
// Buzzer Module
// --------------------
void initBuzzer(int pin);
void buzzerOn(int durationMs);
void updateBuzzer();  // เพิ่มบรรทัดนี้

// --------------------
// Servo Lock Module
// --------------------
void initServoLock(int sigPin, int lockedPos = 0);
void setServoLock(int pos);

// เพิ่ม state สำหรับการแจ้งเตือน
extern bool alertActive;
void triggerTheftAlert();
void stopTheftAlert();
