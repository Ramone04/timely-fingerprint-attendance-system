#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "display_manager.h"
#include "http_manager.h"
#include "fingerprint_manager.h"

// Intervalo mínimo entre dois registos do mesmo dedo (ms)
static const uint32_t DEBOUNCE_MS = 3000;

static uint32_t lastScanTime = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);

    initLCD();
    connectWiFi();
    // MQTT não é necessário no modo scan — mqttLoop() dentro do fingerprint_manager
    // verifica o flag mqttInitialized e retorna imediatamente se não iniciado.
    initSensor();

    LCDMessage("Coloca o dedo", "para registar");
}

void loop() {
    // Mantém a ligação WiFi activa; reconecta se necessário.
    connectWiFi();

    uint16_t confidence = 0;
    int fingerprintId = scanFinger(&confidence);

    if (fingerprintId > 0) {
        // ── Correspondência encontrada ────────────────────────
        uint32_t now = millis();
        // Unsigned subtraction is rollover-safe on 32-bit types (~49-day wrap-around handled correctly)
        if (now - lastScanTime < DEBOUNCE_MS) {
            delay(200);
            return;
        }
        lastScanTime = now;

        Serial.printf("Match! ID #%d  confiança: %d\n", fingerprintId, confidence);
        LCDMessage("Dedo reconhecido!", ("ID: " + String(fingerprintId)).c_str());
        delay(500);

        LCDMessage("A registar...", "");

        if (sendScanResult((uint16_t)fingerprintId, confidence)) {
            Serial.println("Presença registada com sucesso.");
            LCDMessage("Presenca", "registada!");
        } else {
            Serial.println("Falha ao registar presença.");
            LCDMessage("Erro ao registar", "presenca");
        }

        delay(3000);
        LCDMessage("Coloca o dedo", "para registar");

    } else if (fingerprintId == 0) {
        // ── Dedo lido mas não reconhecido ────────────────────
        Serial.println("Dedo não reconhecido.");
        LCDMessage("Dedo nao", "reconhecido");
        delay(2000);
        LCDMessage("Coloca o dedo", "para registar");

    }
    // fingerprintId == -1: sem dedo ou erro de leitura — fica em idle
    delay(200);
}
