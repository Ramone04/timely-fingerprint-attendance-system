#pragma once
#include <stdint.h>

bool initSensor();
// Retorna: true se o sensor foi encontrado

int enrollFinger(uint16_t slotId);
// Retorna: 1 se sucesso, 0 se falhou (mismatch ou erro)