// Test 4 — Attendance-style display simulation
// Cycles through all the states the display will show in real use:
// idle → scanning → match → fail → idle...

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

enum State { IDLE, SCANNING, MATCH };

void showState(State s, String name = "") {
  lcd.clear();
  switch (s) {
    case IDLE:
      lcd.setCursor(0, 0); lcd.print("  Place finger  ");
      lcd.setCursor(0, 1); lcd.print("   to clock in  ");
      break;
    case SCANNING:
      lcd.setCursor(0, 0); lcd.print("  Scanning...   ");
      lcd.setCursor(0, 1); lcd.print("  Please wait   ");
      break;
    case MATCH:
      lcd.setCursor(0, 0); lcd.print("  Welcome!      ");
      lcd.setCursor(0, 1);
      if (name.length() > 0) {
        // centre the name on 16 chars
        int pad = (16 - name.length()) / 2;
        lcd.print(String(' ', pad) + name);
      } else {
        lcd.print("  Access OK     ");
      }
      break;
  }
}

void setup() {
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
}

void loop() {
  showState(IDLE);           delay(3000);
  showState(SCANNING);       delay(2000);
  showState(MATCH, "Joao S."); delay(3000);
  showState(IDLE);           delay(2000);
  showState(SCANNING);       delay(2000);
}