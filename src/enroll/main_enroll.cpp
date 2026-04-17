#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "display_manager.h"
#include "mqtt_manager.h"

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
    Serial.print("MQTT mensagem em ");
    Serial.print(topic);
    Serial.print(": ");

    for (unsigned int index = 0; index < length; index++) {
        Serial.print((char)payload[index]);
    }
    Serial.println();
}

void setup() {
    Serial.begin(115200);   
    delay(1000);

    initLCD();
    connectWiFi();
    mqttSetup(onMqttMessage);
}

void loop()
{
    static bool mqttSubscribed = false;

    // Blocks here if Wi-Fi drops, and resumes only when connected again.
    connectWiFi();
    mqttLoop();

    if (mqttConnect()) {
        if (!mqttSubscribed) {
            mqttSubscribed = mqttSubscribe(TOPIC_ENROLL_RESPONSE);
        }
    } else {
        mqttSubscribed = false;
    }
}