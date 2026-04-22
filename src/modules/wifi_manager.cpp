#include "wifi_manager.h"
#include "config.h"
#include <display_manager.h>
#include <WiFi.h>

void connectWiFi()
{
    if (WiFi.status() == WL_CONNECTED)
        return;

    Serial.printf("A ligar ao WiFi: %s", WIFI_SSID);
    LCDMessage("Connecting to", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    IPAddress dns1(8, 8, 8, 8);
    IPAddress dns2(1, 1, 1, 1);

    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, dns1, dns2);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("WiFi OK - IP: %s", WiFi.localIP().toString().c_str());
    LCDMessage("WiFi OK", ("IP: " + WiFi.localIP().toString()).c_str());
}