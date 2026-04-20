#pragma once
#include <PubSubClient.h>

void mqttSetup();
bool mqttConnect();
void mqttLoop();
bool mqttPublish(const char* topic, const char* payload);
bool mqttSubscribe(const char* topic);

extern PubSubClient mqttClient;