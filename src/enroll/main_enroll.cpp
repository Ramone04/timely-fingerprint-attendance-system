#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "display_manager.h"
#include "mqtt_manager.h"

// Variáveis globais para armazenar os dados pendentes do mqtt
static uint16_t pendingUserID = 0;
static char pendingNome[64] = "";

// Flags para controlar a chegada dos dados
static bool gotUserID     = false;
static bool gotNome       = false;
static bool enrollPending = false;

// Declaração do callback do MQTT
void onMqttMessage(char* topic, byte* payload, unsigned int length);

void setup() {
    Serial.begin(115200);   
    delay(1000);

    initLCD();
    connectWiFi();
    mqttSetup(onMqttMessage);
}

// Implementação do callback — com acesso às variáveis globais
void onMqttMessage(char* topic, byte* payload, unsigned int length) {
    char buf[128] = {};
    memcpy(buf, payload, min(length, (unsigned int)127));

    if (strcmp(topic, TOPIC_ENROLL_USERID) == 0) {
        pendingUserID = (uint16_t)atoi(buf);
        gotUserID = true;
        Serial.printf("UserID recebido: %d\n", pendingUserID);
    }
    else if (strcmp(topic, TOPIC_ENROLL_NOME) == 0) {
        strncpy(pendingNome, buf, sizeof(pendingNome) - 1);
        gotNome = true;
        Serial.printf("Nome recebido: %s\n", pendingNome);
    }

    if (gotUserID && gotNome) {
        enrollPending = true;
        Serial.println("Dados completos — pronto para enroll");
        LCDMessage("Dados recebidos", "Pronto para enroll");
        delay(2000);
        LCDMessage(pendingNome, String(pendingUserID).c_str());
    }
}

void loop()
{
    // Blocks here if Wi-Fi drops, and resumes only when connected again.
    connectWiFi();
    mqttLoop();

}