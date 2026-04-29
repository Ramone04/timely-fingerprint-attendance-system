#include "fingerprint_manager.h"
#include "display_manager.h"
#include "mqtt_manager.h"
#include "config.h"
#include <Adafruit_Fingerprint.h>

static HardwareSerial fpSerial(2);
static Adafruit_Fingerprint finger(&fpSerial);

bool initSensor()
{
    fpSerial.begin(FP_BAUD, SERIAL_8N1, FP_RX_PIN, FP_TX_PIN);
    finger.begin(FP_BAUD);
    delay(1000); // Aguarda o sensor inicializar

    if (finger.verifyPassword())
    {
        LCDMessage("Sensor OK", "");
        return true;
    }
    else
    {
        LCDMessage("Sensor Error", "Check connections");
        return false;
    }
}

// Aguarda que o dedo seja colocado e captura imagem
// Retorna FINGERPRINT_OK ou código de erro
static uint8_t waitForFinger()
{
    uint8_t result;
    do
    {
        result = finger.getImage();
        mqttLoop(); // para que a ligação MQTT não caia durante a espera
        delay(50);
    } while (result == FINGERPRINT_NOFINGER);
    return result;
}

// Aguarda que o dedo seja removido
static void waitForLift()
{
    while (finger.getImage() != FINGERPRINT_NOFINGER)
    {
        mqttLoop(); // para que a ligação MQTT não caia durante a espera
        delay(100);
    }
}

int enrollFinger(uint16_t slotId)
{
    Serial.printf("Enroll no slot #%d\n", slotId);

    // ── Leitura 1 ────────────────────────────────────────────
    LCDMessage("Coloca o dedo...", "");

    if (waitForFinger() != FINGERPRINT_OK)
        return 0;
    if (finger.image2Tz(1) != FINGERPRINT_OK)
        return 0;

    LCDMessage("Levanta o dedo", "");
    waitForLift();
    delay(500);

    // ── Leitura 2 ────────────────────────────────────────────
    LCDMessage("Coloca o mesmo dedo...", "");

    if (waitForFinger() != FINGERPRINT_OK)
        return 0;
    if (finger.image2Tz(2) != FINGERPRINT_OK)
        return 0;

    // Verifica match entre leitura 1 e 2
    if (finger.createModel() != FINGERPRINT_OK)
    {
        Serial.println("Leituras não coincidem");
        LCDMessage("Leituras não", "coincidem");
        return 0;
    }

    LCDMessage("Levanta o dedo", "");
    waitForLift();
    delay(500);

    // ── Guardar no slot ──────────────────────────────────────
    if (finger.storeModel(slotId) != FINGERPRINT_OK)
    {
        Serial.println("Erro ao guardar no slot");
        LCDMessage("Erro ao guardar", ("no slot: " + String(slotId)).c_str());
        delay(3000);
        return 0;
    }

    Serial.printf("Fingerprint guardada no slot #%d\n", slotId);
    LCDMessage("Fingerprint guardada", ("no slot: " + String(slotId)).c_str());
    delay(3000);
    return 1;
}

// Apaga a impressão digital do slot especificado
int deleteFinger(uint16_t slotId)
{
    uint8_t result = finger.deleteModel(slotId);
    if (result == FINGERPRINT_OK)
    {
        Serial.printf("Slot #%d apagado\n", slotId);
        return 1;
    }
    Serial.printf("Erro ao apagar slot #%d — code: %d\n", slotId, result);
    return 0;
}