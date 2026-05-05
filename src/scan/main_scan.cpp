#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "display_manager.h"
#include "mqtt_manager.h"
#include "http_manager.h"
#include "fingerprint_manager.h"
#include "ota_manager.h"

void setup()
{
    Serial.begin(115200);
    delay(1000);
    initLCD();
    connectWiFi();
    initOTA();

}

void loop()
{
    connectWiFi();
    handleOTA(); // Necessário para processar os eventos do OTA
    mqttLoop();

    // Lógica de scan do dedo
}