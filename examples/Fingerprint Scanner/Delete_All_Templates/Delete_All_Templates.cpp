#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
HardwareSerial fpSerial(2);
Adafruit_Fingerprint finger(&fpSerial);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();

  fpSerial.begin(57600, SERIAL_8N1, 16, 17);
  finger.begin(57600);

  if (!finger.verifyPassword()) {
    lcd.setCursor(0, 0); lcd.print(" Sensor error!  ");
    Serial.println("Sensor not found!"); while(1);
  }

  finger.getParameters();
  uint16_t count = finger.templateCount;
  finger.getTemplateCount();

  Serial.println("=============================");
  Serial.println("  FINGERPRINT WIPE UTILITY   ");
  Serial.println("=============================");
  Serial.printf("Templates stored: %d\n", finger.templateCount);
  Serial.println("\nType  YES  and press Enter to delete ALL templates.");
  Serial.println("Type anything else to cancel.");

  lcd.setCursor(0, 0); lcd.print("Type YES in     ");
  lcd.setCursor(0, 1); lcd.print("Serial to wipe  ");
}

void loop() {
  if (!Serial.available()) return;

  String input = Serial.readStringUntil('\n');
  input.trim();

  if (input == "YES") {
    Serial.println("\nDeleting all templates...");
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("  Wiping all    ");
    lcd.setCursor(0, 1); lcd.print("  templates...  ");

    if (finger.emptyDatabase() == FINGERPRINT_OK) {
      Serial.println("Done! All templates deleted.");
      Serial.println("Sensor is clean and ready for production.");
      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("  Done! Sensor  ");
      lcd.setCursor(0, 1); lcd.print("  is now clean  ");
    } else {
      Serial.println("Delete failed! Try again.");
      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("  Delete failed ");
      lcd.setCursor(0, 1); lcd.print("  Try again     ");
    }

  } else {
    Serial.println("Cancelled — no templates were deleted.");
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("  Cancelled     ");
    lcd.setCursor(0, 1); lcd.print("  No changes    ");
  }

  // Stop here — require re-upload to run again
  while(1) delay(1);
}