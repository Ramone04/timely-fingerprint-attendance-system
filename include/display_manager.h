#pragma once

// Initialises the I2C bus and LCD backlight.
// Must be called once in setup() before any displayWrite() calls.
void displayInit();

// Clears the LCD and writes up to LCD_COLS characters on each row.
// Pass nullptr (or "") to leave a row blank.
void displayWrite(const char* row0, const char* row1);
