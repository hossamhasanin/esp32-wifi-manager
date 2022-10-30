#include "webServer.h"
#include "wifiManager.h"
#include <Arduino.h>

static httpd_handle_t server = NULL;


/* An HTTP GET handler */
static esp_err_t authGetHandler(httpd_req_t *req)
{

    char*  buf;
    size_t buf_len;
    /* Read URL query string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = (char *) malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            char ssid[32];
            char password[32];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "ssid", ssid, sizeof(ssid)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => ssid=%s", ssid);
            } else {
                ESP_LOGI(TAG, "Failed to find URL query parameter => ssid");
                return ESP_FAIL;
            }

            if (httpd_query_key_value(buf, "password", password, sizeof(password)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => password=%s", password);
            } else {
                ESP_LOGI(TAG, "Failed to find URL query parameter => password");
                return ESP_FAIL;
            }
            httpd_resp_send(req, "Done", HTTPD_RESP_USE_STRLEN);
            // vTaskDelay(1000 / portTICK_PERIOD_MS);
            Serial.print("authGetHandler ");
            Serial.print("ssid: ");
            Serial.println(ssid);
            Serial.print("password: ");
            Serial.println(password);
            WiFiManager::startStationMode(ssid, password);
        }
        free(buf);
    }

    // /* Set some custom headers */
    // httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
    // httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");

    /* Send response with custom headers and body set as the
     * string passed in user context*/
    // const char* resp_str = (const char*) req->user_ctx;

    return ESP_OK;
}

static const httpd_uri_t auth = {
    .uri       = "/auth",
    .method    = HTTP_GET,
    .handler   = authGetHandler,
    .user_ctx  = NULL
};

void WebServer::startWebServer(){
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    Serial.print("Starting server on port:");
    Serial.println(config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &auth);
        Serial.println("Registering URI handlers");
    } else {
            ESP_LOGI(TAG, "Error starting server!");
            Serial.println("Error starting server!");
    }

}

esp_err_t WebServer::stopWebServer(){
    return httpd_stop(server);
}