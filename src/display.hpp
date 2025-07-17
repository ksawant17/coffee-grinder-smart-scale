#pragma once

#include <U8g2lib.h>
#include "scale.hpp"

extern Scale scale;  // Reference to the global scale object

void setupDisplay();
void updateDisplay(void * parameter);
