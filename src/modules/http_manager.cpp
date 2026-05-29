// HTTP manager: posts enroll/delete status over TLS.
#include "http_manager.h"
#include "config.h"
#include "certificates.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Static TLS client created once for the lifetime of the program.
// Reuse avoids mbedTLS heap fragmentation between requests.
static WiFiClientSecure httpClient;
static bool httpClientInitialized = false;

// Initialize the TLS client once and cache the CA certificate.
// Call before any HTTP requests to ensure TLS is ready.
static void initHttpClient()
{
    if (httpClientInitialized)
        return;
    httpClient.setCACert(LARAVEL_CA_CERT);
    httpClientInitialized = true;
}

// Reset the TLS client to recover from errors like heap fragmentation or connection issues.
static void resetHttpClient()
{
    httpClient.stop();             // fecha o socket atual
    httpClientInitialized = false; // força re-init no próximo uso
    Serial.println("[HTTP] Cliente TLS reiniciado");
}

// Helper function to send a JSON POST request and print the response.
static bool postJson(const char *url, const char *jsonPayload)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("[HTTP] WiFi nao ligado");
        return false;
    }

    initHttpClient();

    HTTPClient http;
    http.setReuse(true);

    if (!http.begin(httpClient, url))
    {
        Serial.println("[HTTP] Falha ao iniciar ligacao");
        return false;
    }

    http.addHeader("Content-Type", "application/json");
    http.addHeader("Connection", "keep-alive");

    Serial.printf("[HTTP] POST %s — Body: %s\n", url, jsonPayload);

    int code = http.POST(jsonPayload);

    if (code > 0)
    {
        Serial.printf("[HTTP] Resposta: %d — %s\n",
                      code, http.getString().c_str());
    }
    else
    {
        Serial.printf("[HTTP] Erro: %s\n", http.errorToString(code).c_str());
    }

    http.end();
    return code >= 200 && code < 300;
}

// ── Public API ─────────────────────────────────────────────
bool sendEnrollStatus(uint16_t userId, uint8_t status)
{
    char payload[64];
    snprintf(payload, sizeof(payload),
             "{\"user_id\":%u,\"status\":%u}", userId, status);
    return postJson(ENROLL_STATUS_URL, payload);
}

// Post delete result to the backend: status 1=success, 0=failure.
// Returns true only when the API responds with HTTP 2xx.
bool sendDeleteStatus(uint16_t userId, uint8_t status)
{
    char payload[64];
    snprintf(payload, sizeof(payload),
             "{\"user_id\":%u,\"status\":%u}", userId, status);
    return postJson(DELETE_STATUS_URL, payload);
}

// Parses the JSON response into the event info struct.
// Safe to call on empty/malformed responses — leaves info empty.
static void parseEventInfo(const String &response, PontoEventInfo &info)
{
    info.eventType[0] = '\0';
    info.userName[0] = '\0';

    if (response.length() == 0)
        return;

    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, response);
    if (err)
    {
        Serial.printf("[HTTP] JSON parse erro: %s\n", err.c_str());
        return;
    }

    const char *type = doc["event_type"] | "";
    const char *name = doc["user_name"] | "";

    strncpy(info.eventType, type, sizeof(info.eventType) - 1);
    info.eventType[sizeof(info.eventType) - 1] = '\0';

    strncpy(info.userName, name, sizeof(info.userName) - 1);
    info.userName[sizeof(info.userName) - 1] = '\0';
}

// Post attendance point to the backend.
static PontoResult sendPonto(uint16_t userId, PontoEventInfo& info) {
    info.eventType[0] = '\0';
    info.userName[0]  = '\0';

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[HTTP] WiFi nao ligado");
        return PONTO_FAILED;
    }

    initHttpClient();

    HTTPClient http;
    http.setReuse(true);

    if (!http.begin(httpClient, PONTO_URL)) {
        Serial.println("[HTTP] Falha ao iniciar ligacao");
        resetHttpClient();
        return PONTO_FAILED;
    }

    http.addHeader("Content-Type", "application/json");
    http.addHeader("Connection", "keep-alive");

    char payload[64];
    snprintf(payload, sizeof(payload), "{\"user_id\":%u}", userId);

    Serial.printf("[HTTP] POST %s — Body: %s\n", PONTO_URL, payload);

    int code = http.POST(payload);
    String response = (code > 0) ? http.getString() : "";

    Serial.printf("[HTTP] Resposta: %d — %s\n", code, response.c_str());

    http.end();

    if (code >= 200 && code < 300) {
        parseEventInfo(response, info);
        return PONTO_OK;
    }
    if (code == 400) {
        parseEventInfo(response, info);   // ★ também populamos no after hours
        return PONTO_AFTER_HOURS;
    }

    resetHttpClient();
    return PONTO_FAILED;
}

// Post attendance point to the backend.
// Returns the result of the operation, retrying once on failure.
PontoResult sendPontoWithRetry(uint16_t userId, PontoEventInfo& info) {
    for (uint8_t attempt = 1; attempt <= 2; attempt++) {
        PontoResult res = sendPonto(userId, info);

        if (res == PONTO_OK || res == PONTO_AFTER_HOURS) {
            return res;
        }

        Serial.printf("[HTTP] Tentativa %d falhou, a repetir...\n", attempt);
        delay(500);
    }
    return PONTO_FAILED;
}