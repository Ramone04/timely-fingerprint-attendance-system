#pragma once
#include <PubSubClient.h>

// Tipo do callback 
typedef void (*MqttCallback)(char*, byte*, unsigned int);

void mqttSetup(MqttCallback callback);
bool mqttConnect();
void mqttLoop();
bool mqttPublish(const char* topic, const char* payload);
// Envia para o backend o resultado do enroll: status 1=sucesso, 0=falha.
// Retorna true apenas se a API responder com HTTP 2xx.
bool sendEnrollStatus(uint16_t userId, uint8_t status);
