// OTA manager: configures ArduinoOTA callbacks and polling.
#include "ota_manager.h"
#include "config.h"
#include <ArduinoOTA.h>
#include "display_manager.h"

// Track last reported percentage to limit LCD updates
static uint8_t lastPct = 0;

// Configure OTA callbacks and start the OTA service.
void initOTA() {
    // Hostname used to identify the ESP32 on the network
    ArduinoOTA.setHostname(OTA_HOSTNAME);

    // Password required for OTA uploads on the local network
    ArduinoOTA.setPassword(OTA_PASSWORD);

    // Called once when OTA begins.
    ArduinoOTA.onStart([]() {
        lastPct = 0;
        String type = ArduinoOTA.getCommand() == U_FLASH ? "firmware" : "filesystem";
        // Report the type of update and show a starting message
        Serial.printf("[OTA] A iniciar update: %s\n", type.c_str());
        LCDMessage("OTA Update", "A iniciar...");
    });

    // Called when the OTA upload finishes successfully.
    ArduinoOTA.onEnd([]() {
        // Inform the user that the device will reboot after OTA
        Serial.println("[OTA] Concluído — a reiniciar");
        LCDMessage("OTA Concluido", "A reiniciar...");
    });

    // Called repeatedly with transfer progress.
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        uint8_t pct = 0;
        // Protect against division by zero and clamp to 100.
        if (total > 0) {
            pct = static_cast<uint8_t>((progress * 100U) / total);
            if (pct > 100) {
                pct = 100;
            }
        }
        Serial.printf("[OTA] Progresso: %u%%\r", pct);
        // Update the LCD every 10% to avoid overloading I2C
        if (pct >= lastPct + 10 || pct == 100) {
            char line2[17];
            snprintf(line2, sizeof(line2), "   %u%%   ", pct);
            LCDMessage("OTA Update", line2);
            lastPct = pct;
        }
    });

    // Called when the OTA process fails.
    ArduinoOTA.onError([](ota_error_t error) {
        // Map OTA errors to a short LCD message
        Serial.printf("[OTA] Erro: %u\n", error);
        LCDMessage("OTA Erro", error == OTA_AUTH_ERROR    ? "Auth falhou"  :
                               error == OTA_BEGIN_ERROR   ? "Begin falhou" :
                               error == OTA_CONNECT_ERROR ? "Conn falhou"  :
                               error == OTA_RECEIVE_ERROR ? "Recv falhou"  :
                                                            "End falhou");
    });

    // Start OTA service after callbacks are registered.
    ArduinoOTA.begin();
    // Ready to receive OTA updates
    Serial.printf("[OTA] Pronto — hostname: %s\n", OTA_HOSTNAME);
}

// Poll ArduinoOTA; call frequently from the main loop.
void handleOTA() {
    // Pump OTA events from the main loop
    ArduinoOTA.handle();
}