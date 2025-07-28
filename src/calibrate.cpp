// Calibrating the load cell
#include <Arduino.h>
#include "HX711.h"
#include <EEPROM.h>

// HX711 circuit wiring for XIAO ESP32-C3
const int LOADCELL_DOUT_PIN = 10;  
const int LOADCELL_SCK_PIN = 8;   

HX711 scale;
float calibration_factor = -1000; // This will be adjusted during calibration

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect
  }
  setCpuFrequencyMhz(80);
  
  Serial.println("\nHX711 Calibration Setup");
  Serial.println("Initializing scale...");
  
  Serial.printf("Using DOUT pin: GPIO%d (D10)\n", LOADCELL_DOUT_PIN);
  Serial.printf("Using SCK pin: GPIO%d (D8)\n", LOADCELL_SCK_PIN);
  
  // Power cycle the HX711
  pinMode(LOADCELL_SCK_PIN, OUTPUT);
  digitalWrite(LOADCELL_SCK_PIN, LOW);
  delay(100);
  
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  delay(100); // Give the scale some time to stabilize
  
  // Check if scale is found
  byte retries = 0;
  while (!scale.is_ready() && retries < 5) {
    Serial.println("HX711 not found. Retrying...");
    Serial.println("Checking DOUT pin state: " + String(digitalRead(LOADCELL_DOUT_PIN)));
    delay(1000);
    retries++;
  }
  
  if (!scale.is_ready()) {
    Serial.println("\nERROR: HX711 not responding. Please check:");
    Serial.println("1. Wiring connections:");
    Serial.printf("   - DOUT: GPIO%d (D10) → HX711 DOUT\n", LOADCELL_DOUT_PIN);
    Serial.printf("   - SCK:  GPIO%d (D8)  → HX711 SCK\n", LOADCELL_SCK_PIN);
    Serial.println("2. Power connections:");
    Serial.println("   - HX711 VCC → 3.3V");
    Serial.println("   - HX711 GND → GND");
    Serial.println("3. Check if the HX711 module is getting warm (indicating power)");
    Serial.println("4. Try disconnecting power and reconnecting");
    while (1) {
      Serial.println("DOUT pin state: " + String(digitalRead(LOADCELL_DOUT_PIN)));
      delay(1000);
    }
  }
  
  Serial.println("HX711 found successfully!");
  scale.set_scale();
  scale.tare();
  
  Serial.println("\n====== CALIBRATION INSTRUCTIONS ======");
  Serial.println("1. Make sure the scale is empty");
  Serial.println("2. Send 't' to tare/zero the scale");
  Serial.println("3. Place a known weight (in grams) on the scale");
  Serial.println("4. Send the weight value (e.g., '100' for 100g)");
  Serial.println("5. The calibration factor will be calculated");
  Serial.println("6. Send 'r' to read current weight");
  Serial.println("===================================\n");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (input == "t") {
      scale.tare();
      Serial.println("Scale tared. Current reading set to 0.");
    }
    else if (input == "r") {
      float current_weight = scale.get_units(10);
      Serial.printf("Current weight: %.2f g\n", current_weight);
    }
    else if (input.toInt() != 0) {
      float known_weight = input.toFloat();
      long raw_reading = scale.get_value(10);
      calibration_factor = raw_reading / known_weight;
      
      scale.set_scale(calibration_factor);
      
      Serial.printf("\nCalibration factor: %.2f\n", calibration_factor);
      Serial.println("Testing calibration...");
      float verified_weight = scale.get_units(10);
      Serial.printf("Measured weight: %.2f g (should be close to %.2fg)\n", verified_weight, known_weight);
      
      Serial.println("\nCalibration complete!");
      Serial.println("Send 'r' to read weight");
      Serial.println("Send 't' to tare");
    }
  }
  
  // // Check if scale is still responding
  // if (!scale.is_ready()) {
  //   Serial.println("WARNING: Scale connection lost! Check wiring.");
  //   delay(1000);
  // }
}

