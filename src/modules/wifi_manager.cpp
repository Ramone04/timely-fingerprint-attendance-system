// WiFi manager: connects in STA mode and reports status on Serial/LCD.
#include "wifi_manager.h"
#include "config.h"
#include <display_manager.h>
#include <WiFi.h>

// Connect to the configured SSID using a static IP.
// Returns immediately when already connected.
void connectWiFi()
{
    // Already connected, nothing to do.
    if (WiFi.status() == WL_CONNECTED)
        return;

    // Report the target SSID on Serial and LCD.
    Serial.printf("A ligar ao WiFi: %s\n", WIFI_SSID);
    LCDMessage("Connecting to", WIFI_SSID);

    // Reset any previous connection state before configuring.
    WiFi.disconnect(true); // Reset any previous connection state
    delay(100);

    // Apply static network configuration before connecting
    IPAddress staticIP, gateway, subnet, dns1, dns2;
    staticIP.fromString(STATIC_IP);
    gateway.fromString(STATIC_GATEWAY);
    subnet.fromString(STATIC_SUBNET);
    dns1.fromString("8.8.8.8");
    dns2.fromString("1.1.1.1");

    // Configure network parameters and start STA connection.
    WiFi.config(staticIP, gateway, subnet, dns1, dns2);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    // Block until the connection is established
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    // Wait for DNS to be available before returning
    // Uses a test resolve and retries if it fails
    Serial.println("\nWiFi ligado — a verificar DNS...");
    IPAddress testIP;
    int dnsRetries = 0;
    while (!WiFi.hostByName("pool.ntp.org", testIP) && dnsRetries < 10)
    {
        Serial.println("DNS ainda não disponível — a aguardar...");
        delay(1000);
        dnsRetries++;
    }

    // Report the assigned IP and keep it visible briefly.
    Serial.printf("WiFi OK — IP: %s\n", WiFi.localIP().toString().c_str());
    LCDMessage("WiFi OK", ("IP:" + WiFi.localIP().toString()).c_str());
    delay(2000);
}