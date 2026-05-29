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
            LCDMessage("A ler...", "Aguarde");
            Serial.printf("Match! Slot #%d\n", result);

            // Nome local do NVS como fallback caso o servidor não devolva nome
            char localName[32];
            loadUser(result, localName, sizeof(localName));

            PontoEventInfo info = {};
            PontoResult res = sendPontoWithRetry(result, info);

            // Preferir nome do servidor; cair para o local se vazio
            const char *displayName = (info.userName[0] != '\0') ? info.userName : localName;

            switch (res)
            {
            case PONTO_OK:
            {
                char line1[17];
                if (strcmp(info.eventType, "entry") == 0)
                {
                    snprintf(line1, sizeof(line1), "Ola, %s", displayName);
                    LCDMessage(line1, "Bom trabalho!");
                }
                else if (strcmp(info.eventType, "exit") == 0)
                {
                    snprintf(line1, sizeof(line1), "Adeus, %s", displayName);
                    LCDMessage(line1, "Ate amanha!");
                }
                else
                {
                    // Fallback se o servidor não devolveu event_type
                    LCDMessage("Ponto registado!", displayName);
                }
                break;
            }

            case PONTO_AFTER_HOURS:
            {
                char line1[17];
                snprintf(line1, sizeof(line1), "After hours");
                LCDMessage(line1, displayName);
                break;
            }

            case PONTO_FAILED:
                LCDMessage("Erro de ligacao", localName);
                break;
            }

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