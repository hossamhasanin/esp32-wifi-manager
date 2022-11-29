#include <esp_wifi.h>
#include "wifimanager/wifiManager.h"
#include <Arduino.h>
#include "webserver/webServer.h"
#include "freertos/FreeRTOS.h"

void setup(){
    Serial.begin(115200);
    WiFiManager::setupWifi([](){
        Serial.println("Connected to wifi");
    }, [](){
        Serial.println("Lost wifi connection");
    });
    WebServer::startWebServer(WiFiManager::startStationMode);
}

void loop(){
    vTaskDelay(10);
}