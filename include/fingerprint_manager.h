#pragma once
#include <stdint.h>

// Fingerprint manager public API.
bool initSensor();
// Returns true when the sensor responds to the password check.

int enrollFinger(uint16_t slotId);
// Returns 1 on success, 0 on failure (mismatch or sensor error).
// slotId is the template slot index used by the sensor.

int deleteFinger(uint16_t slotId);
// Returns 1 on success, 0 on failure (empty slot or sensor error).

// Retorna: slotId (≥0) se match, -1 sem dedo, -2 imagem má, -3 sem match
int scanFinger();

// Performs dummy reads to stabilize sensor LEDs and auto-exposure.
// Called automatically by initSensor() — no need to call manually.
// Public only for advanced cases (e.g. recovery after error).
void warmUpSensor();

// Performs a dummy getImage() to keep the optical sensor active.
// Call periodically (~30s) when the system is idle to prevent
// the sensor from entering deep standby, which causes first-scan failures.
void keepSensorAlive();