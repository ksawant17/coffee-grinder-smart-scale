#pragma once

// Scale status enumeration
enum class ScaleStatus {
    EMPTY,
    GRINDING_IN_PROGRESS,
    GRINDING_FINISHED,
    GRINDING_FAILED,
    ERROR
};

// Scale configuration structure
struct ScaleConfig {
    double cupWeight;
    double cupDetectionTolerance;
    double coffeeTargetWeight;
    int tareMeasures;
    double significantWeightChange;
    unsigned long maxGrindingTime;
    double weightToReset;
    unsigned long tareMinInterval;
    float loadcellScaleFactor;
};

// Pin configuration structure
struct PinConfig {
    int loadcellDout;
    int loadcellSck;
    int grinderActive;
};

// Scale state structure
struct ScaleState {
    double currentWeight;
    double cupEmptyWeight;
    unsigned long lastUpdateTime;
    unsigned long lastSignificantChange;
    unsigned long lastTareTime;
    unsigned long grindStartTime;
    unsigned long grindEndTime;
    bool isReady;
    ScaleStatus status;
};
