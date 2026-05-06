// MQTT manager: TLS setup, connect, and loop handling.
#include "mqtt_manager.h"
#include "config.h"
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include "certificates.h"

// TLS socket used by PubSubClient
static WiFiClientSecure net;
PubSubClient mqttClient(net);

// Configure TLS and broker settings; call once during setup.
void mqttSetup(MqttCallback callback)
{
    // Load CA certificate for HiveMQ TLS
    net.setCACert(HIVEMQ_CA_CERT);
    // net.setHandshakeTimeout(30);
    // net.setInsecure(); // Accepts any certificate (not recommended for production)
    // Set broker endpoint and register the message callback.
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setCallback(callback);
}

// Ensure a connected session and active subscriptions.
// Returns true on success, false when connection fails.
bool mqttConnect()
{
    if (mqttClient.connected())
        return true;
    if (WiFi.status() != WL_CONNECTED)
        return false;

    // Attempt authenticated connection to the broker.
    if (mqttClient.connect(MQTT_CLIENT, MQTT_USER, MQTT_PASS))
    {
        // Subscribe to topics after a successful session
        mqttClient.subscribe(TOPIC_ENROLL_USERID);
        mqttClient.subscribe(TOPIC_ENROLL_NOME);
        mqttClient.subscribe(TOPIC_DELETE_USERID);
        return true;
    }

    // Capture TLS error details for diagnostics.
    char sslError[128] = {0};
    net.lastError(sslError, sizeof(sslError));
    // Log TLS error details to help diagnose connection failures
    Serial.printf("MQTT connect falhou. state=%d tls=%s\n", mqttClient.state(), sslError);
    return false;
}

// Keep the MQTT client alive; call frequently from the main loop.
void mqttLoop()
{
    // Do nothing if WiFi is down; caller should handle reconnect
    if (WiFi.status() != WL_CONNECTED)
        return;
    if (!mqttClient.connected())
    {
        // Attempt reconnect; if it fails we'll try again next loop
        mqttConnect();
        return;
    }

    // Process inbound/outbound traffic and callbacks.
    mqttClient.loop();
}
