#include "display.hpp"
#include "scale_config.hpp"
#include "display_faces.hpp"
#include <Wire.h>

// Initialize the OLED display using hardware I2C
// XIAO ESP32-C3 uses GPIO4 (SDA) and GPIO5 (SCL)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// FreeRTOS task handle for the display update task
TaskHandle_t DisplayTask;

// Display will go to sleep after 180 seconds (3 minutes) of no significant weight changes
#define SLEEP_AFTER_MS 180 * 1000 

// Animation frame counter
static uint8_t animationFrame = 0;

/**
 * Utility function to center text horizontally on the screen
 * @param str Text to center
 * @param y Vertical position for the text
 */
void centerPrintToScreen(char const *str, u8g2_uint_t y) {
    u8g2_uint_t width = u8g2.getStrWidth(str);
    u8g2.setCursor(128 / 2 - width / 2, y);
    u8g2.print(str);
}

/**
 * Main display update task - runs continuously
 * Updates the OLED display based on scale status and measurements
 */
void updateDisplay(void * parameter) {
    char buf[64];  // Buffer for formatting text strings

    // Infinite loop for continuous display updates
    for(;;) {
        u8g2.clearBuffer();  // Clear the display buffer
        u8g2.setPowerSave(0);  // Ensure display is on

        // Get current weight
        float currentWeight = scale.getCurrentWeight();
        snprintf(buf, sizeof(buf), "%.1f g", currentWeight);
        
        // Draw weight in large font in center
        u8g2.setFont(u8g2_font_ncenB14_tr);
        centerPrintToScreen(buf, 35);

        // Draw status line at bottom
        u8g2.setFont(u8g2_font_7x13_tr);
        centerPrintToScreen("Ready", 60);

        // Send to display
        u8g2.sendBuffer();

        // Small delay to prevent task from consuming too much CPU
        delay(50);
        if (scale.getLastUpdateTime() == 0) {
            // Scale not yet initialized - show startup animation
            Faces::drawStartup(u8g2, animationFrame++);
            u8g2.setFont(u8g2_font_7x13_tr);
            centerPrintToScreen("Starting up...", 50);
        } else if (!scale.isReady()) {
            // Scale error condition - show sad face
            Faces::drawSad(u8g2);
            u8g2.setFont(u8g2_font_7x13_tr);
            centerPrintToScreen("SCALE ERROR", 50);
        } else {
            // Handle different scale states
            switch (scale.getStatus()) {
                case ScaleStatus::GRINDING_IN_PROGRESS: {
                    // Show working face animation
                    Faces::drawWorking(u8g2);
                    
                    // Display current weight (in yellow region)
                    u8g2.setFontPosCenter();
                    u8g2.setFont(u8g2_font_9x18B_tf); // Larger font for better visibility
                    snprintf(buf, sizeof(buf), "%3.1fg", 
                            scale.getCurrentWeight() - scale.getCupEmptyWeight());
                    u8g2.setCursor(0, 48);
                    u8g2.print(buf);

                    // Draw arrow symbol (in yellow-blue boundary)
                    u8g2.setFont(u8g2_font_unifont_t_symbols);
                    u8g2.drawGlyph(64, 48, 0x2794);

                    // Display target weight (in blue region)
                    u8g2.setFont(u8g2_font_9x18B_tf);
                    u8g2.setCursor(84, 48);
                    snprintf(buf, sizeof(buf), "%3.1fg", 
                            (float)DEFAULT_SCALE_CONFIG.coffeeTargetWeight);
                    u8g2.print(buf);
                    break;
                }

                case ScaleStatus::EMPTY: {
                    // Show happy idle face
                    Faces::drawHappy(u8g2);
                    
                    // Show current weight in large font (in blue region for contrast)
                    u8g2.setFont(u8g2_font_9x18B_tf);
                    u8g2.setFontPosBottom();
                    snprintf(buf, sizeof(buf), "%3.1fg", scale.getCurrentWeight());
                    centerPrintToScreen(buf, 60);
                    break;
                }

                case ScaleStatus::GRINDING_FAILED: {
                    // Show error message when grinding fails
                    u8g2.setFontPosTop();
                    u8g2.setFont(u8g2_font_7x14B_tf);
                    centerPrintToScreen("Grinding failed", 0);

                    u8g2.setFont(u8g2_font_7x13_tr);
                    centerPrintToScreen("Press the balance", 32);
                    centerPrintToScreen("to reset", 42);
                    break;
                }

                case ScaleStatus::GRINDING_FINISHED: {
                    // Show completion screen with final weight and time
                    u8g2.setFontPosTop();
                    u8g2.setFont(u8g2_font_7x13_tr);
                    centerPrintToScreen("Grinding finished", 0);

                    // Display final weight
                    u8g2.setFontPosCenter();
                    u8g2.setFont(u8g2_font_7x14B_tf);
                    snprintf(buf, sizeof(buf), "%3.1fg", 
                            scale.getCurrentWeight() - scale.getCupEmptyWeight());
                    u8g2.setCursor(0, 32);
                    u8g2.print(buf);

                    // Draw arrow symbol
                    u8g2.setFont(u8g2_font_unifont_t_symbols);
                    u8g2.drawGlyph(64, 32, 0x2794);

                    // Display target weight
                    u8g2.setFont(u8g2_font_7x14B_tf);
                    u8g2.setCursor(84, 32);
                    snprintf(buf, sizeof(buf), "%3.1fg", 
                            (float)DEFAULT_SCALE_CONFIG.coffeeTargetWeight);
                    u8g2.print(buf);

                    // Show total grinding time
                    u8g2.setFontPosBottom();
                    u8g2.setFont(u8g2_font_7x13_tr);
                    snprintf(buf, sizeof(buf), "%3.1fs", 
                            (double)(scale.getGrindEndTime() - scale.getGrindStartTime()) / 1000);
                    centerPrintToScreen(buf, 64);
                    break;
                }
            }
        }
        u8g2.sendBuffer();  // Update the display with new content
        delay(50);
    }
}

/**
 * Initialize the display and start the display update task
 */
void setupDisplay() {
    // Initialize I2C for XIAO ESP32-C3
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(400000); // Set I2C clock to 400kHz

    // Initialize display
    u8g2.begin();  // Initialize the U8G2 display
    u8g2.setFont(u8g2_font_7x13_tr);  // Set default font

    // Create FreeRTOS task for display updates
    xTaskCreatePinnedToCore(
        updateDisplay,    // Task function
        "Display",        // Task name
        10000,           // Stack size (words)
        NULL,            // Task parameters
        1,               // Priority (slightly higher than default)
        &DisplayTask,    // Task handle
        0);             // Run on core 0 (ESP32-C3 has 1 core, but core 0 is main)
}
