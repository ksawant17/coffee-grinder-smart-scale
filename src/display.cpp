#include "display.hpp"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

TaskHandle_t DisplayTask;

#define SLEEP_AFTER_MS 10 * 1000 // sleep after 10 seconds

void centerPrintToScreen(char const *str, u8g2_uint_t y) {
  u8g2_uint_t width = u8g2.getStrWidth(str);
  u8g2.setCursor(128 / 2 - width / 2, y);
  u8g2.print(str);
}

void updateDisplay( void * parameter) {
  char buf[64];

  for(;;) {
    u8g2.clearBuffer();
    if (millis() - lastSignificantWeightChangeAt > SLEEP_AFTER_MS) {
      u8g2.sendBuffer();
      delay(100);
      continue;
    }

    if (scaleLastUpdatedAt == 0) {
      u8g2.setFontPosTop();
      u8g2.drawRFrame(16, 12, 96, 40, 4);  // Rounded rectangle frame
      u8g2.setFont(u8g2_font_8x13B_tr);
      centerPrintToScreen("Init...", 28);
    } else if (!scaleReady) {
      u8g2.setFontPosTop();
      u8g2.drawRFrame(16, 12, 96, 40, 4);  // Rounded rectangle frame
      u8g2.setFont(u8g2_font_8x13B_tr);
      centerPrintToScreen("SCALE ERROR", 28);
    } else {
      if (scaleStatus == STATUS_GRINDING_IN_PROGRESS) {
        // Title with decorative line
        u8g2.drawHLine(0, 12, 128);
        u8g2.setFontPosTop();
        u8g2.setFont(u8g2_font_8x13B_tr);
        centerPrintToScreen("Grinding...", 2);


        u8g2.setFontPosCenter();
        u8g2.setFont(u8g2_font_9x18B_tf);  // Larger, bolder font for better visibility
        u8g2.setCursor(0, 32);
        snprintf(buf, sizeof(buf), "%3.1fg", scaleWeight - cupWeightEmpty);
        u8g2.print(buf);

        u8g2.setFontPosCenter();
        u8g2.setFont(u8g2_font_unifont_t_symbols);
        u8g2.drawGlyph(64, 32, 0x2794);

        u8g2.setFontPosCenter();
        u8g2.setFont(u8g2_font_7x14B_tf);
        u8g2.setCursor(84, 32);
        snprintf(buf, sizeof(buf), "%3.1fg", (float)COFFEE_DOSE_WEIGHT);
        u8g2.print(buf);

        u8g2.setFontPosBottom();
        u8g2.setFont(u8g2_font_7x13_tr);
        snprintf(buf, sizeof(buf), "%3.1fs", (double)(millis() - startedGrindingAt) / 1000);
        centerPrintToScreen(buf, 64);
      } else if (scaleStatus == STATUS_EMPTY) {
        // Draw a box around the display area
        u8g2.drawRFrame(0, 0, 128, 64, 2);
        
        // Title with underline
        u8g2.setFontPosTop();
        u8g2.setFont(u8g2_font_8x13B_tr);
        centerPrintToScreen("Weight:", 4);
        u8g2.drawHLine(32, 20, 64);  // Underline

        // Weight display with box
        u8g2.drawRFrame(24, 24, 80, 28, 3);  // Box around weight
        u8g2.setFont(u8g2_font_9x18B_tf);
        u8g2.setFontPosCenter();
        snprintf(buf, sizeof(buf), "%3.1fg", scaleWeight);
        centerPrintToScreen(buf, 38);
      } else if (scaleStatus == STATUS_GRINDING_FAILED) {
        // Error box with double frame
        u8g2.drawRFrame(8, 4, 112, 56, 4);
        u8g2.drawRFrame(12, 8, 104, 48, 4);

        // Warning symbol
        u8g2.setFont(u8g2_font_unifont_t_symbols);
        centerPrintToScreen("⚠", 16);

        u8g2.setFontPosTop();
        u8g2.setFont(u8g2_font_7x14B_tf);
        centerPrintToScreen("Grinding failed", 28);

        u8g2.setFontPosTop();
        u8g2.setFont(u8g2_font_7x13_tr);
        centerPrintToScreen("Press the balance", 42);
        centerPrintToScreen("to reset", 52);
      } else if (scaleStatus == STATUS_GRINDING_FINISHED) {
        // Success box with checkmark
        u8g2.drawRFrame(0, 0, 128, 64, 3);
        
        u8g2.setFontPosTop();
        // Add success checkmark
        u8g2.setFont(u8g2_font_unifont_t_symbols);
        centerPrintToScreen("✓", 2);
        
        u8g2.setFont(u8g2_font_7x13_tr);
        centerPrintToScreen("Grinding finished", 16);

        u8g2.setFontPosCenter();
        u8g2.setFont(u8g2_font_7x14B_tf);
        u8g2.setCursor(0, 32);
        snprintf(buf, sizeof(buf), "%3.1fg", scaleWeight - cupWeightEmpty);
        u8g2.print(buf);

        u8g2.setFontPosCenter();
        u8g2.setFont(u8g2_font_unifont_t_symbols);
        u8g2.drawGlyph(64, 32, 0x2794);

        u8g2.setFontPosCenter();
        u8g2.setFont(u8g2_font_7x14B_tf);
        u8g2.setCursor(84, 32);
        snprintf(buf, sizeof(buf), "%3.1fg", (float)COFFEE_DOSE_WEIGHT);
        u8g2.print(buf);

        u8g2.setFontPosBottom();
        u8g2.setFont(u8g2_font_7x13_tr);
        u8g2.setCursor(64, 64);
        snprintf(buf, sizeof(buf), "%3.1fs", (double)(finishedGrindingAt - startedGrindingAt) / 1000);
        centerPrintToScreen(buf, 64);
      }
    }
    u8g2.sendBuffer();
    // delay(100);
  }
}

void setupDisplay() {
  u8g2.begin();
  u8g2.setContrast(255);  // Maximum contrast for better visibility
  u8g2.setFont(u8g2_font_7x13_tr);

  xTaskCreatePinnedToCore(
      updateDisplay, /* Function to implement the task */
      "Display", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      0,  /* Priority of the task */
      &DisplayTask,  /* Task handle. */
      1); /* Core where the task should run */
}
