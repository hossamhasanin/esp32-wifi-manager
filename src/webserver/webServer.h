#pragma once

#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>

#include "esp_tls_crypto.h"
#include <esp_http_server.h>

/* An HTTP GET handler */
// static esp_err_t authGetHandler(httpd_req_t *req)
// {

//     char*  buf;
//     size_t buf_len;
//     /* Read URL query string length and allocate memory for length + 1,
//      * extra byte for null termination */
//     buf_len = httpd_req_get_url_query_len(req) + 1;
//     if (buf_len > 1) {
//         buf = (char *) malloc(buf_len);
//         if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
//             ESP_LOGI(TAG, "Found URL query => %s", buf);
//             char ssid[32];
//             char password[32];
//             /* Get value of expected key from query string */
//             if (httpd_query_key_value(buf, "ssid", ssid, sizeof(ssid)) == ESP_OK) {
//                 ESP_LOGI(TAG, "Found URL query parameter => ssid=%s", ssid);
//             } else {
//                 ESP_LOGI(TAG, "Failed to find URL query parameter => ssid");
//                 return ESP_FAIL;
//             }

//             if (httpd_query_key_value(buf, "password", password, sizeof(password)) == ESP_OK) {
//                 ESP_LOGI(TAG, "Found URL query parameter => password=%s", password);
//             } else {
//                 ESP_LOGI(TAG, "Failed to find URL query parameter => password");
//                 return ESP_FAIL;
//             }
//             httpd_resp_send(req, "Done", HTTPD_RESP_USE_STRLEN);
//             // vTaskDelay(1000 / portTICK_PERIOD_MS);
//             Serial.print("authGetHandler ");
//             Serial.print("ssid: ");
//             Serial.println(ssid);
//             Serial.print("password: ");
//             Serial.println(password);
//             // WiFiManager::startStationMode(ssid, password);
//         }
//         free(buf);
//     }

//     return ESP_OK;
// }

// static const httpd_uri_t auth = {
//     .uri       = "/auth",
//     .method    = HTTP_GET,
//     .handler   = authGetHandler,
//     .user_ctx  = NULL
// };

// create a pointer to a function that takes two const char* and returns void
typedef void (*OnCredintialsSet)(const char*, const char*);



namespace WebServer {


    void registerHandlers();

    void startWebServer(OnCredintialsSet callback);

    esp_err_t stopWebServer();
}