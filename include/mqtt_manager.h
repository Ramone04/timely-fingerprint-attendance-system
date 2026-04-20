#pragma once
#include <PubSubClient.h>

void mqttSetup();
bool mqttConnect();
void mqttLoop();
bool mqttPublish(const char* topic, const char* payload);

extern PubSubClient mqttClient;