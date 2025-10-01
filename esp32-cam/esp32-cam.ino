#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <esp_system.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <WebServer.h>

WebServer server(80);

// --------- WiFi / Telegram config (เปลี่ยนตามของคุณ) ----------
const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";

String BOTtoken = "YOUR_BOTtoken:BOTtoken";
String CHAT_ID = "YOUR_CHAT_ID";

WiFiClientSecure clientTCP;
UniversalTelegramBot bot(BOTtoken, clientTCP);

// --------- App variables ----------
bool sendPhoto = false;
unsigned long lastTriggerTime = 0;
const unsigned long cooldown = 10000;  // 10s

#define FLASH_LED_PIN 4
bool flashState = LOW;

int botRequestDelay = 1000;
unsigned long lastTimeBotRan = 0;

// --------- CAMERA_MODEL_AI_THINKER (ใช้ pinout เดิมของคุณ) ----------
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

// ================= Camera init with retry and fallback =================
void configInitCameraWithRetry() {
  Serial.printf("configInitCameraWithRetry(): psramFound=%d freeHeap=%u\n", psramFound(), esp_get_free_heap_size());

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;
    config.fb_count = 1;
  } else {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = ESP_FAIL;
  for (int attempt = 1; attempt <= 3; ++attempt) {
    Serial.printf("Camera init attempt %d (xclk=%u frame=%d) ...\n", attempt, (unsigned)config.xclk_freq_hz, config.frame_size);
    err = esp_camera_init(&config);
    if (err == ESP_OK) {
      Serial.println("Camera init OK");
      // configure some defaults
      sensor_t* s = esp_camera_sensor_get();
      if (s) {
        s->set_brightness(s, 0);
        s->set_contrast(s, 2);
        s->set_saturation(s, -2);
        s->set_whitebal(s, 1);
        s->set_awb_gain(s, 1);
        s->set_wb_mode(s, 0);
        s->set_exposure_ctrl(s, 1);
        s->set_aec2(s, 0);
        s->set_ae_level(s, 0);
        s->set_aec_value(s, 300);
        s->set_gain_ctrl(s, 1);
        s->set_agc_gain(s, 0);
        s->set_gainceiling(s, (gainceiling_t)0);
        s->set_bpc(s, 0);
        s->set_wpc(s, 1);
        s->set_raw_gma(s, 1);
        s->set_lenc(s, 1);
        s->set_hmirror(s, 0);
        s->set_vflip(s, 0);
        s->set_dcw(s, 1);
        s->set_colorbar(s, 0);
        s->set_framesize(s, FRAMESIZE_VGA);
      }
      return;
    }
    Serial.printf("Camera init failed (0x%x)\n", err);
    esp_camera_deinit();
    delay(200);

    // Fallback adjustments for next attempt
    if (attempt == 1) {
      // reduce frame and xclk
      config.frame_size = FRAMESIZE_QVGA;
      config.xclk_freq_hz = 10000000;
    } else if (attempt == 2) {
      // further reduce
      config.frame_size = FRAMESIZE_QQVGA;
      config.xclk_freq_hz = 8000000;
    }
    // loop continue
  }

  Serial.printf("Camera init permanently failed with 0x%x\n", err);
  // Note: ไม่ restart อัตโนมัติที่นี่ เพื่อให้ user ดู log ได้
}

// ================= Handle incoming Telegram messages =================
void handleNewMessages(int numNewMessages) {
  Serial.printf("Handle New Messages: %d\n", numNewMessages);
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    String text = bot.messages[i].text;
    Serial.printf("From %s: %s\n", bot.messages[i].from_name.c_str(), text.c_str());

    // แปลงข้อความภาษาไทยเป็นคำสั่ง
    if (text == "ถ่ายรูป") text = "/photo";

    if (text == "/camera") {
      String welcome = "📦 ยินดีต้อนรับคุณ " + bot.messages[i].from_name + "\n";
      welcome += "นี่คือกล้องของ Smart Parcel Box คุณสามารถใช้คำสั่งต่อไปนี้ \n\n";
      welcome += "📷 /photo : ถ่ายภาพทันที\n";
      welcome += "💡 /flash : เปิด/ปิด ไฟแฟลช\n";
      bot.sendMessage(CHAT_ID, welcome, "");

    } else if (text == "/flash") {
      flashState = !flashState;
      digitalWrite(FLASH_LED_PIN, flashState);
      Serial.println("Flash toggled");

    } else if (text == "/photo") {
      sendPhoto = true;
      Serial.println("Photo requested via Telegram");
    }
  }
}

