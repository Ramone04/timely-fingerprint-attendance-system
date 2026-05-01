#include "ota_manager.h"
#include "config.h"
#include <ArduinoOTA.h>
#include "display_manager.h"

static uint8_t lastPct = 0;

void initOTA() {
    // Hostname identifica o ESP32 na rede
    ArduinoOTA.setHostname(OTA_HOSTNAME);

    // Password — sem isto qualquer pessoa na rede pode fazer upload
    ArduinoOTA.setPassword(OTA_PASSWORD);

    ArduinoOTA.onStart([]() {
        lastPct = 0;
        String type = ArduinoOTA.getCommand() == U_FLASH ? "firmware" : "filesystem";
        Serial.printf("[OTA] A iniciar update: %s\n", type.c_str());
        LCDMessage("OTA Update", "A iniciar...");
    });

    ArduinoOTA.onEnd([]() {
        Serial.println("[OTA] Concluído — a reiniciar");
        LCDMessage("OTA Concluido", "A reiniciar...");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        uint8_t pct = 0;
        if (total > 0) {
            pct = static_cast<uint8_t>((progress * 100U) / total);
            if (pct > 100) {
                pct = 100;
            }
        }
        Serial.printf("[OTA] Progresso: %u%%\r", pct);
        // Actualiza o LCD a cada 10% para não sobrecarregar o I2C
        if (pct >= lastPct + 10 || pct == 100) {
            char line2[17];
            snprintf(line2, sizeof(line2), "   %u%%   ", pct);
            LCDMessage("OTA Update", line2);
            lastPct = pct;
        }
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("[OTA] Erro: %u\n", error);
        LCDMessage("OTA Erro", error == OTA_AUTH_ERROR    ? "Auth falhou"  :
                               error == OTA_BEGIN_ERROR   ? "Begin falhou" :
                               error == OTA_CONNECT_ERROR ? "Conn falhou"  :
                               error == OTA_RECEIVE_ERROR ? "Recv falhou"  :
                                                            "End falhou");
    });

    ArduinoOTA.begin();
    Serial.printf("[OTA] Pronto — hostname: %s\n", OTA_HOSTNAME);
}

void handleOTA() {
    ArduinoOTA.handle();
}