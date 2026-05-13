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

// Declaração do callback do MQTT
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

void setup()
{
    Serial.begin(115200);
    delay(1000);

    // Hardware and network bring-up.
    initLCD();
    connectWiFi();

    // OTA must be initialized before MQTT so updates work even with Wi-Fi instability.
    initOTA();
    LCDMessage("WiFi ligado", "OTA pronto");

    // Messaging and sensor setup.
    mqttSetup(onMqttMessage);
    initSensor();
    resetState();
}

// Implementação do callback — com acesso às variáveis globais
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
        gotNome = true;
        Serial.printf("Nome recebido: %s\n", pendingNome);
    }
    // Topic: delete request (name not required)
    else if (strcmp(topic, TOPIC_DELETE_USERID) == 0)
    {
        pendingUserID = (uint16_t)atoi(buf);
        deletePending = true;

        Serial.printf("Delete pedido para ID: %d\n", pendingUserID);
        LCDMessage("Delete pedido", ("ID: " + String(pendingUserID)).c_str());
    }

    // When both ID and name are present, we can start the enroll flow.
    if (gotUserID && gotNome)
    {
        enrollPending = true;
        Serial.println("Dados completos — pronto para enroll");
        LCDMessage("Dados recebidos", "");
        LCDMessage(("Nome: " + String(pendingNome)).c_str(), ("ID: " + String(pendingUserID)).c_str());
    }
}

void loop()
{
    // Blocks here if Wi-Fi drops, and resumes only when connected again.
    connectWiFi();
    // OTA handler must run frequently to accept updates.
    handleOTA();
    mqttLoop();

    // Process delete requests immediately (no name required).
    if (deletePending)
    {
        Serial.printf("A apagar ID=%d\n", pendingUserID);
        LCDMessage("A apagar...", "");

        int result = deleteFinger(pendingUserID);

        // Report result locally and to the server.
        if (result == 1)
        {
            Serial.println("Delete bem-sucedido!");
            String slotMsg = "Slot: " + String(pendingUserID);
            deleteUser(pendingUserID); // Remove user data from storage
            LCDMessage("Apagado!", slotMsg.c_str());
            sendDeleteStatus(pendingUserID, 1);
        }
        else
        {
            Serial.println("Delete falhou.");
            LCDMessage("Delete falhou", "");
            sendDeleteStatus(pendingUserID, 0);
        }

        // Show result briefly, then reset to idle.
        delay(5000);
        resetState();
        return;
    }

    // No enroll work pending; stay idle.
    if (!enrollPending)
        return;

    // Data complete; start enroll flow.
    Serial.printf("A registar: ID=%d Nome=%s\n", pendingUserID, pendingNome);
    LCDMessage("A iniciar registo", pendingNome);
    delay(2000);

    // Capture fingerprint samples and build the template.
    int result = enrollFinger(pendingUserID);

    // Report result locally and to the server.
    if (result == 1)
    {
        Serial.println("Enroll bem-sucedido!");
        LCDMessage("Enroll", "bem-sucedido!");
        saveUser(pendingUserID, pendingNome);
        if (!sendEnrollStatus(pendingUserID, 1))
        {
            Serial.println("Falha ao enviar status HTTP (enroll sucesso) — sem retry automático.");
        }
    }
    else
    {
        Serial.println("Enroll falhou.");
        LCDMessage("Enroll", "falhou.");
        if (!sendEnrollStatus(pendingUserID, 0))
        {
            Serial.println("Falha ao enviar status HTTP (enroll falha) — sem retry automático.");
        }
    }

    // Keep the result on screen briefly.
    delay(3000);

    // Reset for the next user request.
    resetState();
}
