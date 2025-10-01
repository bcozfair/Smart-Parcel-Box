#include "app.h"
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// --------------------
// WiFi / Telegram
// --------------------
const char* ssid = "ํYOUR_WIFI";
const char* password = "YOUR_password";
const char* botToken = "YOUR_botToken:botToken";
const char* chatID = "YOUR_chatID";

// --------------------
// Hardware pins
// --------------------
const int PIR_PIN = 34;
const int IR_PIN = 14;
const int TRIG_PIN = 26;
const int ECHO_PIN = 25;
const int SERVO_PIN = 15;
const int BUZZER_PIN = 27;
const int BOX_HEIGHT = 28;  // cm

// --------------------
// Global state
// --------------------
int lastParcelCount = 0;
extern ScreenState currentScreen;
extern unsigned long triggerStartTime;
extern const unsigned long TIMEOUT_MS;

bool authorizedUnlock = false;
bool alertActive = false;
int lastFillPercent = 0;    // baseline fill สำหรับ update display / ของใหม่
int theftBaselineFill = 0;  // baseline สำหรับตรวจจับของหาย
int lastServoPos = 0;       // 0 = locked, 90 = unlocked

unsigned long lastLoopTime = 0;
const unsigned long loopInterval = 200;  // ms
unsigned long lastAlertTime = 0;

const float THEFT_THRESHOLD = 5.0;  // % ลดลงถึงจะเตือน
const float LOW_BASELINE = 10.0;    // baseline ต่ำสุดที่จะใช้ logic พิเศษ

// --------------------
// Helper functions
// --------------------
void updateDisplayAll(const String& status, int count, int fill) {
  updateDisplayStatus(status);
  updateDisplayCount(count);
  updateDisplayFill(fill);
}

void handleNewParcel(int count) {
  // วัดค่าความจุปัจจุบันจาก ultrasonic จริงๆ
  int currentFillPercent = getBoxFillPercent();

  // ตรวจสอบค่าผิดปกติ (0% อาจเกิดจาก measurement error)
  if (currentFillPercent == 0) {
    // ลองวัดใหม่หลังจาก delay สักครู่
    delay(100);
    currentFillPercent = getBoxFillPercent();

    // ถ้ายังเป็น 0% ให้ใช้ค่าก่อนหน้า (ป้องกันการ reset baseline เป็น 0)
    if (currentFillPercent == 0 && lastFillPercent > 0) {
      currentFillPercent = lastFillPercent;
      Serial.println("⚠️ ค่า ultrasonic ผิดปกติ ใช้ค่าก่อนหน้าแทน");
    }
  }

  String msg = "📦 ตรวจพบพัสดุใหม่\n"
               "จำนวนพัสดุ: "
               + String(count) + "\nความจุ: " + String(currentFillPercent) + "%";

  Serial.println(msg);

  if (isWiFiConnected()) {
    sendTelegramMessage(msg);
  }

  // อัปเดต baseline ด้วยค่าจริงที่วัดได้ (เฉพาะค่าที่น่าเชื่อถือ)
  if (currentFillPercent > 0 || lastFillPercent == 0) {
    lastFillPercent = currentFillPercent;
    Serial.printf("[Baseline Updated] lastFillPercent=%d%%\n", lastFillPercent);
  } else {
    Serial.printf("⚠️ ไม่อัปเดต baseline ค่าไม่น่าเชื่อถือ: %d%%\n", currentFillPercent);
  }
}

void triggerTheftAlert() {
  if (!alertActive) {
    alertActive = true;
    Serial.println("🚨 Trigger Theft Alert! 🚨");
    sendTelegramMessage("🚨 ตรวจพบพัสดุหาย! 🚨");
  }
  buzzerOn(5000);
}

void stopTheftAlert() {
  alertActive = false;
  int currentFill = getBoxFillPercent();

  // รีเซ็ตทั้งสอง baseline
  lastFillPercent = currentFill;
  theftBaselineFill = currentFill;

  Serial.printf("✅ Stop Alert called, baselines reset to %d%%\n", currentFill);
  sendTelegramMessage("✅ หยุดการแจ้งเตือนแล้ว\nความจุกล่อง ถูกรีเซ็ตเป็น " + String(currentFill) + "%");
}

