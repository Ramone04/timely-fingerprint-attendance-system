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
    if (WiFi.status() != WL_CONNECTED) return false;

    WiFiClientSecure httpClient;
    HTTPClient http;

    httpClient.setCACert(ENROLL_API_CA_CERT);
    if (!http.begin(httpClient, ENROLL_STATUS_URL)) return false;

    http.addHeader("Content-Type", "application/json");
    String payload = "{\"user_id\":";
    payload += String(userId);
    payload += ",\"status\":";
    payload += String(status);
    payload += "}";

    int code = http.POST(payload);
    Serial.printf("Enroll status HTTP code: %d\n", code);
    http.end();

    return code >= 200 && code < 300;
}
