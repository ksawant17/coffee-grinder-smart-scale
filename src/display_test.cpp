#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// Create display object - using hardware I2C
// For yellow/blue OLED display, use the SSD1306 128x64 driver
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void setup() {
    Serial.begin(115200);
    Wire.begin();  // Initialize I2C
    
    // Initialize display
    if (!u8g2.begin()) {
        Serial.println("Display initialization failed!");
        while (1) { delay(1000); }  // Halt if display init fails
    }
    
    u8g2.clearBuffer();  // Clear the internal memory
    u8g2.setFont(u8g2_font_ncenB10_tr);  // Choose a suitable font
    
    // Calculate text width and height
    const char* text = "Welcome";
    int16_t width = u8g2.getStrWidth(text);
    int16_t height = u8g2.getAscent() - u8g2.getDescent();
    
    // Calculate center position
    int16_t x = (128 - width) / 2;
    int16_t y = 32 + (height / 2);  // 32 is half of display height (64)
    
    // Draw the text
    u8g2.drawStr(x, y, text);
    u8g2.sendBuffer();  // Transfer internal memory to the display
    
    Serial.println("Display test initialized");
}

void loop() {
    // Nothing to do in the loop for this test
    delay(100);
}
