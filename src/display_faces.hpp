#pragma once
#include <U8g2lib.h>

// Face expressions for different states
namespace Faces {
    // Draw a cute face based on given parameters
    void drawFace(U8G2_SSD1306_128X64_NONAME_F_HW_I2C& u8g2, 
                  bool eyesOpen, bool mouthSmile, bool mouthOpen,
                  uint8_t eyeStyle = 0) {
        // For yellow and blue SSD1306:
        // - Top half of the display (0-31 rows) shows in yellow
        // - Bottom half of the display (32-63 rows) shows in blue
        
        const uint8_t centerX = 64;
        const uint8_t centerY = 24; // Moved up to fit in yellow region
        
        // Draw eyes (in yellow region)
        if (eyesOpen) {
            switch (eyeStyle) {
                case 0: // Normal eyes
                    u8g2.drawCircle(centerX - 15, centerY - 5, 3);
                    u8g2.drawCircle(centerX + 15, centerY - 5, 3);
                    break;
                case 1: // Happy eyes (^ ^)
                    u8g2.drawDisc(centerX - 15, centerY - 5, 2);
                    u8g2.drawDisc(centerX + 15, centerY - 5, 2);
                    break;
                case 2: // Tired eyes (- -)
                    u8g2.drawHLine(centerX - 20, centerY - 5, 10);
                    u8g2.drawHLine(centerX + 10, centerY - 5, 10);
                    break;
            }
        } else {
            // Closed eyes
            u8g2.drawHLine(centerX - 20, centerY - 5, 10);
            u8g2.drawHLine(centerX + 10, centerY - 5, 10);
        }

        // Draw mouth (in blue region)
        if (mouthOpen) {
            u8g2.drawEllipse(centerX, centerY + 8, 8, 6);
        } else if (mouthSmile) {
            // Draw smile using small line segments
            for (int i = -10; i <= 10; i++) {
                int y = (i * i) / 20; // Parabolic curve
                u8g2.drawPixel(centerX + i, centerY + 6 + y);
            }
        } else {
            // Draw frown using small line segments
            for (int i = -10; i <= 10; i++) {
                int y = -(i * i) / 20; // Inverted parabolic curve
                u8g2.drawPixel(centerX + i, centerY + 10 + y);
            }
        }
    }

    // Different face expressions
    void drawHappy(U8G2_SSD1306_128X64_NONAME_F_HW_I2C& u8g2) {
        drawFace(u8g2, true, true, false, 1);
    }

    void drawSleepy(U8G2_SSD1306_128X64_NONAME_F_HW_I2C& u8g2) {
        drawFace(u8g2, false, false, false, 2);
    }

    void drawExcited(U8G2_SSD1306_128X64_NONAME_F_HW_I2C& u8g2) {
        drawFace(u8g2, true, true, true, 1);
    }

    void drawSad(U8G2_SSD1306_128X64_NONAME_F_HW_I2C& u8g2) {
        drawFace(u8g2, true, false, false, 0);
    }

    void drawWorking(U8G2_SSD1306_128X64_NONAME_F_HW_I2C& u8g2) {
        static uint8_t frame = 0;
        drawFace(u8g2, (frame % 2) == 0, true, false, 1);
        frame++;
    }

    void drawStartup(U8G2_SSD1306_128X64_NONAME_F_HW_I2C& u8g2, uint8_t frame) {
        // Animated startup sequence
        switch (frame % 4) {
            case 0:
                drawFace(u8g2, true, false, false, 0);
                break;
            case 1:
                drawFace(u8g2, true, false, true, 0);
                break;
            case 2:
                drawFace(u8g2, true, true, false, 1);
                break;
            case 3:
                drawFace(u8g2, false, true, false, 2);
                break;
        }
    }
}
