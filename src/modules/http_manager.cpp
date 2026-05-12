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

// File-local helper; not exposed in the header.
// Builds and sends a small JSON payload and returns true for 2xx responses.
static bool postStatus(const char *url, uint16_t userId, uint8_t status)
{
    // Abort early if there is no network connection.
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("[HTTP] WiFi não ligado");
        return false;
    }

    initHttpClient();

    // Use a short-lived HTTPClient for each POST
    HTTPClient http;
    if (!http.begin(httpClient, url))
    {
        Serial.println("[HTTP] Falha ao iniciar ligação");
        return false;
    }

    // JSON payload with user id and status code.
    http.addHeader("Content-Type", "application/json");

    char payload[64];
    snprintf(payload, sizeof(payload),
             "{\"user_id\":%u,\"status\":%u}", userId, status);

    Serial.printf("[HTTP] POST %s — Body: %s\n", url, payload);

    // Send request and capture the HTTP response code.
    int code = http.POST(payload);

    if (code > 0)
    {
        Serial.printf("[HTTP] Resposta: %d — %s\n", code, http.getString().c_str());
    }
    else
    {
        Serial.printf("[HTTP] Erro: %s\n", http.errorToString(code).c_str());
    }

    // Release resources (socket and buffers).
    http.end();
    return code >= 200 && code < 300; // Success for any 2xx response
}


static bool postPonto(const char *url, uint16_t userId)
{
    // Abort early if there is no network connection.
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("[HTTP] WiFi não ligado");
        return false;
    }

    initHttpClient();

    // Use a short-lived HTTPClient for each POST
    HTTPClient http;
    if (!http.begin(httpClient, url))
    {
        Serial.println("[HTTP] Falha ao iniciar ligação");
        return false;
    }

    // JSON payload with user id and status code.
    http.addHeader("Content-Type", "application/json");

    char payload[64];
    snprintf(payload, sizeof(payload),
             "{\"user_id\":%u}", userId);

    Serial.printf("[HTTP] POST %s — Body: %s\n", url, payload);

    // Send request and capture the HTTP response code.
    int code = http.POST(payload);

    if (code > 0)
    {
        Serial.printf("[HTTP] Resposta: %d — %s\n", code, http.getString().c_str());
    }
    else
    {
        Serial.printf("[HTTP] Erro: %s\n", http.errorToString(code).c_str());
    }

    // Release resources (socket and buffers).
    http.end();
    return code >= 200 && code < 300; // Success for any 2xx response
}


// Public API wrappers
// Post enroll status to the server.
bool sendEnrollStatus(uint16_t userId, uint8_t status)
{
    return postStatus(ENROLL_STATUS_URL, userId, status);
}

// Post delete status to the server.
bool sendDeleteStatus(uint16_t userId, uint8_t status)
{
    return postStatus(DELETE_STATUS_URL, userId, status);
}

bool sendPonto(uint16_t userId)
{
    return postPonto(PONTO_URL, userId); 
}   