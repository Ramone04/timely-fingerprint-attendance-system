#pragma once

#include <Arduino.h>

// Envia para o backend o resultado do enroll: status 1=sucesso, 0=falha.
// Retorna true apenas se a API responder com HTTP 2xx.
bool sendEnrollStatus(uint16_t userId, uint8_t status);

