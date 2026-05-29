// Fingerprint manager: initializes the sensor, runs enroll flow, and deletes templates.
// Keeps MQTT and OTA alive during sensor waits to avoid timeouts.
#include "fingerprint_manager.h"
#include "display_manager.h"
#include "mqtt_manager.h"
#include "config.h"
#include <Adafruit_Fingerprint.h>
#include "ota_manager.h"

// Default confidence thresholds for identifying low-confidence matches.
// Adjust these based on your sensor's performance and the specific users.
#ifndef LOW_CONFIDENCE_USERS
#define LOW_CONFIDENCE_USERS {14}
#endif

#ifndef NORMAL_CONFIDENCE_THRESHOLD
#define NORMAL_CONFIDENCE_THRESHOLD 50
#endif

#ifndef LOW_CONFIDENCE_THRESHOLD
#define LOW_CONFIDENCE_THRESHOLD 20
#endif

// Dedicated UART and Adafruit driver instance
static HardwareSerial fpSerial(2);
static Adafruit_Fingerprint finger(&fpSerial);

// Initialize UART and verify the sensor responds.
// Returns true when the sensor passes the password check.
bool initSensor()
{
    // Open UART and initialize the sensor driver
    fpSerial.begin(FP_BAUD, SERIAL_8N1, FP_RX_PIN, FP_TX_PIN);
    finger.begin(FP_BAUD);
    delay(1000); // Allow sensor boot time

    // verifyPassword() confirms communication with the sensor
    if (finger.verifyPassword())
    {
        LCDMessage("Sensor OK", "");
        return true;
    }
    else
    {
        LCDMessage("Sensor Error", "Check wiring");
        return false;
    }
}

// Wait for a finger to be placed and capture the image.
// Returns FINGERPRINT_OK or a sensor error code.
// Polls the sensor while keeping MQTT/OTA handlers alive.
static uint8_t waitForFinger()
{
    uint8_t result;
    do
    {
        // Poll the sensor and yield to networking tasks.
        result = finger.getImage();
        mqttLoop();  // Keep MQTT session alive while waiting
        handleOTA(); // Process OTA events during long waits
        delay(50);
    } while (result == FINGERPRINT_NOFINGER);
    return result;
}

// Wait until the finger is removed from the sensor.
// Polls the sensor while keeping MQTT/OTA handlers alive.
static void waitForLift()
{

    while (finger.getImage() != FINGERPRINT_NOFINGER)
    {
        // Poll the sensor and yield to networking tasks.
        mqttLoop();  // Keep MQTT session alive while waiting
        handleOTA(); // Process OTA events during long waits
        delay(100);
    }
}

// Enroll a finger into the given slot.
// Returns 1 on success, 0 on failure.
// fingerprint_manager.cpp

int enrollFinger(uint16_t slotId)
{
    Serial.printf("Enroll no slot #%d\n", slotId);

    // -- Read 1 -----------------------------------------------
    LCDMessage("Coloca o dedo", "Leitura 1/2");

    if (waitForFinger() != FINGERPRINT_OK)
        return 0;
    if (finger.image2Tz(1) != FINGERPRINT_OK)
    {
        Serial.println("image2Tz(1) falhou");
        return 0;
    }

    // Immediately prompt to lift the finger
    LCDMessage("Levanta o dedo", "");
    waitForLift();
    delay(300); // Short pause to let the sensor stabilize

    // -- Read 2 -----------------------------------------------
    LCDMessage("Coloca o mesmo", "dedo.Leitura 2/2");

    if (waitForFinger() != FINGERPRINT_OK)
        return 0;
    if (finger.image2Tz(2) != FINGERPRINT_OK)
    {
        Serial.println("image2Tz(2) falhou");
        return 0;
    }

    // Ask to lift the finger BEFORE createModel
    LCDMessage("Levanta o dedo", "");
    waitForLift();

    // Now process the model
    LCDMessage("A processar...", "");
    if (finger.createModel() != FINGERPRINT_OK)
    {
        Serial.println("Leituras nao coincidem");
        LCDMessage("Leituras nao", "coincidem");
        return 0;
    }

    // -- Store in slot -----------------------------------------
    if (finger.storeModel(slotId) != FINGERPRINT_OK)
    {
        Serial.println("Erro ao guardar no slot");

        char line2[17];
        snprintf(line2, sizeof(line2), "Slot: %u", slotId);
        LCDMessage("Erro ao guardar", line2);
        delay(2000);
        return 0;
    }

    Serial.printf("Fingerprint guardada no slot #%u\n", slotId);

    char line2[17];
    snprintf(line2, sizeof(line2), "Slot: %u", slotId);
    LCDMessage("Dedo guardado", line2);
    delay(2000);
    return 1;
}

// Delete the template stored in the specified slot.
// Returns 1 on success, 0 on failure.
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

// -- Scan logic -----------------------------------------------

// Returns true if the slot is allowed to use the reduced confidence threshold.
static bool isLowConfidenceUser(uint16_t slotId)
{
    static const uint16_t lowConfSlots[] = LOW_CONFIDENCE_USERS;
    static const size_t count = sizeof(lowConfSlots) / sizeof(lowConfSlots[0]);

    for (size_t i = 0; i < count; i++)
    {
        if (lowConfSlots[i] == slotId)
            return true;
    }
    return false;
}

// 
int scanFinger() {
    if (finger.getImage() != FINGERPRINT_OK) return -1;
    if (finger.image2Tz() != FINGERPRINT_OK) return -2;
    if (finger.fingerSearch() != FINGERPRINT_OK) return -3;

    uint16_t slotId = finger.fingerID;
    uint16_t confidence = finger.confidence;

    // Pick the threshold based on whether this user is flagged as low-confidence.
    uint16_t threshold = isLowConfidenceUser(slotId)
        ? LOW_CONFIDENCE_THRESHOLD
        : NORMAL_CONFIDENCE_THRESHOLD;

    if (confidence < threshold) {
        Serial.printf("[FP] Match rejeitado: slot=%u confidence=%u threshold=%u\n",
                      slotId, confidence, threshold);
        return -3;
    }

    Serial.printf("[FP] Match aceite: slot=%u confidence=%u threshold=%u\n",
                  slotId, confidence, threshold);
    return slotId;
}

// -- Utility functions -----------------------------------------
uint16_t getTemplateCount()
{
    if (finger.getTemplateCount() != FINGERPRINT_OK)
        return 0;
    return finger.templateCount;
}

bool wipeAllFingerprints()
{
    return finger.emptyDatabase() == FINGERPRINT_OK;
}