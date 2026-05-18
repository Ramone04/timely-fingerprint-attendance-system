#include <display_manager.h>
#include <config.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

// Reinitialize the LCD every 5 minutes to prevent it from freezing due to I2C issues
static const unsigned long LCD_REINIT_INTERVAL_MS = 5 * 60 * 1000UL;
static unsigned long lastReinitMs = 0;

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
    Wire.setClock(100000); // Set I2C clock to 100kHz (more resistant to noise)
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lastReinitMs = millis();
}

void LCDMessage(const char *line1, const char *line2)
{
    // Reinitialize the LCD if it has been more than 5 minutes since the last initialization
    if (millis() - lastReinitMs >= LCD_REINIT_INTERVAL_MS)
    {
        lcd.init();
        lcd.backlight();
        lastReinitMs = millis();
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(normalizeLine(line1));
    lcd.setCursor(0, 1);
    lcd.print(normalizeLine(line2));
}