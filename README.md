# Coffee Grinder Smart Scale

<p align="center">
  <img src="./assets/3d-model.png" width="300" />
</p>


A smart scale project for precise coffee grinding using an ESP32 microcontroller, load cell, and OLED display. This project helps achieve consistent coffee doses by automatically controlling a grinder based on weight measurements.

## Features

- Real-time weight measurement with noise filtering
- Automatic grinder control
- OLED display showing:
  - Current weight
  - Target weight
  - Grinding progress
  - Grinding time
- Auto-sleep after 10 seconds of inactivity
- Cup detection and tare functionality
- Error detection and handling
- Optional MQTT connectivity (currently disabled)

## Hardware Components

- ESP32 microcontroller (ESP32-C3)
- HX711 load cell amplifier
- SSD1306 128x64 OLED display
- Load cell/weight sensor
- Coffee grinder control output

## Pin Configuration

- **Display (I2C)**:
  - SDA: Default ESP32 I2C pins
  - SCL: Default ESP32 I2C pins
- **Load Cell (HX711)**:
  - DOUT: Defined in scale.hpp
  - SCK: Defined in scale.hpp
- **Grinder Control**:
  - Control Pin: Defined in scale.hpp

## Software Architecture

### Tasks
The system runs three main FreeRTOS tasks on core 1:
1. **Scale Measurement**: Continuous weight reading and filtering
2. **Scale Status**: State management and grinder control
3. **Display Update**: UI updates and user feedback

### States
- Empty/Ready
- Grinding in Progress
- Grinding Finished
- Grinding Failed (with error handling)

### Libraries Used
- U8g2: OLED display control
- HX711: Load cell ADC interface
- SimpleKalmanFilter: Weight measurement noise reduction
- MathBuffer: Custom circular buffer for weight history
- (Optional) PubSubClient: MQTT communication

## Building and Testing

This project uses PlatformIO for development and testing.

### Dependencies
All required libraries are listed in platformio.ini:
- bogde/HX711
- SimpleKalmanFilter
- U8g2
- PubSubClient (for MQTT)

### Testing
The `test` directory contains PlatformIO Test Runner configurations and project tests.

Unit Testing is a software testing method by which individual units of
source code, sets of one or more MCU program modules together with associated
control data, usage procedures, and operating procedures, are tested to
determine whether they are fit for use. Unit testing finds problems early
in the development cycle.

More information about PlatformIO Unit Testing:
- https://docs.platformio.org/en/latest/advanced/unit-testing/index.html

### Building
```bash
# Build the project
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor
```

## Usage

1. Power on the device
2. Wait for initialization ("Init..." on display)
3. Place empty cup (will be auto-detected)
4. Grinding starts automatically
5. Grinding stops when target weight is reached
6. Remove cup when done
7. Scale resets for next use

## Error Handling

The system monitors for several error conditions:
- Scale communication errors
- Cup removal during grinding
- No weight change during grinding
- Grinding timeout
- Weight threshold violations

## Future Enhancements

- Enable MQTT connectivity for logging and remote monitoring
- Add Bluetooth support for mobile app configuration
- Implement multiple coffee dose presets
- Add grinding profile support
- Implement auto-calibration routine

More info: https://besson.co/projects/coffee-grinder-smart-scale
