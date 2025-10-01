#include "app.h"
#include <Arduino.h>

static int IR_PIN;
static volatile int parcelCount = 0;
static volatile unsigned long lastTriggerTime = 0;
const unsigned long debounceTime = 250; // กันสัญญาณเด้ง (ms)

void IR_ISR() {
    unsigned long now = millis();
    if (now - lastTriggerTime > debounceTime) {  
        parcelCount++;
        lastTriggerTime = now;
    }
}

void setupIR(int pin){
    IR_PIN = pin;
    pinMode(IR_PIN, INPUT);

    attachInterrupt(digitalPinToInterrupt(IR_PIN), IR_ISR, FALLING);

    parcelCount = 0;
    lastTriggerTime = 0;
    Serial.println("IR Sensor ready (Interrupt Mode + Debounce)");
}

int getParcelCount(){
    return parcelCount;
}

void resetParcelCount() {
    parcelCount = 0;
    Serial.println("📦 Parcel count reset to 0");
}