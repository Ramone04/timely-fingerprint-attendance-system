#include "wifi_manager.h"
#include "config.h"
#include <display_manager.h>
#include <WiFi.h>

void connectWiFi() {
    if (WiFi.status() == WL_CONNECTED) return;
    
    Serial.printf("A ligar ao WiFi: %s", WIFI_SSID);
    LCDMessage("Connecting to", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("WiFi OK - IP: %s", WiFi.localIP().toString().c_str());
    LCDMessage("WiFi OK", ("IP: " + WiFi.localIP().toString()).c_str());
}