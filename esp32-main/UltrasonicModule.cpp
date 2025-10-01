#include "app.h"
#include <Arduino.h>

static int trigPin, echoPin, boxHeight;

void initUltrasonic(int tPin, int ePin, int bHeight) {
  trigPin = tPin;
  echoPin = ePin;
  boxHeight = bHeight;

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

long measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);  // timeout 30ms
  if (duration == 0) return boxHeight;            // ถ้าวัดไม่ได้ให้คืนค่าสูงสุด

  return duration * 0.034 / 2;
}

// ฟังก์ชันวัดค่าเฉลี่ยเพื่อลด noise
int getStableFillPercent() {
  const int numReadings = 3;
  long totalDistance = 0;
  int validReadings = 0;

  for (int i = 0; i < numReadings; i++) {
    long distance = measureDistance();

    // กรองค่าผิดปกติ (ระยะทางไม่สมเหตุสมผล)
    if (distance > 0 && distance <= boxHeight) {
      totalDistance += distance;
      validReadings++;
    }
    delay(50);  // รอระหว่างการวัด
  }

  if (validReadings == 0) return 0;  // ไม่สามารถวัดค่าได้

  long avgDistance = totalDistance / validReadings;
  int fill = 100 - (int)((avgDistance * 100) / boxHeight);

  if (fill < 0) fill = 0;
  if (fill > 100) fill = 100;

  return fill;
}

// ฟังก์ชันเดียวสำหรับวัดระดับความจุ
int getBoxFillPercent() {
  return getStableFillPercent();  // ใช้ฟังก์ชันวัดค่าเฉลี่ยแทน
}