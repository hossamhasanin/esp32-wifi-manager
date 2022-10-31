#include "webServer.h"
#include "wifiManager.h"
#include <Arduino.h>
#include <ArduinoJson.h>

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

    return ESP_OK;
}

static const httpd_uri_t auth = {
    .uri       = "/auth",
    .method    = HTTP_GET,
    .handler   = authGetHandler,
    .user_ctx  = NULL
};


static esp_err_t wifiCredintialsPostHandler(httpd_req_t *req)
{
    /* Destination buffer for content of HTTP POST request.
     * httpd_req_recv() accepts char* only, but content could
     * as well be any binary data (needs type casting).
     * In case of string data, null termination will be absent, and
     * content length would give length of string */
    char content[100];

    /* Truncate if content length larger than the buffer */
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {  /* 0 return value indicates connection closed */
        /* Check if timeout occurred */
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            /* In case of timeout one can choose to retry calling
             * httpd_req_recv(), but to keep it simple, here we
             * respond with an HTTP 408 (Request Timeout) error */
            httpd_resp_send_408(req);
        }
        /* In case of error, returning ESP_FAIL will
         * ensure that the underlying socket is closed */
        return ESP_FAIL;
    }


    Serial.println("recieved data");
    Serial.println(content);

    // Parse JSON object
    StaticJsonDocument<32> doc;
    DeserializationError error = deserializeJson(doc, content);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return ESP_FAIL;
    }
    const char* ssid = doc["ssid"];
    const char* password = doc["password"];

    Serial.print("wifiCredintialsPostHandler ");
    Serial.print("ssid: ");
    Serial.println(ssid);
    Serial.print("password: ");
    Serial.println(password);

    WiFiManager::startStationMode(ssid, password);

    return ESP_OK;
}

static const httpd_uri_t wifiCredintials = {
    .uri       = "/connect",
    .method    = HTTP_POST,
    .handler   = wifiCredintialsPostHandler,
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
        httpd_register_uri_handler(server, &auth);
        httpd_register_uri_handler(server, &wifiCredintials);
        Serial.println("Registered URI handlers");
    } else {
            ESP_LOGI(TAG, "Error starting server!");
            Serial.println("Error starting server!");
    }

}

esp_err_t WebServer::stopWebServer(){
    return httpd_stop(server);
}