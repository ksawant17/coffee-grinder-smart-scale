// Core Arduino library for basic functionality
#include <Arduino.h>
#include <HX711.h>
#include <EEPROM.h>
#include <Wire.h>

#if CALIBRATION_MODE
    // Global objects for calibration mode
    HX711 scale;
    const int HX711_DOUT_PIN = 19;
    const int HX711_SCK_PIN = 18;
    const int EEPROM_CAL_ADDRESS = 0;
#else
    #include "config.hpp"
    
    // Only include WiFi and MQTT if enabled
    #if ENABLE_MQTT
        // WiFi library for ESP32 network connectivity
        #include <WiFi.h>
        // MQTT client library for message queuing and telemetry
        #include <PubSubClient.h>
    #endif

    #include "display.hpp"
    #include "scale.hpp"
    #include "scale_config.hpp"
    #include "error_handler.hpp"
    
    #if ENABLE_MQTT
        #include "mqtt_manager.hpp"
    #endif

    // Global objects for normal mode
    Scale scale;
    #if ENABLE_MQTT
        MQTTManager mqtt;
    #endif
#endif

// Forward declarations of callback functions
void onGrindingComplete(double finalWeight, unsigned long grindTime);
void onGrindingFailed(const char* reason);

void setup() {
#if CALIBRATION_MODE
    Serial.begin(115200);
    delay(100);
    Serial.println("\nCoffee Scale Calibration Tool");
    Serial.println("----------------------------");

    // Initialize scale
    scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
    scale.set_scale();  // Initialize with default scale factor
    scale.tare();       // Reset the scale to zero

    Serial.println("\nCalibration Instructions:");
    Serial.println("1. Remove all weight from scale");
    Serial.println("2. Type 't' and press Enter to tare");
    Serial.println("3. Place a known weight on the scale");
    Serial.println("4. Enter the weight in grams (e.g., '100.0')");
    Serial.println("5. Choose whether to save calibration");
#else
    // Initialize serial communication
    Serial.begin(SERIAL_BAUD);
    while(!Serial) { delay(100); }

    Serial.println("Coffee Grinder Smart Scale Starting...");
    
    // Initialize hardware components
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(400000);
    
    delay(100); // Give I2C time to initialize
    
    setupDisplay();
    Serial.println("Display setup complete");
    
    // Test display
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB14_tr);
    centerPrintToScreen("Starting...", 35);
    u8g2.sendBuffer();
    delay(2000);
    
    // Initialize scale with default configuration
    scale.begin(DEFAULT_PIN_CONFIG, DEFAULT_SCALE_CONFIG);
    scale.onGrindingComplete(onGrindingComplete);
    scale.onGrindingFailed(onGrindingFailed);
    
    #if ENABLE_MQTT
        // Initialize MQTT if enabled
        mqtt.setup();
    #endif

    Serial.println("Initialization complete");
#endif
}

/**
 * Callback handler for grinding completion
 */
void onGrindingComplete(double finalWeight, unsigned long grindTime) {
    Serial.printf("Grinding complete: %.1fg in %.1fs\n", 
                 finalWeight, grindTime / 1000.0);
    #if ENABLE_MQTT
        char msg[64];
        snprintf(msg, sizeof(msg), "{\"weight\":%.1f,\"time\":%.1f}", 
                 finalWeight, grindTime / 1000.0);
        mqtt.publishStatus("grinding_complete");
        mqtt.publish("coffee-scale/result", msg);
    #endif
}

/**
 * Callback handler for grinding failures
 */
void onGrindingFailed(const char* reason) {
    Serial.printf("Grinding failed: %s\n", reason);
    #if ENABLE_MQTT
        mqtt.publishStatus("grinding_failed");
        mqtt.publish("coffee-scale/error", reason);
    #endif
}

/**
 * Main program loop - runs continuously after setup
 * Handles scale updates and MQTT communication
 */
void loop() {
#if CALIBRATION_MODE
    static bool waitingForWeight = true;
    
    if (Serial.available() > 0) {
        if (waitingForWeight) {
            char cmd = Serial.read();
            if (cmd == 't') {
                Serial.println("\nTaring...");
                scale.tare();
                Serial.println("Tare complete!");
                Serial.println("\nPlace known weight and enter its value in grams:");
                waitingForWeight = false;
            }
        } else {
            float knownWeight = Serial.parseFloat();
            if (knownWeight > 0) {
                // Get average reading
                float reading = 0;
                for (int i = 0; i < 10; i++) {
                    reading += scale.get_units(10);
                }
                reading /= 10.0;
                
                // Calculate calibration factor
                float newCalFactor = reading / knownWeight;
                
                Serial.print("\nNew calibration factor: ");
                Serial.println(newCalFactor);
                Serial.println("\nSave to EEPROM? (y/n)");
                
                while (!Serial.available());
                char save = Serial.read();
                if (save == 'y') {
                    EEPROM.begin(512);
                    EEPROM.put(EEPROM_CAL_ADDRESS, newCalFactor);
                    EEPROM.commit();
                    Serial.println("Calibration saved!");
                    
                    // Apply the new calibration factor
                    scale.set_scale(newCalFactor);
                } else {
                    Serial.println("Calibration not saved.");
                }
                
                Serial.println("\nCalibration complete!");
                Serial.println("Current readings:");
                waitingForWeight = true;
            }
        }
    }

    // Continuously show weight readings
    if (Serial.available() <= 0) {  // Only show readings when not waiting for input
        Serial.print("Weight: ");
        Serial.print(scale.get_units(5), 1);
        Serial.println(" g");
        delay(100);
    }
#else
    // Update scale readings and status
    scale.update();
    
    #if ENABLE_MQTT
        // Handle MQTT connection and data publishing
        mqtt.connect();
        
        // Publish weight and status periodically
        static unsigned long lastPublish = 0;
        if (millis() - lastPublish > 1000 && scale.isReady()) {
            mqtt.publishWeight(scale.getCurrentWeight());
            mqtt.publishStatus(scale.getStatusString());
            lastPublish = millis();
        }
        
        mqtt.loop();
    #endif

    // Handle any system errors
    if (ErrorHandler::hasError()) {
        Serial.println(ErrorHandler::getErrorMessage());
        delay(1000);
    }

    delay(SCALE_UPDATE_INTERVAL);
#endif
}
