#pragma once

#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>

#include "esp_tls_crypto.h"
#include <esp_http_server.h>

namespace WebServer {


    void registerHandlers();

    void startWebServer();

    esp_err_t stopWebServer();
}