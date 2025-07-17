#include "scale.hpp"
#include <Arduino.h>

#define ABS(a) (((a) > 0) ? (a) : ((a) * -1))

// ESP32-C3 specific optimization - use IRAM for critical functions
void IRAM_ATTR Scale::update() {
    if (loadcell.wait_ready_timeout(300)) {
        // Update weight with Kalman filter
        state.currentWeight = kalmanFilter.updateEstimate(loadcell.get_units());
        state.lastUpdateTime = millis();
        state.isReady = true;
        
        // Check for significant weight changes
        if (ABS(state.currentWeight - state.cupEmptyWeight) > config.significantWeightChange) {
            state.lastSignificantChange = millis();
        }
        
        // Store in history buffer
        weightHistory.push(state.currentWeight);
        
        updateStatus();
    } else {
        state.isReady = false;
        handleGrindingFailure("Scale communication error");
    }
}

void Scale::begin(const PinConfig& pins, const ScaleConfig& config) {
    this->pins = pins;
    this->config = config;
    
    // Initialize load cell with higher gain for ESP32-C3
    loadcell.begin(pins.loadcellDout, pins.loadcellSck);
    loadcell.set_scale(config.loadcellScaleFactor);
    loadcell.set_gain(128); // Higher gain for better resolution
    
    // Configure grinder pin with ESP32 specific settings
    pinMode(pins.grinderActive, OUTPUT);
    digitalWrite(pins.grinderActive, LOW);
    
    // Initialize state
    state = {
        .currentWeight = 0,
        .cupEmptyWeight = 0,
        .lastUpdateTime = 0,
        .lastSignificantChange = 0,
        .lastTareTime = 0,
        .grindStartTime = 0,
        .grindEndTime = 0,
        .isReady = false,
        .status = ScaleStatus::EMPTY
    };

    // Initial tare
    tare();
}

void Scale::tare() {
    Serial.println("Taring scale");
    loadcell.tare(config.tareMeasures);
    state.lastTareTime = millis();
    state.cupEmptyWeight = 0;
}

void Scale::updateStatus() {
    switch (state.status) {
    case ScaleStatus::EMPTY:
        if (detectCup()) {
            startGrinding();
        } else if (shouldAutoTare()) {
            tare();
        }
        break;
        
    case ScaleStatus::GRINDING_IN_PROGRESS:
        checkGrindingProgress();
        break;
        
    case ScaleStatus::GRINDING_FINISHED:
    case ScaleStatus::GRINDING_FAILED:
        if (state.currentWeight < 5.0) { // Cup removed
            state.status = ScaleStatus::EMPTY;
        }
        break;
    }
}

bool Scale::detectCup() const {
    if (!state.isReady) return false;
    
    double weightDiff = ABS(state.currentWeight - config.cupWeight);
    return weightDiff < config.cupDetectionTolerance;
}

void Scale::startGrinding() {
    state.cupEmptyWeight = state.currentWeight;
    state.status = ScaleStatus::GRINDING_IN_PROGRESS;
    state.grindStartTime = millis();
    digitalWrite(pins.grinderActive, HIGH);
    Serial.println("Starting grinding");
}

void Scale::stopGrinding() {
    digitalWrite(pins.grinderActive, LOW);
    state.grindEndTime = millis();
}

void Scale::checkGrindingProgress() {
    unsigned long grindingTime = millis() - state.grindStartTime;
    
    // Check timeout
    if (grindingTime > config.maxGrindingTime) {
        handleGrindingFailure("Grinding timeout");
        return;
    }
    
    // Check for cup removal
    if (state.currentWeight < state.cupEmptyWeight - config.cupDetectionTolerance) {
        handleGrindingFailure("Cup removed");
        return;
    }
    
    // Check progress after initial delay
    if (grindingTime > 2000) {
        double weightChange = state.currentWeight - state.cupEmptyWeight;
        if (weightChange < 1.0) { // Less than 1g increase in 2 seconds
            handleGrindingFailure("No weight increase detected");
            return;
        }
    }
    
    // Check if target reached
    if (state.currentWeight >= state.cupEmptyWeight + config.coffeeTargetWeight) {
        handleGrindingCompletion();
    }
}

void Scale::handleGrindingCompletion() {
    stopGrinding();
    state.status = ScaleStatus::GRINDING_FINISHED;
    
    if (grindingCompleteCallback) {
        double finalWeight = state.currentWeight - state.cupEmptyWeight;
        unsigned long grindTime = state.grindEndTime - state.grindStartTime;
        grindingCompleteCallback(finalWeight, grindTime);
    }
}

void Scale::handleGrindingFailure(const char* reason) {
    stopGrinding();
    state.status = ScaleStatus::GRINDING_FAILED;
    
    if (grindingFailedCallback) {
        grindingFailedCallback(reason);
    }
}

bool Scale::shouldAutoTare() const {
    if (millis() - state.lastTareTime < config.tareMinInterval) {
        return false;
    }
    
    return ABS(state.currentWeight) > 0.2 && 
           ABS(state.currentWeight) < 3.0;
}

const char* Scale::getStatusString() const {
    switch (state.status) {
        case ScaleStatus::EMPTY:
            return "empty";
        case ScaleStatus::GRINDING_IN_PROGRESS:
            return "grinding";
        case ScaleStatus::GRINDING_FINISHED:
            return "finished";
        case ScaleStatus::GRINDING_FAILED:
            return "failed";
        default:
            return "unknown";
    }
}
