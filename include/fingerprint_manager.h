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

// Returns the number of fingerprint templates currently stored on the sensor.
// Returns 0 if the sensor doesn't respond.
uint16_t getTemplateCount();

// Wipes ALL fingerprint templates from the sensor — irreversible.
// Returns true on success, false otherwise.
bool wipeAllFingerprints();