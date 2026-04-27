#include "mqtt_manager.h"
#include "config.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include "display_manager.h"
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

bool mqttPublish(const char* topic, const char* payload)
{
    if (!mqttClient.connected()) return false;
    return mqttClient.publish(topic, payload);
}

bool sendEnrollStatus(uint16_t userId, uint8_t status)
{
    WiFiClientSecure httpClient;
    HTTPClient http;

    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    httpClient.setCACert(ENROLL_API_CA_CERT);
    if (!http.begin(httpClient, ENROLL_STATUS_URL)) {
        httpClient.stop();
        return false;
    }

    http.addHeader("Content-Type", "application/json");
    // {"user_id":65535,"status":255} => 29 chars + '\0'
    char payload[64];
    snprintf(payload, sizeof(payload), "{\"user_id\":%u,\"status\":%u}", userId, status);

    int code = http.POST(payload);
    Serial.printf("Enroll status HTTP code: %d\n", code);
    http.end();
    httpClient.stop();

    return code >= 200 && code < 300;
}
