#include "app.h"
#include <Arduino.h>

static int buzzerPin;
static unsigned long buzzerStartTime = 0;
static int buzzerDuration = 0;
static bool buzzerActive = false;

void initBuzzer(int pin){
    buzzerPin = pin;
    pinMode(buzzerPin, OUTPUT);
    digitalWrite(buzzerPin, LOW);
}

// กำหนด default value ที่นี่เท่านั้น
void buzzerOn(int durationMs){
    buzzerStartTime = millis();
    buzzerDuration = durationMs;
    buzzerActive = true;
    digitalWrite(buzzerPin, HIGH);
    Serial.println("Buzzer ON for " + String(durationMs) + "ms");
}

void updateBuzzer(){
    if(buzzerActive && (millis() - buzzerStartTime >= buzzerDuration)){
        digitalWrite(buzzerPin, LOW);
        buzzerActive = false;
        Serial.println("Buzzer OFF");
    }
}