#pragma once

#include "nvs_flash.h"
#include <string>

#define STORAGE_PARTITION "storage"


namespace NvsManager{
    void initNvsMemory();
    nvs_handle_t openStorage();
    std::string getStringVal(nvs_handle_t nvsHandle , const char* key);
    void storeString(nvs_handle_t nvsHandle , const char* key ,  const char* value);
    uint8_t getU8(nvs_handle_t nvsHandle , const char* key);
    void storeU8(nvs_handle_t nvsHandle , const char* key ,  uint8_t value);
    void closeStorage(nvs_handle_t nvsHandle);
}