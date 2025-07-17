// Core Arduino library for basic functionality
#include <Arduino.h>
// WiFi library for ESP32 network connectivity
#include <WiFi.h>
// MQTT client library for message queuing and telemetry
#include <PubSubClient.h>

#include "config.hpp"
#include "display.hpp"
#include "scale.hpp"
#include "scale_config.hpp"
#include "error_handler.hpp"
#include "mqtt_manager.hpp"

// Global objects
Scale scale;
MQTTManager mqtt;

// Forward declarations of callback functions
void onGrindingComplete(double finalWeight, unsigned long grindTime);
void onGrindingFailed(const char* reason);

/**
 * Initial setup function - runs once at startup
 * Initializes all system components
 */
void setup() {
    // Initialize serial communication
    Serial.begin(SERIAL_BAUD);
    while(!Serial) { delay(100); }

    Serial.println("Coffee Grinder Smart Scale Starting...");
    
    // Initialize hardware components
    setupDisplay();
    
    // Initialize scale with default configuration
    scale.begin(DEFAULT_PIN_CONFIG, DEFAULT_SCALE_CONFIG);
    scale.onGrindingComplete(onGrindingComplete);
    scale.onGrindingFailed(onGrindingFailed);
    
    #if defined(ENABLE_MQTT)
    // Initialize MQTT if enabled
    mqtt.setup();
    #endif

    Serial.println("Initialization complete");
}

/**
 * Callback handler for grinding completion
 */
void onGrindingComplete(double finalWeight, unsigned long grindTime) {
    Serial.printf("Grinding complete: %.1fg in %.1fs\n", 
                 finalWeight, grindTime / 1000.0);
    #if defined(ENABLE_MQTT)
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
    #if defined(ENABLE_MQTT)
    mqtt.publishStatus("grinding_failed");
    mqtt.publish("coffee-scale/error", reason);
    #endif
}

/**
 * Main program loop - runs continuously after setup
 * Handles scale updates and MQTT communication
 */
void loop() {
    // Update scale readings and status
    scale.update();
    
    #if defined(ENABLE_MQTT)
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
}
