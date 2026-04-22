#pragma once
#include <PubSubClient.h>

// Tipo do callback 
typedef void (*MqttCallback)(char*, byte*, unsigned int);

void mqttSetup(MqttCallback callback);
bool mqttConnect();
void mqttLoop();
bool mqttPublish(const char* topic, const char* payload);