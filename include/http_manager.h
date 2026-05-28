#pragma once

#include <Arduino.h>

enum PontoResult {
    PONTO_OK,         // 2xx — entrada ou saída registada
    PONTO_AFTER_HOURS, // 400 — terceira marcação rejeitada
    PONTO_FAILED      // qualquer outro erro (rede, servidor, etc.)
};

// Post enroll result to the backend: status 1=success, 0=failure.
// Returns true only when the API responds with HTTP 2xx.
bool sendEnrollStatus(uint16_t userId, uint8_t status);

// Post delete result to the backend: status 1=success, 0=failure.
// Returns true only when the API responds with HTTP 2xx.
bool sendDeleteStatus(uint16_t userId, uint8_t status);

// Post attendance point to the backend.
// Returns the result of the operation.
PontoResult sendPontoWithRetry(uint16_t userId);