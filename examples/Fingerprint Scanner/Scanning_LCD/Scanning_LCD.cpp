#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include "ota_manager.h"
#include "wifi_manager.h"
#include "display_manager.h"

HardwareSerial fpSerial(2);
Adafruit_Fingerprint finger(&fpSerial);

// ── LCD state machine ────────────────────────────────────────
enum State
{
  IDLE,
  SCANNING,
  MATCHED,
  FAILED
};

void lcdShow(State s, int slotId = -1, int confidence = -1)
{
  switch (s)
  {
  case IDLE:
    LCDMessage("  Place finger  ", "  to clock in   ");
    break;

  case SCANNING:
    LCDMessage("   Scanning...  ", "  Please wait   ");
    break;

  case MATCHED:
  {
    char line2[17];
    snprintf(line2, sizeof(line2), "  Slot #%-3d  %3d", slotId, confidence);
    LCDMessage("  Access OK!    ", line2);
  }
    break;

  case FAILED:
    LCDMessage(" Not recognised ", "  Try again...  ");
    break;
  }
}

// ── Scan logic ───────────────────────────────────────────────
int scanFinger()
{
  if (finger.getImage() != FINGERPRINT_OK)
    return -1;
  if (finger.image2Tz() != FINGERPRINT_OK)
    return -2;
  if (finger.fingerSearch() != FINGERPRINT_OK)
    return -3;
  return finger.fingerID;
}

// ── Setup ────────────────────────────────────────────────────
void setup()
{
  Serial.begin(115200);

  // ── WiFi + OTA ── adiciona estas duas linhas antes do resto
  connectWiFi();
  initOTA();

  // LCD
  initLCD();
  LCDMessage("  Initialising  ", "  Please wait   ");

  // Sensor
  fpSerial.begin(57600, SERIAL_8N1, 16, 17);
  finger.begin(57600);

  if (!finger.verifyPassword())
  {
    LCDMessage(" Sensor error!  ", " Check wiring   ");
    Serial.println("Sensor not found!");
    while (1)
      ;
  }

  finger.getParameters();
  Serial.printf("Sensor OK — %d slots\n", finger.capacity);

  lcdShow(IDLE);
}

// ── Main loop ────────────────────────────────────────────────
void loop()
{
  connectWiFi();
  handleOTA(); // Necessário para processar os eventos do OTA

  int result = scanFinger();

  if (result >= 0)
  {
    // ── Match ────────────────────────────────────
    Serial.printf("Match! Slot #%d  confidence: %d\n",
                  finger.fingerID, finger.confidence);

    lcdShow(MATCHED, finger.fingerID, finger.confidence);
    delay(3000);
  }
  else if (result == -3)
  {
    // ── No match (finger read OK but no template) ─
    Serial.println("No match found");
    lcdShow(FAILED);
    delay(2000);
  }
  else
  {
    // ── No finger / blurry — stay idle ───────────
    lcdShow(IDLE);
  }

  delay(200);
}