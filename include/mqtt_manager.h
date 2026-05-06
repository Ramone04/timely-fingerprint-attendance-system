#pragma once
#include <PubSubClient.h>

// MQTT callback signature used by PubSubClient.
typedef void (*MqttCallback)(char*, byte*, unsigned int);

// Configure TLS and broker settings; call once during setup.
void mqttSetup(MqttCallback callback);

// Connect and subscribe to required topics.
bool mqttConnect();

// Keep the MQTT client alive; call frequently from the main loop.
void mqttLoop();
