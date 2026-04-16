#include "display_manager.h"
#include "config.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

static LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

void displayInit() {
    Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);
    lcd.init();
    lcd.backlight();
    lcd.clear();
}

void displayWrite(const char* row0, const char* row1) {
    lcd.clear();
    if (row0 && *row0) {
        lcd.setCursor(0, 0);
        lcd.print(row0);
    }
    if (row1 && *row1) {
        lcd.setCursor(0, 1);
        lcd.print(row1);
    }
}
