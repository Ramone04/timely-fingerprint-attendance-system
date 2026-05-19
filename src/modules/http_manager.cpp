// HTTP manager: posts enroll/delete status over TLS.
#include "http_manager.h"
#include "config.h"
#include "certificates.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

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

bool sendDeleteStatus(uint16_t userId, uint8_t status)
{
    char payload[64];
    snprintf(payload, sizeof(payload),
             "{\"user_id\":%u,\"status\":%u}", userId, status);
    return postJson(DELETE_STATUS_URL, payload);
}

bool sendPonto(uint16_t userId)
{
    char payload[64];
    snprintf(payload, sizeof(payload),
             "{\"user_id\":%u}", userId);
    return postJson(PONTO_URL, payload);
}