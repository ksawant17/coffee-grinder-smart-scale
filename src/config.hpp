#pragma once

// Feature Flags
#define ENABLE_MQTT 0  // Set to 1 to enable MQTT functionality

// WiFi Configuration
#define WIFI_SSID "IoT"
#define WIFI_PASSWORD "password"
#define MQTT_BROKER "192.168.2.201"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "coffee-scale"
#define MQTT_RETRY_INTERVAL 5000  // 5 seconds between reconnection attempts

// Scale Configuration
#define COFFEE_TARGET_WEIGHT 18.0  // Target weight in grams
#define SCALE_UPDATE_INTERVAL 50   // Scale reading interval in ms
#define SCALE_AVERAGING_SAMPLES 5   // Number of samples to average

// Display Configuration
#define DISPLAY_UPDATE_INTERVAL 100 // Display refresh rate in ms
#define DISPLAY_SLEEP_TIMEOUT 10000 // Display sleep timeout in ms

// Debug Configuration
#define DEBUG_MODE 1  // Enable/disable debug messages
#define SERIAL_BAUD 115200
