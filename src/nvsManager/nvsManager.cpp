#include "nvsManager.h"
#include <Arduino.h>



void NvsManager::initNvsMemory(){
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

}

nvs_handle_t NvsManager::openStorage(){
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open(STORAGE_PARTITION, NVS_READWRITE, &nvsHandle);
    if (err != ESP_OK) {
        Serial.println("Error (" + String(esp_err_to_name(err)) + ") opening NVS handle!");
        return ESP_FAIL;
    }
    return nvsHandle;
}

std::string NvsManager::getStringVal(nvs_handle_t nvsHandle , const char* key){
    size_t required_size;
    esp_err_t err = nvs_get_str(nvsHandle, key, NULL, &required_size);
    if (err != ESP_OK) {
        Serial.println("Error (" + String(esp_err_to_name(err)) + ") reading ssid!");
        return "";
    }

    char value[required_size];
    err = nvs_get_str(nvsHandle, key, value, &required_size);
    if (err != ESP_OK) {
        Serial.println("Error (" + String(esp_err_to_name(err)) + ") reading ssid!");
        return "";
    }
    
    return value;
}

uint8_t NvsManager::getU8(nvs_handle_t nvsHandle , const char* key){
    uint8_t value;
    esp_err_t err = nvs_get_u8(nvsHandle, key, &value);
    if (err != ESP_OK) {
        Serial.println("Error (" + String(esp_err_to_name(err)) + ") reading ssid!");
        return 0;
    }
    return value;
}

void NvsManager::storeU8(nvs_handle_t nvsHandle , const char* key ,  uint8_t value){
    esp_err_t err = nvs_set_u8(nvsHandle, key, value);
    if (err != ESP_OK) {
        Serial.println("Error (" + String(esp_err_to_name(err)) + ") storing ssid!");
    }
}


void NvsManager::storeString(nvs_handle_t nvsHandle , const char* key , const char* value){
    esp_err_t err = nvs_set_str(nvsHandle, key, value);
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

void NvsManager::closeStorage(nvs_handle_t nvsHandle){
    nvs_close(nvsHandle);
}