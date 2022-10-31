#include <esp_wifi.h>
#include "wifimanager/wifiManager.h"
#include <Arduino.h>
#include "webserver/webServer.h"
#include "freertos/FreeRTOS.h"

void setup(){
    Serial.begin(115200);
    WiFiManager::setupWifi();
    WebServer::startWebServer(WiFiManager::startStationMode);
}

void loop(){
    vTaskDelay(10);
}