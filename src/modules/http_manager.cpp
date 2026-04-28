#include "http_manager.h"
#include "config.h"
#include "certificates.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// ← static — criado uma vez, vive durante toda a execução
// Evita fragmentação do heap do mbedTLS entre chamadas
static WiFiClientSecure httpClient;
static bool httpClientInitialized = false;

// Inicializa o cliente TLS uma única vez
static void initHttpClient() {
    if (httpClientInitialized) return;
    httpClient.setCACert(LARAVEL_CA_CERT);
    httpClientInitialized = true;
}

bool sendEnrollStatus(uint16_t userId, uint8_t status) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[HTTP] WiFi não ligado");
        return false;
    }

    initHttpClient();  // só faz algo na primeira chamada

    HTTPClient http;
    if (!http.begin(httpClient, ENROLL_STATUS_URL)) {
        Serial.println("[HTTP] Falha ao iniciar ligação");
        return false;
    }

    http.addHeader("Content-Type", "application/json");

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

    http.end();
    return code >= 200 && code < 300;
}