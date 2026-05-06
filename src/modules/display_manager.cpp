#include <display_manager.h>
#include <config.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

static String normalizeLine(const char *text)
{
    String line = text ? String(text) : String("");
    if (line.length() > LCD_COLS)
    {
        line = line.substring(0, LCD_COLS);
    }
    while (line.length() < LCD_COLS)
    {
        line += ' ';
    }
    return line;
}

void initLCD()
{
    Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);
    lcd.init();
    lcd.backlight();
    lcd.clear();
}

void LCDMessage(const char *line1, const char *line2)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(normalizeLine(line1));
    lcd.setCursor(0, 1);
    lcd.print(normalizeLine(line2));
}