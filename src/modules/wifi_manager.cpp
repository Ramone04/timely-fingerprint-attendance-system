#include "wifi_manager.h"
#include "config.h"
#include <display_manager.h>
#include <WiFi.h>

// wifi_manager.cpp
void connectWiFi()
{
    if (WiFi.status() == WL_CONNECTED)
        return;

    Serial.printf("A ligar ao WiFi: %s\n", WIFI_SSID);
    LCDMessage("Connecting to", WIFI_SSID);

    WiFi.disconnect(true); // ← garante limpeza do estado anterior
    delay(100);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    // ← espera que o DNS esteja operacional antes de regressar
    // Faz um resolve de teste — se falhar aguarda mais
    Serial.println("\nWiFi ligado — a verificar DNS...");
    IPAddress testIP;
    int dnsRetries = 0;
    while (!WiFi.hostByName("pool.ntp.org", testIP) && dnsRetries < 10)
    {
        Serial.println("DNS ainda não disponível — a aguardar...");
        delay(1000);
        dnsRetries++;
    }

    Serial.printf("WiFi OK — IP: %s\n", WiFi.localIP().toString().c_str());
    LCDMessage("WiFi OK", ("IP: " + WiFi.localIP().toString()).c_str());
}