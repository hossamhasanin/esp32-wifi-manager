#include "wifiManager.h"
#include <Arduino.h>


static bool apModeOn = false;
static bool isWifiOn = false;
static esp_netif_t* wifiAp;
static esp_netif_t* wifiSta;
static esp_netif_ip_info_t ipInfo;
static int retryToConnectSta = 0;



void WiFiManager::wifiEventHandler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data){
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        // start station mode
        Serial.println("WIFI_EVENT_STA_START");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        Serial.println("Wifi disconnected");
        if (retryToConnectSta < ESP_MAXIMUM_RETRY) {
            // retry to connect to the AP
            esp_wifi_connect();
            retryToConnectSta++;
            ESP_LOGI(TAG, "retry to connect to the AP");
            Serial.println("retry to connect to the AP");
        } else {
            // failed to connect to the AP
            Serial.println("failed to connect to the AP");
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        // connected to the AP successfully
        // get the IP address
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        Serial.println("got ip:");
        retryToConnectSta = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    } else if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        // a station connected to the AP
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;

        Serial.println("station join");
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        // a station disconnected from the AP
        
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        Serial.println("station disconnected");

    }
}

void initNvsMemory(){
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

}

nvs_handle_t openStorage(){
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvsHandle);
    if (err != ESP_OK) {
        Serial.println("Error (" + String(esp_err_to_name(err)) + ") opening NVS handle!");
        return 0;
    }
    return nvsHandle;
}

char* getStoredSsid(nvs_handle_t nvsHandle){
    size_t required_size;
    esp_err_t err = nvs_get_str(nvsHandle, "ssid", NULL, &required_size);
    if (err != ESP_OK) {
        Serial.println("Error (" + String(esp_err_to_name(err)) + ") reading ssid!");
        return NULL;
    }

    char* mssid = (char*) malloc(required_size);
    nvs_get_str(nvsHandle, "ssid", mssid, &required_size);
    
    return mssid;
}

char* getStoredPassword(nvs_handle_t nvsHandle){
    size_t required_size;
    esp_err_t err = nvs_get_str(nvsHandle, "password", NULL, &required_size);
    if (err != ESP_OK) {
        Serial.println("Error (" + String(esp_err_to_name(err)) + ") reading ssid!");
        return NULL;
    }
    char* mpassword = (char*) malloc(required_size);
    nvs_get_str(nvsHandle, "password", mpassword, &required_size);

    return mpassword;
}

void storeSsidToStorage(nvs_handle_t nvsHandle , const char* ssid){
    esp_err_t err = nvs_set_str(nvsHandle, "ssid", ssid);
    if (err != ESP_OK) {
        Serial.println("Error (" + String(esp_err_to_name(err)) + ") saving ssid!");
        return;
    }
    err = nvs_commit(nvsHandle);
    if (err != ESP_OK) {
        Serial.println("Error (" + String(esp_err_to_name(err)) + ") committing ssid!");
        return;
    }
}

void storePasswordToStorage(nvs_handle_t nvsHandle , const char* password){
    esp_err_t err = nvs_set_str(nvsHandle, "password", password);
    if (err != ESP_OK) {
        Serial.println("Error (" + String(esp_err_to_name(err)) + ") saving password!");
        return;
    }
    err = nvs_commit(nvsHandle);
    if (err != ESP_OK) {
        Serial.println("Error (" + String(esp_err_to_name(err)) + ") committing password!");
        return;
    }
}

void storeWifiCredentialsToStorage(const char* ssid, const char* password){
    nvs_handle_t nvsHandle = openStorage();

    if (strcmp(getStoredSsid(nvsHandle), ssid) == 0){
        Serial.println("ssid already stored");
        nvs_close(nvsHandle);
        return;
    }

    Serial.println("Storage opened");
    storeSsidToStorage(nvsHandle, ssid);
    Serial.println("ssid stored");
    storePasswordToStorage(nvsHandle, password);
    Serial.println("password stored");
    nvs_close(nvsHandle);
    Serial.println("Storage closed");
}

