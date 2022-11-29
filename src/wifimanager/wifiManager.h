#pragma once
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"


#define ESP_WIFI_SSID      "ESPTesting"
#define ESP_WIFI_PASS      "mypassword"
#define ESP_MAXIMUM_RETRY  3

#define SET_STATIC_IP IP4_ADDR(&ipInfo.ip, 192,168,220,222)
#define SET_STATIC_GATEWAY IP4_ADDR(&ipInfo.gw, 192,168,220,61)
#define SET_STATIC_NETMASK IP4_ADDR(&ipInfo.netmask, 255,255,255,0)

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group = NULL;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "WifiManager";

namespace WiFiManager {

    typedef void (*OnSuccessfullyConncetedToWifi)();
    typedef void (*OnWifiConnectionLost)();


    void setupWifi(OnSuccessfullyConncetedToWifi onSuccessfullyConncetedToWifi, OnWifiConnectionLost onWifiConnectionLost);
    
    void wifiEventHandler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data);

    void startAPMode();
    void startStationMode(const char* ssid, const char* password);

    static void setWifiCredintials(uint8_t* ssidInput , uint8_t* passInput , const char* ssid , const char* password){
        for (int i = 0; i < strlen(ssid); i++){
            (*ssidInput) = ssid[i];
            ssidInput++;
        }
        for (int i = 0; i < strlen(password); i++){
            (*passInput) = password[i];
            passInput++;
        }
    }
    
}