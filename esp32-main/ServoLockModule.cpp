#include "app.h"
#include <Arduino.h>
#include <ESP32Servo.h>

static Servo lockServo;
static int lastPos = -1;

void initServoLock(int sigPin, int lockedPos){
    lockServo.attach(sigPin);
    lockServo.write(lockedPos);
    lastPos = lockedPos;
}

void setServoLock(int pos){
    if(pos != lastPos){
        lockServo.write(pos);
        lastPos = pos;
        Serial.println("Servo set to pos: " + String(pos));
    }
}
