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
/* The examples use WiFi configuration that you can set via project configuration menu
   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      "esp is here"
#define EXAMPLE_ESP_WIFI_PASS      "mypassword"
#define EXAMPLE_ESP_MAXIMUM_RETRY  5

static const char* ssid = "ESPTesting";
static const char* password = "mypassword";

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "WifiManager";

namespace WiFiManager {

    void setupWifi();
    
    void wifiEventHandler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data);

    void startAPMode();
    void startStationMode(const char* ssid, const char* password);

    void setWifiCredintials(uint8_t* ssidInput , uint8_t* passInput , const char* ssid , const char* password);

}