// ================= Send photo to Telegram (multipart HTTPS) =================
String sendPhotoTelegram() {
  const char* myDomain = "api.telegram.org";
  String getBody = "";

  camera_fb_t* fb = esp_camera_fb_get();
  if (fb) esp_camera_fb_return(fb);  // คืน buffer เก่า
  delay(50);                         // ให้ sensor update
  fb = esp_camera_fb_get();          // capture ล่าสุด
  if (!fb) {
    Serial.println("Camera capture failed");
    return "Camera capture failed";
  }

  if (clientTCP.connect(myDomain, 443)) {
    Serial.println("Connected to api.telegram.org");

    String head = "--theinfoflux\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + CHAT_ID + "\r\n--theinfoflux\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--theinfoflux--\r\n";

    uint32_t imageLen = fb->len;
    uint32_t totalLen = imageLen + head.length() + tail.length();

    clientTCP.println("POST /bot" + BOTtoken + "/sendPhoto HTTP/1.1");
    clientTCP.println("Host: " + String(myDomain));
    clientTCP.println("Content-Length: " + String(totalLen));
    clientTCP.println("Content-Type: multipart/form-data; boundary=theinfoflux");
    clientTCP.println();
    clientTCP.print(head);

    // send image buffer in chunks
    uint8_t* buf = fb->buf;
    size_t remaining = fb->len;
    while (remaining > 0) {
      size_t chunk = remaining > 1024 ? 1024 : remaining;
      clientTCP.write(buf, chunk);
      buf += chunk;
      remaining -= chunk;
    }

    clientTCP.print(tail);
    esp_camera_fb_return(fb);

    // wait and read response (with timeout)
    long start = millis();
    while (millis() - start < 10000) {
      while (clientTCP.available()) {
        char c = clientTCP.read();
        getBody += c;
      }
      if (getBody.length() > 0) break;
      delay(10);
    }
    clientTCP.stop();
    Serial.println("Telegram response:");
    Serial.println(getBody);
  } else {
    Serial.println("Connection to api.telegram.org failed.");
    getBody = "Connect failed";
  }
  return getBody;
}

// ================= Web server endpoint for external capture trigger =================
void handleCapture() {
  unsigned long now = millis();
  if (!sendPhoto && now - lastTriggerTime > cooldown) {
    sendPhoto = true;
    lastTriggerTime = now;
    server.send(200, "text/plain", "OK, photo will be sent!");
  } else {
    server.send(200, "text/plain", "Wait, camera busy or cooldown");
  }
}

void handleRoot() {
  String rootPage = "<h2>📦 Smart Parcel Box Camera</h2>";
  rootPage += "<p>กล้องออนไลน์ ✅</p>";
  rootPage += "<p>IP: " + WiFi.localIP().toString() + "</p>";
  rootPage += "<p>Flash: " + String(flashState ? "ON 💡" : "OFF") + "</p>";
  rootPage += "<p>ไปที่ <a href='/capture'>/capture</a> เพื่อถ่ายรูป</p>";

  server.send(200, "text/html", rootPage);
}

// ================= SETUP =================
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  // disable brownout detector
  Serial.begin(115200);
  delay(50);
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, flashState);

  Serial.println();
  Serial.println("Starting...");

  // Connect WiFi FIRST (helps with hotspot timing issues)
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.printf("Connecting to WiFi '%s' ", ssid);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi connected. IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi connect failed or timeout. Continuing (camera init will still try).");
  }

  // Set root certificate (if available in your project). If you don't have it,
  // you can use clientTCP.setInsecure() for testing (not recommended for production).
  // clientTCP.setInsecure(); // <-- uncomment for testing without cert
  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  // Init camera (with retries/fallback)
  configInitCameraWithRetry();

  // start web server route
  server.on("/capture", handleCapture);
  server.on("/", handleRoot);  // ✅ เพิ่ม root handler
  server.begin();
  Serial.println("HTTP server started");
}

// ================= LOOP =================
void loop() {
  server.handleClient();

  if (sendPhoto) {
    Serial.println("Preparing photo...");
    String resp = sendPhotoTelegram();
    Serial.println("sendPhotoTelegram done");
    sendPhoto = false;
  }

  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    if (numNewMessages) Serial.printf("Got %d new messages\n", numNewMessages);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
