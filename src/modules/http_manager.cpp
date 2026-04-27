#include "http_manager.h"
#include "config.h"
#include "certificates.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

bool sendEnrollStatus(uint16_t userId, uint8_t status)
{
    WiFiClientSecure httpClient;
    HTTPClient http;

    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    httpClient.setCACert(ENROLL_API_CA_CERT);
    if (!http.begin(httpClient, ENROLL_STATUS_URL)) {
        httpClient.stop();
        return false;
    }

    http.addHeader("Content-Type", "application/json");
    // {"user_id":65535,"status":255}: 30 chars + '\0'
    char payload[64];
    snprintf(payload, sizeof(payload), "{\"user_id\":%u,\"status\":%u}", userId, status);

    int code = http.POST(payload);
    Serial.printf("Enroll status HTTP code: %d\n", code);
    http.end();
    httpClient.stop();
    return code >= 200 && code < 300;
}