// --------------------
// Setup
// --------------------
void setup() {
  Serial.begin(115200);
  Serial.println("=== Smart Parcel Box Booting... ===");

  initWiFi(ssid, password);
  initTelegram(botToken, chatID);

  // เรียกส่งข้อความต้อนรับทันทีเมื่อ WiFi เชื่อมต่อ
  sendWelcomeMessage();

  initPIR(PIR_PIN, "http://192.168.43.212/capture");
  setupIR(IR_PIN);

  initDisplay();
  updateDisplayAll(isWiFiConnected() ? "Connected" : "Failed",
                   getParcelCount(),
                   getBoxFillPercent());

  initUltrasonic(TRIG_PIN, ECHO_PIN, BOX_HEIGHT);
  initServoLock(SERVO_PIN, 0);  // 0 = locked
  lastServoPos = 0;
  initBuzzer(BUZZER_PIN);

  lastParcelCount = getParcelCount();
  lastFillPercent = getBoxFillPercent();
  theftBaselineFill = lastFillPercent;  // ← เพิ่มบรรทัดนี้

  Serial.printf("Init Done | Parcels=%d | Fill=%d%% | Servo=%d (0=lock)\n",
                lastParcelCount, lastFillPercent, lastServoPos);
}

// --------------------
// Loop
// --------------------
void loop() {
  unsigned long now = millis();
  if (now - lastLoopTime < loopInterval) return;
  lastLoopTime = now;

  updateBuzzer();
  handleTelegramCommands();

  bool pirDetected = checkMotion();
  int currentParcelCount = getParcelCount();
  int currentFillPercent = getBoxFillPercent();

  // PIR ตรวจจับ
  if (pirDetected) {
    if (currentScreen == SCREEN_MAIN) {
      showTriggerMessage();
      buzzerOn(10);
    }
  }

  // Timeout หน้ารอวางพัสดุ
  if (currentScreen == SCREEN_WAIT_PACKAGE) {
    if (currentParcelCount > lastParcelCount) {
      Serial.println("✅ Parcel Added, returning to main screen");
      backToMainScreen();
    } else if (millis() - triggerStartTime > TIMEOUT_MS) {
      Serial.println("⏳ Timeout, returning to main screen");
      backToMainScreen();
    }
  }

  // มีพัสดุใหม่
  if (currentParcelCount > lastParcelCount) {
    for (int i = lastParcelCount + 1; i <= currentParcelCount; i++) {
      handleNewParcel(i);  // ส่งทีละ increment
    }
    lastParcelCount = currentParcelCount;

    // อัปเดตค่าความจุปัจจุบันหลังจากเพิ่มพัสดุแล้ว
    currentFillPercent = getBoxFillPercent();
    // อัปเดต baseline สำหรับตรวจจับของหาย
    theftBaselineFill = currentFillPercent;
  }

  // อัปเดต Display
  if (currentScreen == SCREEN_MAIN) {
    updateDisplayAll(
      isWiFiConnected() ? "Connected" : "Failed",
      currentParcelCount,
      currentFillPercent);
  }

  // ตรวจสอบพัสดุหายจาก Fill% ลดลง
  if (!alertActive && lastServoPos == 0 && !authorizedUnlock && currentParcelCount > 0) {
    float deltaPercent = theftBaselineFill - currentFillPercent;

    if (deltaPercent >= THEFT_THRESHOLD) {
      Serial.printf("⚠️ Theft Suspected (delta >= %.1f%%)! Baseline=%d%% Now=%d%%\n",
                    THEFT_THRESHOLD, theftBaselineFill, currentFillPercent);
      triggerTheftAlert();
    }
  }

  // แจ้งเตือนซ้ำทุก 10 วิาถ้า alertActive
  if (alertActive && now - lastAlertTime > 10000) {
    Serial.println("🔔 Repeat Theft Alert");
    sendTelegramMessage("🚨 พัสดุถูกขโมย! 🚨");
    buzzerOn(5000);
    lastAlertTime = now;
  }
}
