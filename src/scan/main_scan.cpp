#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "display_manager.h"
#include "http_manager.h"
#include "fingerprint_manager.h"
#include "ota_manager.h"
#include "user_storage.h"

// Returns:
enum ScanState
{
    IDLE,
    SHOWING_RESULT,
    WAITING_LIFT
};
static ScanState state = IDLE;
static unsigned long stateChangedAt = 0;
static const unsigned long RESULT_DURATION = 3000;

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

    initLCD();

    // Show firmware version on LCD at startup for quick reference (e.g. during OTA testing).
    char line2[17];
    snprintf(line2, sizeof(line2), "v%s", FIRMWARE_VERSION);
    LCDMessage("Timely System", line2);
    delay(2000);

    connectWiFi();
    initOTA();
    if (!initSensor())
    {
        LCDMessage("Sensor erro!", "Verifica wiring");
        while (1)
            ;
    }
    LCDMessage("Coloca o dedo", "no sensor");
}

void loop()
{
    handleOTA();
    connectWiFi();
    updateLCD();

    switch (state)
    {
    case IDLE:
    {
        int result = scanFinger();

        if (result >= 0)
        {
            // Feedback imediato — antes do HTTP
            LCDMessage("A ler...", "Aguarde");
            Serial.printf("Match! Slot #%d\n", result);

            char userName[32];
            loadUser(result, userName, sizeof(userName));

            if (sendPonto(result))
            {
                LCDMessage("Ponto registado!", userName);
            }
            else
            {
                LCDMessage("After hours", userName);
            }

            // Teste HTTP sem retry automático — o resultado é mostrado mesmo que a ligação falhe, para feedback imediato ao utilizador
            //  LCDMessage("Ponto registado!", userName);
            //  sendPonto(result);

            stateChangedAt = millis();
            state = SHOWING_RESULT;
        }
        else if (result == -3)
        {
            LCDMessage("Nao reconhecido", "Tenta novamente");
            stateChangedAt = millis();
            state = SHOWING_RESULT;
        }
        // -1 e -2 → ignorados, loop continua
        break;
    }

    case SHOWING_RESULT:
    {
        if (millis() - stateChangedAt >= RESULT_DURATION)
        {
            state = WAITING_LIFT;
        }
        break;
    }

    case WAITING_LIFT:
    {
        if (scanFinger() == -1)
        {
            LCDMessage("Coloca o dedo", "no sensor");
            state = IDLE;
        }
        break;
    }
    }
}