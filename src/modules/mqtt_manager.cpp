#include "mqtt_manager.h"
#include "config.h"
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include "display_manager.h"

static WiFiClientSecure net;
PubSubClient mqttClient(net);

// CALLBACK
void callback(char* topic, byte* payload, unsigned int length) {
    String msg;

    for (int i = 0; i < length; i++) {
        msg += (char)payload[i];
    }

    Serial.print("Topic: ");
    Serial.println(topic);


    Serial.print("Message: ");
    Serial.println(msg);

    //Teste de exibição de mensagens recebidas no LCD
    LCDMessage(topic, msg.c_str());
}

void mqttSetup()
{
    net.setInsecure(); // Disable SSL certificate validation (not recommended for production)   
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setCallback(callback);
}

bool mqttConnect() {
    if (mqttClient.connected()) return true;
    if (WiFi.status() != WL_CONNECTED) return false;
    
    if (mqttClient.connect(MQTT_CLIENT, MQTT_USER, MQTT_PASS)) {
        mqttClient.subscribe(TOPIC_ENROLL_USERID);
        mqttClient.subscribe(TOPIC_ENROLL_NOME);
        return true;
    }
    return false;
}

void mqttLoop()
{
    if (WiFi.status() != WL_CONNECTED) return;
    if (!mqttClient.connected()) {
        mqttConnect();
        return;
    }

    mqttClient.loop();
}

bool mqttPublish(const char* topic, const char* payload)
{
    if (!mqttClient.connected()) return false;
    return mqttClient.publish(topic, payload);
}

