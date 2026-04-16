#include <Arduino.h>
#include "config.h"
#include "display_manager.h"
#include "wifi_manager.h"

void setup() {
    Serial.begin(115200);
    delay(1000);

    displayInit();
    displayWrite("Initialising...", "");

    connectWiFi();

    displayWrite("WiFi connected", "");
}

void loop()
{
    
}