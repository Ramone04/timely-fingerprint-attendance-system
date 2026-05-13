#pragma once

#include <stdint.h>
#include <stddef.h>

// Save the user name for the specified slot ID.
bool saveUser(uint16_t slotId, const char* name);

// Load the user name for the specified slot into the provided buffer.
bool loadUser(uint16_t slotId, char* nameUser, size_t maxlen);

// Deletes the user data for the specified slot.
bool deleteUser(uint16_t slotId);