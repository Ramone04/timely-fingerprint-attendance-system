#include <display_manager.h>
#include <config.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

static LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

// Periodic re-init helps recover the LCD if it glitches on long uptimes.
static const unsigned long LCD_REINIT_INTERVAL_MS = 5 * 60 * 1000UL;
static unsigned long lastReinitMs = 0;

// Cache the last message so it can be restored after a re-init.
static char lastLine1[17] = "";
static char lastLine2[17] = "";

static String normalizeLine(const char *text)
{
    // Trim or pad to the LCD column width so old characters are overwritten.
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

// Internal helper: writes both lines to the LCD without touching the cache.
static void renderLCD(const char *line1, const char *line2)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(normalizeLine(line1));
    lcd.setCursor(0, 1);
    lcd.print(normalizeLine(line2));
}

void initLCD()
{
    // Initialize I2C and the LCD controller.
    Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);
    Wire.setClock(10000);
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lastReinitMs = millis();
}

void LCDMessage(const char *line1, const char *line2)
{
    // Store the message so updateLCD() can restore it after a re-init.
    strncpy(lastLine1, line1 ? line1 : "", sizeof(lastLine1) - 1);
    lastLine1[sizeof(lastLine1) - 1] = '\0';

    strncpy(lastLine2, line2 ? line2 : "", sizeof(lastLine2) - 1);
    lastLine2[sizeof(lastLine2) - 1] = '\0';

    renderLCD(line1, line2);
}

// Call from loop() to re-init periodically and recover from LCD glitches.
void updateLCD()
{
    if (millis() - lastReinitMs < LCD_REINIT_INTERVAL_MS)
        return;

    // Re-init and re-render the last cached message.
    lcd.init();
    lcd.backlight();
    renderLCD(lastLine1, lastLine2);
    lastReinitMs = millis();
}