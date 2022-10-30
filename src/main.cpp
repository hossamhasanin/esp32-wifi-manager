#include <esp_wifi.h>
#include "wifiManager.h"
#include <Arduino.h>
#include "webServer.h"
#include "freertos/FreeRTOS.h"

void setup(){
    Serial.begin(115200);
    WiFiManager::setupWifi();
    WebServer::startWebServer();
}

void loop(){
    vTaskDelay(10);
}