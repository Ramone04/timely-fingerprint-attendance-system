#include "user_storage.h"
#include <Preferences.h>
#include <string.h>

Preferences prefs;

static void buildKey(uint16_t slotId, char *key, size_t keyLen)
{
    snprintf(key, keyLen, "u_%u", slotId);
}

bool saveUser(uint16_t slotId, const char *name)
{
    char key[16];
    buildKey(slotId, key, sizeof(key));

    prefs.begin("users", false);
    size_t result = prefs.putString(key, name);
    prefs.end();

    Serial.printf("[NVS] saveUser key='%s' name='%s' bytes=%d\n", key, name, result);
    return result > 0;
}

bool loadUser(uint16_t slotId, char *nameOut, size_t maxLen)
{
    char key[16];
    buildKey(slotId, key, sizeof(key));

    prefs.begin("users", true);
    bool exists = prefs.isKey(key);

    Serial.printf("[NVS] loadUser key='%s' exists=%d\n", key, exists);

    if (exists)
    {
        String value = prefs.getString(key);
        strncpy(nameOut, value.c_str(), maxLen - 1);
        Serial.printf("[NVS] loadUser value='%s'\n", value.c_str());
    }
    else
    {
        strncpy(nameOut, "Desconhecido", maxLen - 1);
    }
    nameOut[maxLen - 1] = '\0';
    prefs.end();
    return exists;
}

bool deleteUser(uint16_t slotId)
{
    // Construct the key for the given slot ID (e.g., "u_3" for slot 3)
    char key[16];
    buildKey(slotId, key, sizeof(key));

    // Remove the key-value pair from NVS
    prefs.begin("users", false);
    bool result = prefs.remove(key);
    prefs.end();

    Serial.printf("[NVS] deleteUser key='%s' result=%d\n", key, result);
    return result;
}
