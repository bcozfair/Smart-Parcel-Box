<div align="center">

<img src="https://capsule-render.vercel.app/api?type=waving&color=0:0ea5e9,50:6366f1,100:22d3ee&height=200&section=header&text=Smart%20Parcel%20Box&fontSize=42&fontColor=ffffff&fontAlignY=36&desc=IoT-Powered%20Intelligent%20Parcel%20Receiving%20System&descAlignY=55&descSize=15&animation=fadeIn" width="100%"/>

<br/>

<!-- Trophy Badge -->
<img src="https://img.shields.io/badge/%F0%9F%8F%86%20First%20Prize-Productive%20Learning%20Contest%202026-FFD700?style=for-the-badge"/>

<br/><br/>

<img src="https://img.shields.io/badge/ESP32-0ea5e9?style=flat-square&logo=espressif&logoColor=white"/>
<img src="https://img.shields.io/badge/ESP32--CAM-6366f1?style=flat-square&logo=espressif&logoColor=white"/>
<img src="https://img.shields.io/badge/C%2FC++-22d3ee?style=flat-square&logo=cplusplus&logoColor=white"/>
<img src="https://img.shields.io/badge/Arduino%20IDE-0ea5e9?style=flat-square&logo=arduino&logoColor=white"/>
<img src="https://img.shields.io/badge/IoT-6366f1?style=flat-square&logo=internetofthings&logoColor=white"/>

<br/><br/>

<a href="https://github.com/bcozfair/Smart-Parcel-Box/stargazers"><img src="https://img.shields.io/github/stars/bcozfair/Smart-Parcel-Box?color=22d3ee&style=flat-square&logo=github"/></a>
<a href="https://github.com/bcozfair/Smart-Parcel-Box/network/members"><img src="https://img.shields.io/github/forks/bcozfair/Smart-Parcel-Box?color=6366f1&style=flat-square&logo=github"/></a>
<a href="https://github.com/bcozfair/Smart-Parcel-Box/issues"><img src="https://img.shields.io/github/issues/bcozfair/Smart-Parcel-Box?color=0ea5e9&style=flat-square"/></a>

</div>

---

## 🏆 รางวัล

> **🥇 รางวัลชนะเลิศอันดับ 1** — Productive Learning Contest (ต้นปี 2026)

---

## 📌 เกี่ยวกับโปรเจกต์

**Smart Parcel Box** คือระบบกล่องรับพัสดุอัจฉริยะที่ขับเคลื่อนด้วยเทคโนโลยี **IoT** ออกแบบมาเพื่อเพิ่มความสะดวกและความปลอดภัยในการรับพัสดุ โดยใช้ **ESP32** เป็นตัวควบคุมหลัก และ **ESP32-CAM** สำหรับระบบกล้องตรวจสอบ ทั้งสองบอร์ดทำงานร่วมกันเพื่อสร้างประสบการณ์รับพัสดุที่ปลอดภัยและอัตโนมัติ

---

## 🛠️ Tech Stack