void WiFiManager::setupWifi(){

    //Initialize NVS
    initNvsMemory();

    // Initialize TCP/IP network interface (should be called only once in application)
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    esp_wifi_set_mode(WIFI_MODE_NULL);
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                    &WiFiManager::wifiEventHandler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                    &WiFiManager::wifiEventHandler, NULL, &instance_got_ip));
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    wifiAp = esp_netif_create_default_wifi_ap();
    wifiSta = esp_netif_create_default_wifi_sta();

    
    esp_netif_ip_info_t ipInfo;
    IP4_ADDR(&ipInfo.ip, 192,168,1,222);
    IP4_ADDR(&ipInfo.gw, 192,168,1,222);
    IP4_ADDR(&ipInfo.netmask, 255,255,255,0);
    ESP_ERROR_CHECK(esp_netif_dhcps_stop(wifiAp));
    ESP_ERROR_CHECK(esp_netif_set_ip_info(wifiAp, &ipInfo));
    ESP_ERROR_CHECK(esp_netif_dhcps_start(wifiAp));
    ESP_LOGI(TAG, "IP: " IPSTR, IP2STR(&ipInfo.ip));
    ESP_LOGI(TAG, "GW: " IPSTR, IP2STR(&ipInfo.gw));
    ESP_LOGI(TAG, "Mask: " IPSTR, IP2STR(&ipInfo.netmask));

    nvs_handle_t nvsHandle = openStorage();

    if (nvsHandle == 0){
        Serial.println("Error opening storage");
        return;
    }

    char* ssid = getStoredSsid(nvsHandle);
    char* password = getStoredPassword(nvsHandle);
    if (ssid == NULL || password == NULL){
        Serial.println("No stored wifi credentials");
        WiFiManager::startAPMode();
    } else {
        Serial.println("Found stored wifi credentials");
        WiFiManager::startStationMode(ssid, password);
    }

    nvs_close(nvsHandle);
}

void WiFiManager::startAPMode(){

    if (apModeOn){
        ESP_LOGI(TAG, "AP mode already on");
        return;
    }

    if (isWifiOn){
        ESP_ERROR_CHECK(esp_wifi_disconnect());
        ESP_ERROR_CHECK(esp_wifi_stop());
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
        if (s_wifi_event_group != NULL){
            vEventGroupDelete(s_wifi_event_group);
        }
    }

    ESP_LOGI(TAG, "IP: " IPSTR, IP2STR(&ipInfo.ip));
    ESP_LOGI(TAG, "GW: " IPSTR, IP2STR(&ipInfo.gw));
    ESP_LOGI(TAG, "Mask: " IPSTR, IP2STR(&ipInfo.netmask));

    wifi_config_t wifi_config = {
        .ap = {
            {.ssid = ""},
            {.password = ""},
            .ssid_len = strlen(""),
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .max_connection = 5
        }
    };



    setWifiCredintials(wifi_config.ap.ssid , wifi_config.ap.password , ESP_WIFI_SSID , ESP_WIFI_PASS);
    wifi_config.ap.ssid_len = strlen(ESP_WIFI_SSID);


    if (strlen(ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s", EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    apModeOn = true;
    isWifiOn = true;

    Serial.println("AP mode started");
}

void WiFiManager::startStationMode(const char* ssid, const char* password){
    if(!apModeOn && isWifiOn){
        ESP_LOGI(TAG, "Station mode already on");
        return;
    }

    if (isWifiOn){
        // ESP_ERROR_CHECK(esp_wifi_disconnect());
        ESP_ERROR_CHECK(esp_wifi_stop());
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    }

    Serial.print("try to connect to ");
    Serial.println(ssid);
    apModeOn = false;
    isWifiOn = true;


    s_wifi_event_group = xEventGroupCreate();
    wifi_config_t wifi_config = {
        .sta = {
            {.ssid = ""},
            {.password = ""},
            .threshold{ .authmode = WIFI_AUTH_WPA2_PSK} ,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };

    setWifiCredintials(wifi_config.ap.ssid , wifi_config.ap.password , ssid , password);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE,
            pdFALSE, portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", *ssid, *password);
        Serial.println("Station mode started");
        storeWifiCredentialsToStorage(ssid, password);
    }
    else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", *ssid, *password);
        Serial.println("Failed to connect to SSID");
        Serial.println("Back to AP mode");
        retryToConnectSta = 0;
        WiFiManager::startAPMode();
    }
    else {
        Serial.println("UNEXPECTED EVENT");
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}


