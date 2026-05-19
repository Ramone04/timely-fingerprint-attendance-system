#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "display_manager.h"
#include "mqtt_manager.h"
#include "http_manager.h"
#include "fingerprint_manager.h"
#include "ota_manager.h"
#include "user_storage.h"

// Pending data collected via MQTT (ID and name arrive on separate topics).
static uint16_t pendingUserID = 0;
static char pendingNome[64] = "";

// Flags to track which parts of the payload were received.
static bool gotUserID = false;
static bool gotNome = false;
// High-level actions waiting to be processed.
static bool enrollPending = false;
static bool deletePending = false; // Delete request for a specific ID.

// MQTT callback declaration.
void onMqttMessage(char *topic, byte *payload, unsigned int length);

// Reset state so the device waits for the next user request.
void resetState()
{
    // Clear all pending flags and cached data.
    deletePending = false;
    enrollPending = false;
    gotUserID = false;
    gotNome = false;
    pendingUserID = 0;
    memset(pendingNome, 0, sizeof(pendingNome));

    // Notify the operator on serial and LCD.
    Serial.println("Pronto — a aguardar dados do servidor...\n");
    LCDMessage("A aguardar dados", "do servidor...");
}

// Enrollment/delete state machine driven by MQTT requests and timed LCD prompts.
enum EnrollState
{
    IDLE,                 // waiting for MQTT data
    DATA_RECEIVED,        // has data, waiting to start
    ENROLLING,            // enrolling on the sensor (blocking by nature)
    SHOWING_RESULT,       // shows enroll result on the LCD for a fixed time
    DELETING,             // deleting a specific slot
    SHOWING_DELETE_RESULT // shows delete result on the LCD for a fixed time
};

static EnrollState state = IDLE;
static unsigned long stateChangedAt = 0;
// Durations for showing results on the LCD before resetting to idle.
static const unsigned long RESULT_DURATION = 3000;
static const unsigned long DELETE_RESULT_DURATION = 4000;
static const unsigned long START_DELAY = 2000;
static int lastResult = 0;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    // Initial banner with firmware info.
    Serial.println();
    Serial.println("==========================================");
    Serial.printf("Timely Fingerprint System — v%s\n", FIRMWARE_VERSION);
    Serial.printf("Build: %s\n", FIRMWARE_BUILD_DATE);
    Serial.println("==========================================");

    // Hardware and network bring-up.
    initLCD();

    // Show firmware version on LCD at startup for quick reference (e.g. during OTA testing).
    char line2[17];
    snprintf(line2, sizeof(line2), "v%s", FIRMWARE_VERSION);
    LCDMessage("Timely System", line2);
    delay(2000);

    connectWiFi();

    // OTA must be initialized before MQTT so updates work even with Wi-Fi instability.
    initOTA();
    LCDMessage("WiFi ligado", "OTA pronto");

    // Messaging and sensor setup.
    mqttSetup(onMqttMessage);
    initSensor();
    resetState();
}

// MQTT callback implementation; updates shared state for the main loop.
void onMqttMessage(char *topic, byte *payload, unsigned int length)
{
    // Copy the MQTT payload to a local buffer and ensure null-termination.
    char buf[128] = {};
    memcpy(buf, payload, min(length, (unsigned int)127));

    // Topic: enroll user ID
    if (strcmp(topic, TOPIC_ENROLL_USERID) == 0)
    {
        pendingUserID = (uint16_t)atoi(buf);
        gotUserID = true;
        Serial.printf("UserID recebido: %d\n", pendingUserID);
    }
    // Topic: enroll user name
    else if (strcmp(topic, TOPIC_ENROLL_NOME) == 0)
    {
        strncpy(pendingNome, buf, sizeof(pendingNome) - 1);
        pendingNome[sizeof(pendingNome) - 1] = '\0';
        gotNome = true;
        Serial.printf("Nome recebido: %s\n", pendingNome);
    }
    // Topic: delete request (name not required)
    else if (strcmp(topic, TOPIC_DELETE_USERID) == 0)
    {
        pendingUserID = (uint16_t)atoi(buf);
        deletePending = true;

        Serial.printf("Delete pedido para ID: %d\n", pendingUserID);
        char line2[17];
        snprintf(line2, sizeof(line2), "ID: %u", pendingUserID);
        LCDMessage("Delete pedido", line2);
    }

    // When both ID and name are present, we can start the enroll flow.
    if (gotUserID && gotNome)
    {
        enrollPending = true;
        Serial.println("Dados completos — pronto para enroll");
        char line1[17];
        char line2[17];
        snprintf(line1, sizeof(line1), "Nome: %s", pendingNome);
        snprintf(line2, sizeof(line2), "ID: %u", pendingUserID);
        LCDMessage(line1, line2);
    }
}

void loop()
{
    // Keep connectivity services alive before processing the state machine.
    connectWiFi();
    handleOTA();
    mqttLoop();
    updateLCD();

    switch (state)
    {
    case IDLE:
    {
        // Check if a new request arrived. Delete takes priority if both are set.
        if (deletePending)
        {
            state = DELETING;
            stateChangedAt = millis();
        }
        else if (enrollPending)
        {
            Serial.printf("A registar: ID=%u Nome=%s\n",
                          pendingUserID, pendingNome);
            LCDMessage("A iniciar registo", pendingNome);
            state = DATA_RECEIVED;
            stateChangedAt = millis();
        }
        break;
    }

    case DATA_RECEIVED:
    {
        // Wait START_DELAY before starting enrollment so the user sees the prompt.
        if (millis() - stateChangedAt >= START_DELAY)
        {
            state = ENROLLING;
        }
        break;
    }

    case ENROLLING:
    {
        // enrollFinger() blocks, but it calls mqttLoop() and handleOTA()
        // internally while waiting for finger placement.
        lastResult = enrollFinger(pendingUserID);

        if (lastResult == 1)
        {
            Serial.println("Enroll bem-sucedido!");
            LCDMessage("Enroll", "bem-sucedido!");
            saveUser(pendingUserID, pendingNome);
            sendEnrollStatus(pendingUserID, 1);
        }
        else
        {
            Serial.println("Enroll falhou.");
            LCDMessage("Enroll", "falhou.");
            sendEnrollStatus(pendingUserID, 0);
        }

        state = SHOWING_RESULT;
        stateChangedAt = millis();
        break;
    }

    case SHOWING_RESULT:
    {
        if (millis() - stateChangedAt >= RESULT_DURATION)
        {
            resetState();
            state = IDLE;
        }
        break;
    }

    case DELETING:
    {
        Serial.printf("A apagar ID=%u\n", pendingUserID);
        LCDMessage("A apagar...", "");

        // deleteFinger() returns 1 on success; keep storage and server in sync.
        int result = deleteFinger(pendingUserID);

        if (result == 1)
        {
            Serial.println("Delete bem-sucedido!");
            deleteUser(pendingUserID);

            char line2[17];
            snprintf(line2, sizeof(line2), "Slot: %u", pendingUserID);
            LCDMessage("Apagado!", line2);
            sendDeleteStatus(pendingUserID, 1);
        }
        else
        {
            Serial.println("Delete falhou.");
            LCDMessage("Delete falhou", "");
            sendDeleteStatus(pendingUserID, 0);
        }

        state = SHOWING_DELETE_RESULT;
        stateChangedAt = millis();
        break;
    }

    case SHOWING_DELETE_RESULT:
    {
        if (millis() - stateChangedAt >= DELETE_RESULT_DURATION)
        {
            resetState();
            state = IDLE;
        }
        break;
    }
    }
}