#include <esp_wifi.h>
#include "wifiManager.h"
#include <Arduino.h>

void setup(){
    Serial.begin(115200);
    WiFiManager::setupWifi();
}

void loop(){
}