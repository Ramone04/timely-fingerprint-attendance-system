#include <Arduino.h>
// Test 1 — Verify connection (corrected for AS608 8-pin)
// Wire: V+(red)→3V3, GND(black)→GND,
//       TX(orange,pin2)→GPIO16, RX(yellow,pin3)→GPIO17
// Pins 5-8 (TCH,VA,D+,D-) left floating
// Open Serial Monitor at 115200 baud

#include <Adafruit_Fingerprint.h>

HardwareSerial fpSerial(2);
Adafruit_Fingerprint finger(&fpSerial);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // RX pin = 16, TX pin = 17
  fpSerial.begin(57600, SERIAL_8N1, 16, 17);
  finger.begin(57600);

  Serial.println("AS608 connection test...");

  if (finger.verifyPassword()) {
    Serial.println("Sensor found!");
    finger.getParameters();
    Serial.printf("Capacity  : %d templates\\n", finger.capacity);
    Serial.printf("Security  : level %d\\n",    finger.security_level);
    Serial.printf("Baud rate : %d\\n",           finger.baud_rate * 9600);
  } else {
    Serial.println("Sensor NOT found! Check:");
    Serial.println(" - TX(orange)→GPIO16, RX(yellow)→GPIO17 ?");
    Serial.println(" - V+ on 3V3 ?");
    Serial.println(" - GND connected ?");
    while(1) delay(1);
  }
}

void loop() {}