// Test utility — wipes ALL fingerprint templates from the sensor.
// Run via env:test or env:test-cable.
// WARNING: starts wiping immediately after boot — there is no confirmation.
// After completion, OTA stays active so you can flash another firmware.

#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "display_manager.h"
#include "fingerprint_manager.h"
#include "ota_manager.h"
#include "user_storage.h"

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("==========================================");
    Serial.println("    FINGERPRINT WIPE UTILITY              ");
    Serial.println("==========================================");
    Serial.printf("Firmware: v%s\n", FIRMWARE_VERSION);

    // Hardware bring-up.
    initLCD();

    char line2[17];
    snprintf(line2, sizeof(line2), "v%s", FIRMWARE_VERSION);
    LCDMessage("Wipe Utility", line2);
    delay(1500);

    // Clear only the user Preferences namespace.
    Serial.println("Clearing user storage...");
    LCDMessage("Clearing users", "Please wait");
    if (!clearAllUsers()) {
        Serial.println("User storage clear failed.");
        LCDMessage("Users failed!", "Continuing");
        delay(1500);
    } else {
        Serial.println("User storage cleared.");
        LCDMessage("Users cleared", "Continuing");
        delay(1000);
    }

    // Network — needed so OTA remains accessible afterwards.
    connectWiFi();
    initOTA();

    // Sensor.
    if (!initSensor()) {
        LCDMessage("Sensor erro!", "Verifica wiring");
        while (true) {
            handleOTA();
            delay(50);
        }
    }

    // Report what we are about to wipe.
    uint16_t before = getTemplateCount();
    Serial.printf("Templates stored: %u\n", before);
    Serial.println("Wiping all templates...");
    LCDMessage("Wiping...", "Please wait");

    // The actual wipe — irreversible.
    if (wipeAllFingerprints()) {
        Serial.println("Done! All templates deleted.");
        LCDMessage("Done! Sensor", "is now clean");
    } else {
        Serial.println("Wipe failed! Try re-uploading.");
        LCDMessage("Wipe failed!", "Re-upload");
    }

    Serial.println();
    Serial.println("==========================================");
    Serial.println("Utility finished. OTA still available for");
    Serial.println("flashing another firmware when ready.");
    Serial.println("==========================================");
}

void loop() {
    // Keep OTA and WiFi alive so you can flash another firmware
    // without needing to power-cycle or use cable.
    handleOTA();
    connectWiFi();
    updateLCD();
}