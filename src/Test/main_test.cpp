#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>

// ── Hardware config ──────────────────────────────────────────
LiquidCrystal_I2C lcd(0x27, 16, 2);

HardwareSerial fpSerial(2);
Adafruit_Fingerprint finger(&fpSerial);

// ── LCD state machine ────────────────────────────────────────
enum State { IDLE, SCANNING, MATCHED, FAILED };

void lcdShow(State s, int slotId = -1, int confidence = -1) {
  lcd.clear();
  switch (s) {
    case IDLE:
      lcd.setCursor(0, 0); lcd.print("  Place finger  ");
      lcd.setCursor(0, 1); lcd.print("  to clock in   ");
      break;

    case SCANNING:
      lcd.setCursor(0, 0); lcd.print("   Scanning...  ");
      lcd.setCursor(0, 1); lcd.print("  Please wait   ");
      break;

    case MATCHED:
      lcd.setCursor(0, 0); lcd.print("  Access OK!    ");
      lcd.setCursor(0, 1);
      lcd.printf("  Slot #%-3d  %3d", slotId, confidence);
      break;

    case FAILED:
      lcd.setCursor(0, 0); lcd.print(" Not recognised ");
      lcd.setCursor(0, 1); lcd.print("  Try again...  ");
      break;
  }
}

// ── Scan logic ───────────────────────────────────────────────
int scanFinger() {
  if (finger.getImage()      != FINGERPRINT_OK) return -1;
  if (finger.image2Tz()      != FINGERPRINT_OK) return -2;
  if (finger.fingerSearch()  != FINGERPRINT_OK) return -3;
  return finger.fingerID;
}

// ── Setup ────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  // LCD
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0); lcd.print("  Initialising  ");
  lcd.setCursor(0, 1); lcd.print("  Please wait   ");

  // Sensor
  fpSerial.begin(57600, SERIAL_8N1, 16, 17);
  finger.begin(57600);

  if (!finger.verifyPassword()) {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print(" Sensor error!  ");
    lcd.setCursor(0, 1); lcd.print(" Check wiring   ");
    Serial.println("Sensor not found!"); 
    while (1);
  }

  finger.getParameters();
  Serial.printf("Sensor OK — %d slots\n", finger.capacity);

  lcdShow(IDLE);
}

// ── Main loop ────────────────────────────────────────────────
void loop() {
  int result = scanFinger();

  if (result >= 0) {
    // ── Match ────────────────────────────────────
    Serial.printf("Match! Slot #%d  confidence: %d\n",
      finger.fingerID, finger.confidence);

    lcdShow(MATCHED, finger.fingerID, finger.confidence);
    delay(3000);

  } else if (result == -3) {
    // ── No match (finger read OK but no template) ─
    Serial.println("No match found");
    lcdShow(FAILED);
    delay(2000);

  } else {
    // ── No finger / blurry — stay idle ───────────
    lcdShow(IDLE);
  }

  delay(200);
}