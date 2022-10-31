#include "webServer.h"
#include <Arduino.h>
#include <ArduinoJson.h>

static httpd_handle_t server = NULL;
static OnCredintialsSet onCredintialsSet = NULL;

/* An HTTP GET handler */
static esp_err_t checkConnectedGetHandler(httpd_req_t* req){

    Serial.println("Esp connected to wifi successfully");
    httpd_resp_send(req, "Connected", HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

static const httpd_uri_t checkConnected = {
    .uri       = "/check_connected",
    .method    = HTTP_GET,
    .handler   = checkConnectedGetHandler,
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

    onCredintialsSet(ssid, password);

    return ESP_OK;
}

static const httpd_uri_t wifiCredintials = {
    .uri       = "/connect",
    .method    = HTTP_POST,
    .handler   = wifiCredintialsPostHandler,
    .user_ctx  = NULL
};



void WebServer::startWebServer(OnCredintialsSet callback){

    onCredintialsSet = callback;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    Serial.print("Starting server on port:");
    Serial.println(config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        httpd_register_uri_handler(server, &checkConnected);
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