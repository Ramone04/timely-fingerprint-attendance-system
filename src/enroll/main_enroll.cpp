#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"

void setup() {
    Serial.begin(115200);   
    delay(1000);

    connectWiFi();
}

void loop()
{
    
}