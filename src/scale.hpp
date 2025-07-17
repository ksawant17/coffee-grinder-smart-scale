#pragma once

#include <SimpleKalmanFilter.h>
#include <HX711.h>
#include <MathBuffer.h>
#include "scale_types.hpp"

class Scale {
  public:
    Scale() : kalmanFilter(0.2, 0.2, 0.05), weightHistory() {}

    void begin(const PinConfig& pins, const ScaleConfig& config);
    void IRAM_ATTR update();  // Updated with IRAM_ATTR
    void tare();

    // Getters
    double getCurrentWeight() const { return state.currentWeight; }
    ScaleStatus getStatus() const { return state.status; }
    bool isReady() const { return state.isReady; }
    const ScaleState& getState() const { return state; }
    const char* getStatusString() const;
    unsigned long getLastSignificantChange() const { return state.lastSignificantChange; }
    unsigned long getLastUpdateTime() const { return state.lastUpdateTime; }
    double getCupEmptyWeight() const { return state.cupEmptyWeight; }
    unsigned long getGrindStartTime() const { return state.grindStartTime; }
    unsigned long getGrindEndTime() const { return state.grindEndTime; }

    // Event handlers
    void onGrindingComplete(void (*callback)(double finalWeight, unsigned long grindTime)) {
        grindingCompleteCallback = callback;
    }

    void onGrindingFailed(void (*callback)(const char* reason)) {
        grindingFailedCallback = callback;
    }

  private:
    HX711 loadcell;
    SimpleKalmanFilter kalmanFilter;
    ScaleState state;
    ScaleConfig config;
    PinConfig pins;
    MathBuffer<double, 100> weightHistory;

    void (*grindingCompleteCallback)(double finalWeight, unsigned long grindTime) = nullptr;
    void (*grindingFailedCallback)(const char* reason) = nullptr;

    void updateStatus();
    bool detectCup() const;
    void startGrinding();
    void stopGrinding();
    void checkGrindingProgress();
    void handleGrindingCompletion();
    void handleGrindingFailure(const char* reason);
    bool shouldAutoTare() const;
};
