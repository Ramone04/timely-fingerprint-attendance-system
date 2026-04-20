#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "display_manager.h"
#include "mqtt_manager.h"

void setup() {
    Serial.begin(115200);   
    delay(1000);

    initLCD();
    connectWiFi();
    mqttSetup();
}

void loop()
{
    // Blocks here if Wi-Fi drops, and resumes only when connected again.
    connectWiFi();
    mqttLoop();

    //Teste de subscrição e publicação MQTT
    mqttSubscribe(TOPIC_ENROLL_USERID);
    mqttSubscribe(TOPIC_ENROLL_NOME);
    mqttPublish(TOPIC_ENROLL_RESPONSE, "ESP32 ready to enroll");
    delay(10000); // Publish status every 10 seconds
}