#pragma once

#include "scale_types.hpp"

// XIAO ESP32-C3 Pin Definitions
#define PIN_HX711_DOUT 10    // D10 (GPI10)  - HX711 DOUT pin
#define PIN_HX711_SCK  8    // D8 (GPIO8)  - HX711 SCK pin
#define PIN_GRINDER    D1   // D1 (GPIO2)  - Grinder control pin
#define PIN_I2C_SDA    4    // D4 (GPIO4)  - I2C SDA for OLED
#define PIN_I2C_SCL    5    // D5 (GPIO5)  - I2C SCL for OLED

// Default scale configuration
const ScaleConfig DEFAULT_SCALE_CONFIG = {
    .cupWeight = 59.2,              // Empty cup weight in grams
    .cupDetectionTolerance = 5.0,   // Tolerance for cup detection
    .coffeeTargetWeight = 13.0,     // Target coffee weight in grams
    .tareMeasures = 20,             // Number of measurements for taring
    .significantWeightChange = 5.0,  // Significant weight change threshold
    .maxGrindingTime = 20000,       // Maximum grinding time (20 seconds)
    .weightToReset = 500.0,         // Weight threshold to reset after failure
    .tareMinInterval = 10000,       // Minimum time between auto-tare (10 seconds)
    .loadcellScaleFactor = 449.33   // Load cell calibration factor
};

// Default pin configuration
const PinConfig DEFAULT_PIN_CONFIG = {
    .loadcellDout = PIN_HX711_DOUT,
    .loadcellSck = PIN_HX711_SCK,
    .grinderActive = PIN_GRINDER
};
