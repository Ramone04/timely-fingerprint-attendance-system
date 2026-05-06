#pragma once

#include <Arduino.h>

// Post enroll result to the backend: status 1=success, 0=failure.
// Returns true only when the API responds with HTTP 2xx.
bool sendEnrollStatus(uint16_t userId, uint8_t status);

// Post delete result to the backend: status 1=success, 0=failure.
// Returns true only when the API responds with HTTP 2xx.
bool sendDeleteStatus(uint16_t userId, uint8_t status);
