#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "display_manager.h"
#include "mqtt_manager.h"
#include "fingerprint_manager.h"

// Variáveis globais para armazenar os dados pendentes do mqtt
static uint16_t pendingUserID = 0;
static char pendingNome[64] = "";

// Flags para controlar a chegada dos dados
static bool gotUserID     = false;
static bool gotNome       = false;
static bool enrollPending = false;

// Declaração do callback do MQTT
void onMqttMessage(char* topic, byte* payload, unsigned int length);

// ── Reset do estado para aguardar próximo utilizador ─────────
void resetState() {
    enrollPending = false;
    gotUserID     = false;
    gotNome       = false;
    pendingUserID = 0;
    memset(pendingNome, 0, sizeof(pendingNome));
    Serial.println("Pronto — a aguardar dados do servidor...\n");
    LCDMessage("A aguardar dados", "do servidor...");
}

void setup() {
    Serial.begin(115200);   
    delay(1000);
    
    initLCD();
    connectWiFi();

    mqttSetup(onMqttMessage);
    initSensor();
    resetState();
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
        LCDMessage("Dados recebidos", "");
        LCDMessage(("Nome: " + String(pendingNome)).c_str(), (  "ID: " + String(pendingUserID)).c_str());
    }
}

void loop()
{
    // Blocks here if Wi-Fi drops, and resumes only when connected again.
    connectWiFi();
    mqttLoop();

    if(!enrollPending) return;

    // ── Dados recebidos — iniciar enroll ─────────────────────
    Serial.printf("A registar: ID=%d Nome=%s\n", pendingUserID, pendingNome);
    LCDMessage("A iniciar registo", pendingNome);
    delay(1000);

    // Leituras do sensor e criação do modelo
    int result = enrollFinger(pendingUserID);

    // ── Resultado ────────────────────────────────────────────
    if (result == 1) {
        Serial.println("Enroll bem-sucedido!");
        LCDMessage("Enroll", "bem-sucedido!");
        if (!sendEnrollStatus(pendingUserID, 1)) {
            Serial.println("Falha ao enviar status HTTP de enroll (sucesso) — sem retry automático.");
        }
    } else {
        Serial.println("Enroll falhou.");
        LCDMessage("Enroll", "falhou.");
        if (!sendEnrollStatus(pendingUserID, 0)) {
            Serial.println("Falha ao enviar status HTTP de enroll (falha) — sem retry automático.");
        }
    }

    // Mantem a mensagem de resultado 3 segundos
    delay(3000);

    // ── Reset para próximo utilizador ─────────────────────────
    resetState();
}
