#include "app.h"
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <HTTPClient.h>

static WiFiClientSecure secured_client;
static UniversalTelegramBot* bot;
static String telegramChatID;

// Auto reset state
static unsigned long lastLockTime = 0;
static bool waitingForReset = false;

// ฟังก์ชันเช็คสถานะกล้อง
bool isCameraOnline() {
  if (WiFi.status() != WL_CONNECTED) return false;

  HTTPClient http;
  http.begin("http://192.168.43.212/");  // 👈 IP ของ ESP32-CAM
  int httpCode = http.GET();
  http.end();

  return (httpCode == 200);  // ถ้า response code = 200 → กล้องออนไลน์
}

void initTelegram(const char* botToken, const char* chatID) {
  secured_client.setInsecure();
  bot = new UniversalTelegramBot(botToken, secured_client);
  telegramChatID = String(chatID);
}

void sendTelegramMessage(const String& msg) {
  if (bot) {
    bot->sendMessage(telegramChatID.c_str(), msg, "");
    Serial.println("ส่ง Telegram สำเร็จ");
  }
}

void sendWelcomeMessage() {
  if (!bot) return;

  String welcome = "🗄️ ยินดีต้อนรับสู่ Smart Parcel Box\n\n";
  welcome += "⬇️ กดปุ่มคำสั่งด้านล่าง";

  // ✅ Reply Keyboard แบบ array ธรรมดา
  String keyboardJson =
    "[[\"ล็อค\", \"ปลดล็อค\", \"สถานะ\"],"
    "[\"ถ่ายรูป\", \"รีเซ็ต\", \"หยุดแจ้งเตือน\"]]";

  bot->sendMessageWithReplyKeyboard(
    telegramChatID,
    welcome,
    "",            // parse_mode ไม่ต้องใส่
    keyboardJson,  // keyboard
    true,          // resize_keyboard
    false          // one_time_keyboard
  );

  Serial.println("✅ ส่ง Welcome Message พร้อม Reply Keyboard แล้ว");
}


void handleTelegramCommands() {
  if (!isWiFiConnected() || !bot) return;

  // ✅ Auto Reset: หลังจาก lock แล้ว 5 วิ ถ้ากล่องว่าง → reset
  if (waitingForReset && millis() - lastLockTime > 5000) {
    if (getBoxFillPercent() == 0 && getParcelCount() > 0) {
      resetParcelCount();
      lastParcelCount = 0;                    // ← sync ค่า
      lastFillPercent = getBoxFillPercent();  // ← sync baseline
      theftBaselineFill = lastFillPercent;    // <<< เพิ่มตรงนี้
      sendTelegramMessage("📤 กล่องถูกล็อกและว่าง → Reset จำนวนพัสดุอัตโนมัติ");
    }
    waitingForReset = false;
  }

  int numNewMessages = bot->getUpdates(bot->last_message_received + 1);

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot->messages[i].chat_id);
    String text = bot->messages[i].text;

    if (chat_id != telegramChatID) continue;

    // แปลงข้อความภาษาไทยเป็นคำสั่ง
    if (text == "ล็อค") text = "/lock";
    else if (text == "ปลดล็อค") text = "/unlock";
    else if (text == "สถานะ") text = "/status";
    else if (text == "หยุดแจ้งเตือน") text = "/stopalert";
    else if (text == "รีเซ็ต") text = "/reset";
    else if (text == "ถ่ายรูป") text = "/photo";

    if (text == "/lock") {
      setServoLock(0);
      bot->sendMessage(chat_id.c_str(), "🔒 ล็อคแล้ว", "");
      lastLockTime = millis();
      waitingForReset = true;
      authorizedUnlock = false;                 // reset flag เมื่อกลับไป lock
      theftBaselineFill = getBoxFillPercent();  // baseline ใหม่หลังล็อค
    } else if (text == "/unlock") {
      setServoLock(90);
      bot->sendMessage(chat_id.c_str(), "🔓 ปลดล็อคแล้ว", "");
      authorizedUnlock = true;
    } else if (text == "/stopalert") {
      stopTheftAlert();
      bot->sendMessage(chat_id.c_str(), "🚨 การแจ้งเตือนถูกปิดแล้ว", "");
    } else if (text == "/status") {
      String statusMsg = "📊 สถานะปัจจุบัน\n";
      statusMsg += "📡 WiFi: " + String(isWiFiConnected() ? "ทำงาน" : "ล้มเหลว") + "\n";
      statusMsg += "📦 จำนวนพัสดุ: " + String(getParcelCount()) + "\n";
      statusMsg += "📥 ความจุกล่อง: " + String(getBoxFillPercent()) + "%\n";

      if (isCameraOnline()) {
        statusMsg += "📷 กล้อง: ทำงาน\n";
      } else {
        statusMsg += "📷 กล้อง: ไม่ทำงาน\n";
      }

      if (alertActive) {
        statusMsg += "🚨 สถานะ: มีการแจ้งเตือนโจรกรรม!\n";
      } else {
        statusMsg += "✅ สถานะ: ปกติ\n";
      }

      bot->sendMessage(chat_id.c_str(), statusMsg, "");
    } else if (text == "/reset") {
      resetParcelCount();
      lastParcelCount = 0;                    // ← sync ค่า
      lastFillPercent = getBoxFillPercent();  // ← sync baseline
      theftBaselineFill = lastFillPercent;    // <<< เพิ่มตรงนี้
      bot->sendMessage(chat_id.c_str(), "♻️ Reset จำนวนพัสดุเรียบร้อยแล้ว", "");
    }

    else if (text == "/photo") {
      // TODO: สั่งถ่ายรูปจาก ESP32-CAM (placeholder)
      bot->sendMessage(chat_id.c_str(), "📷 รูปถ่ายจากคำสั่ง", "");
    } else if (text == "/start") {
      sendWelcomeMessage();
    } else {
      bot->sendMessage(chat_id.c_str(), "⬇️ ใช้ปุ่มคำสั่งด้านล่างเท่านั้น!!", "");
    }
  }
}