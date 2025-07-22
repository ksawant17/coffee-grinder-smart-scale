#pragma once

#include <U8g2lib.h>
#include "scale.hpp"

extern Scale scale;  // Reference to the global scale object

// Declare the display object as external so it can be accessed from other files
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

void setupDisplay();
void updateDisplay(void * parameter);

// Utility function to center text on the display
void centerPrintToScreen(const char* str, u8g2_uint_t y);
