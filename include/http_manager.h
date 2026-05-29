#pragma once

#include <Arduino.h>

enum PontoResult {
    PONTO_OK,         // 2xx — entrada ou saída registada
    PONTO_AFTER_HOURS, // 400 — terceira marcação rejeitada
    PONTO_FAILED      // qualquer outro erro (rede, servidor, etc.)
};

// Event details returned by the server.
// Populated only when result is PONTO_OK or PONTO_AFTER_HOURS.
struct PontoEventInfo {
    char eventType[16];   // "entry", "exit" or "after_hours"
    char userName[32];    // name returned by the server (preferred over local NVS)
};

// Post enroll result to the backend: status 1=success, 0=failure.
// Returns true only when the API responds with HTTP 2xx.
bool sendEnrollStatus(uint16_t userId, uint8_t status);

// Post delete result to the backend: status 1=success, 0=failure.
// Returns true only when the API responds with HTTP 2xx.
bool sendDeleteStatus(uint16_t userId, uint8_t status);

// Posts attendance point and fills info with the server's response.
// info is always cleared, even on failure.
PontoResult sendPontoWithRetry(uint16_t userId, PontoEventInfo& info);