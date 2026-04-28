#pragma once

#include <Arduino.h>

// Envia para o backend o resultado do enroll: status 1=sucesso, 0=falha.
// Retorna true apenas se a API responder com HTTP 2xx.
bool sendEnrollStatus(uint16_t userId, uint8_t status);

// Envia para o backend o resultado do scan: fingerprint_id e confiança.
// Retorna true apenas se a API responder com HTTP 2xx.
bool sendScanResult(uint16_t fingerprintId, uint16_t confidence);
