#pragma once
#include <stdint.h>

bool initSensor();
// Retorna: true se o sensor foi encontrado

int enrollFinger(uint16_t slotId);
// Retorna: 1 se sucesso, 0 se falhou (mismatch ou erro)

int scanFinger(uint16_t* outConfidence);
// Retorna: fingerID (>=1) se encontrado, 0 se não reconhecido, -1 se sem dedo ou erro
// outConfidence: preenchido com a confiança da correspondência (se encontrado)