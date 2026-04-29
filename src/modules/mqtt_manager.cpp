#include "mqtt_manager.h"
#include "config.h"
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include "certificates.h"

static WiFiClientSecure net;
PubSubClient mqttClient(net);

void mqttSetup(MqttCallback callback)
{
    net.setCACert(HIVEMQ_CA_CERT);
    // net.setHandshakeTimeout(30);
    //net.setInsecure(); // Aceita qualquer certificado (não recomendado para produção, mas simplifica o desenvolvimento)
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setCallback(callback);
}

bool mqttConnect() {
    if (mqttClient.connected()) return true;
    if (WiFi.status() != WL_CONNECTED) return false;
    
    if (mqttClient.connect(MQTT_CLIENT, MQTT_USER, MQTT_PASS)) {
        mqttClient.subscribe(TOPIC_ENROLL_USERID);
        mqttClient.subscribe(TOPIC_ENROLL_NOME);
        mqttClient.subscribe(TOPIC_DELETE_USERID);
        return true;
    }

    char sslError[128] = {0};
    net.lastError(sslError, sizeof(sslError));
    Serial.printf("MQTT connect falhou. state=%d tls=%s\n", mqttClient.state(), sslError);
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
