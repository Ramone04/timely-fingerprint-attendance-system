#pragma once

// Display manager API for LCD initialization and message output.
void initLCD();

// Print up to two lines; text is truncated or padded to LCD width.
void LCDMessage(const char* line1, const char* line2 = "");
