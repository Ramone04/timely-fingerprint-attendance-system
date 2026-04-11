#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>


// ===== WiFi =====
const char* WIFI_SSID = "your-ssid";
const char* WIFI_PASS = "your-password";

// ===== HiveMQ =====
const char* MQTT_HOST   = "hostname";
const int   MQTT_PORT   = 8883;
const char* MQTT_USER   = "username";
const char* MQTT_PASS   = "password";
const char* MQTT_CLIENT = "client-id"; // único por dispositivo

// ===== Tópicos =====
const char* TOPIC_UserID = "Enroll/UserID";
const char* TOPIC_Nome   = "Enroll/Nome";
const char* TOPIC_Res    = "Enroll/Response";
const char* TOPIC_FingerID = "Ponto/FingerID";
const char* TOPIC_PRes   = "Ponto/Response";

WiFiClientSecure net;
PubSubClient mqtt(net);

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  Serial.print("WiFi a ligar a ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi OK!");
  Serial.print("IP: "); Serial.println(WiFi.localIP());
}

void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("MQTT a ligar...");
    // FIX 2: connect(clientID, user, pass) — três argumentos
    if (mqtt.connect(MQTT_CLIENT, MQTT_USER, MQTT_PASS)) {
      Serial.println(" OK!");
      mqtt.subscribe(TOPIC_PRes);
    } else {
      Serial.print(" falhou rc=");
      Serial.print(mqtt.state());
      Serial.println(" (tentar em 2s)");
      delay(2000);
    }
  }
}

// Callback — chamado quando chega uma mensagem subscrita
void onMessage(char* topic, byte* payload, unsigned int len) {
  String msg;
  for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];
  Serial.printf("Mensagem recebida [%s]: %s\n", topic, msg.c_str());
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  connectWiFi();
  net.setInsecure();
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMessage);
  connectMQTT();
}

void loop() {
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  static unsigned long last = 0;
  if (millis() - last > 30000) {
    mqtt.publish(TOPIC_FingerID, "5");
    last = millis();
  }
}