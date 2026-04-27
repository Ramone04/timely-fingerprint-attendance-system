#include "http_manager.h"
#include "config.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

bool sendEnrollStatus(uint16_t userId, uint8_t status) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[HTTP] WiFi não ligado — não é possível enviar");
        return false;
    }

    WiFiClientSecure httpClient;
    httpClient.setInsecure();  // temporário — substituir por setCACert quando tiveres o cert
    //httpClient.setCACert(LARAVEL_CA_CERT);
    HTTPClient http;
    if (!http.begin(httpClient, ENROLL_STATUS_URL)) {
        Serial.println("[HTTP] Falha ao iniciar ligação");
        return false;
    }

    http.addHeader("Content-Type", "application/json");

    // {"user_id":65535,"status":255} → 30 chars + '\0'
    char payload[64];
    snprintf(payload, sizeof(payload),
        "{\"user_id\":%u,\"status\":%u}", userId, status);

    Serial.printf("[HTTP] POST %s\n", ENROLL_STATUS_URL);
    Serial.printf("[HTTP] Body: %s\n", payload);

    int code = http.POST(payload);

    if (code > 0) {
        Serial.printf("[HTTP] Resposta: %d\n", code);
        Serial.printf("[HTTP] Body: %s\n", http.getString().c_str());
    } else {
        Serial.printf("[HTTP] Erro: %s\n", http.errorToString(code).c_str());
    }

    http.end();  // único close necessário
    return code >= 200 && code < 300;
}
 