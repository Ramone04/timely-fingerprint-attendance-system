#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "display_manager.h"
#include "http_manager.h"
#include "fingerprint_manager.h"
#include "ota_manager.h"

void setup() {
    Serial.begin(115200);
    delay(1000);

    initLCD();
    connectWiFi();
    initOTA();
    if (!initSensor()) {
        LCDMessage("Sensor erro!", "Verifica wiring");
        while(1);
    }
    LCDMessage("Coloca o dedo", "no sensor");
    //LCDMessage("Teste de ", "upload");
}

void loop() {
    handleOTA();     
    connectWiFi();

    int result = scanFinger();

    if (result >= 0) {
        Serial.printf("Match! Slot #%d\n", result);
        char line2[17];
        snprintf(line2, sizeof(line2), "User ID: %d", result);

        if (sendPonto(result)) {
            Serial.println("Ponto registado com sucesso");
            LCDMessage("Ponto registado!", line2);
        } else {
            Serial.println("Falha ao registar ponto");
            LCDMessage("Erro ao registar", line2);
        }
        delay(3000);
        LCDMessage("Coloca o dedo", "no sensor");

    } else if (result == -3) {
        Serial.println("Dedo não reconhecido");
        LCDMessage("Nao reconhecido", "Tenta novamente");
        delay(2000);
        LCDMessage("Coloca o dedo", "no sensor");
    } 
}