| ส่วนประกอบ | รายละเอียด |
|-----------|-----------|
| ![ESP32](https://img.shields.io/badge/ESP32%20Main-0ea5e9?style=flat-square&logo=espressif&logoColor=white) | ควบคุมลอจิกหลัก, เซนเซอร์, ระบบล็อก |
| ![ESP32-CAM](https://img.shields.io/badge/ESP32--CAM-6366f1?style=flat-square&logo=espressif&logoColor=white) | จัดการระบบภาพ, ถ่ายภาพพัสดุ |
| ![C/C++](https://img.shields.io/badge/C%2FC++-22d3ee?style=flat-square&logo=cplusplus&logoColor=white) | ภาษาหลักในการเขียนโปรแกรม |
| ![Arduino IDE](https://img.shields.io/badge/Arduino%20IDE-0ea5e9?style=flat-square&logo=arduino&logoColor=white) | Development Environment |
| ![PlatformIO](https://img.shields.io/badge/PlatformIO-6366f1?style=flat-square&logo=platformio&logoColor=white) | Build System (รองรับ) |

---

## 📁 โครงสร้างโปรเจกต์

```
Smart-Parcel-Box/
├── esp32-main/       # โค้ดบอร์ดหลัก — ควบคุมลอจิก, เซนเซอร์, ระบบล็อก
│   └── ...
├── esp32-cam/        # โค้ดโมดูลกล้อง — ถ่ายภาพ, ตรวจสอบความปลอดภัย
│   └── ...
└── README.md
```

### หน้าที่ของแต่ละส่วน

**`esp32-main/`** — บอร์ดควบคุมหลัก
- อ่านค่าเซนเซอร์ตรวจจับพัสดุ
- ควบคุมระบบล็อก (Lock mechanism)
- เชื่อมต่อ Wi-Fi และรับส่งข้อมูล
- แจ้งเตือนเมื่อมีพัสดุมาส่ง

**`esp32-cam/`** — โมดูลกล้อง (AI Thinker ESP32-CAM)
- ถ่ายภาพเมื่อมีพัสดุถูกหย่อนลงกล่อง
- ตรวจสอบความปลอดภัยบริเวณกล่อง
- ส่งภาพไปยังระบบแจ้งเตือน

---

## 🚀 วิธีติดตั้งและใช้งาน

### 1. Clone โปรเจกต์

```bash
git clone https://github.com/bcozfair/Smart-Parcel-Box.git
cd Smart-Parcel-Box
```

### 2. ติดตั้ง Library ที่จำเป็น

เปิด **Arduino IDE** แล้วติดตั้ง Library ต่อไปนี้ผ่าน Library Manager:

```
- WiFi (built-in ESP32)
- PubSubClient     (MQTT)
- ESP32 Camera     (esp32-cam)
- ArduinoJson      (JSON parsing)
```

### 3. อัปโหลดโค้ดบอร์ดหลัก (ESP32)

1. เปิดโฟลเดอร์ `esp32-main/` ใน Arduino IDE
2. เลือกบอร์ด: **ESP32 Dev Module**
3. ตั้งค่า Wi-Fi credentials ในไฟล์โค้ด:
   ```cpp
   const char* ssid     = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```
4. กด **Upload** ⬆️

### 4. อัปโหลดโค้ดโมดูลกล้อง (ESP32-CAM)

1. เปิดโฟลเดอร์ `esp32-cam/` ใน Arduino IDE
2. เลือกบอร์ด: **AI Thinker ESP32-CAM**
3. ตั้งค่า Wi-Fi credentials ในไฟล์โค้ด
4. **ก่อนอัปโหลด:** ต่อสาย IO0 กับ GND เพื่อเข้า flash mode
5. กด **Upload** ⬆️ แล้วถอดสาย IO0-GND ออก กด Reset

### 5. ทดสอบระบบ

1. เปิด **Serial Monitor** (baud rate: `115200`)
2. ตรวจสอบว่าบอร์ดทั้งสองเชื่อมต่อ Wi-Fi สำเร็จ
3. ทดสอบโดยหย่อนสิ่งของลงในกล่อง
4. ระบบควรแจ้งเตือนและถ่ายภาพโดยอัตโนมัติ

---

## ⚠️ หมายเหตุสำคัญ

- 🔐 อย่า hardcode Wi-Fi password หรือ API key ลงใน source code ที่จะ push ขึ้น GitHub
- 🔋 ESP32-CAM กินไฟสูงขณะถ่ายภาพ — ควรใช้ power supply ที่จ่ายกระแสได้อย่างน้อย **2A**
- 📷 เลือกบอร์ด **AI Thinker** ให้ถูกต้องก่อนอัปโหลด esp32-cam มิฉะนั้นกล้องจะไม่ทำงาน

---

## 👤 Author

**Natchaphat (Fair)**

[![GitHub](https://img.shields.io/badge/GitHub-bcozfair-0ea5e9?style=flat-square&logo=github)](https://github.com/bcozfair)
[![LinkedIn](https://img.shields.io/badge/LinkedIn-natchaphat--fair-6366f1?style=flat-square&logo=linkedin)](https://www.linkedin.com/in/natchaphat-fair/)
[![Email](https://img.shields.io/badge/Email-bcozfair@gmail.com-22d3ee?style=flat-square&logo=gmail)](mailto:bcozfair@gmail.com)

---

<div align="center">

<img src="https://capsule-render.vercel.app/api?type=waving&color=0:22d3ee,50:6366f1,100:0ea5e9&height=100&section=footer" width="100%"/>

</div>
