#include "app.h"
#include <WiFi.h>
#include <Arduino.h>

void initWiFi(const char* ssid, const char* password){
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while(WiFi.status() != WL_CONNECTED){
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
}

bool isWiFiConnected(){
    return WiFi.status() == WL_CONNECTED;
}
