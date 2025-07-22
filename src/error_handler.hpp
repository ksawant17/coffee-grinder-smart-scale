#pragma once

#include <Arduino.h>

// Define DEBUG_MODE as 1 to enable debug output, or 0 to disable
#ifndef DEBUG_MODE
#define DEBUG_MODE 1
#endif

enum class ErrorCode {
    NONE,
    SCALE_NOT_FOUND,
    SCALE_READ_ERROR,
    WIFI_CONNECTION_FAILED,
    MQTT_CONNECTION_FAILED,
    GRINDER_ERROR,
    CUP_REMOVED,
    TIMEOUT
};

class ErrorHandler {
public:
    static void handleError(ErrorCode error);
    static bool hasError() { return currentError != ErrorCode::NONE; }
    static ErrorCode getCurrentError() { return currentError; }
    static const char* getErrorMessage();
    static void clearError() { currentError = ErrorCode::NONE; }

private:
    static ErrorCode currentError;
};

// Implementation
ErrorCode ErrorHandler::currentError = ErrorCode::NONE;

void ErrorHandler::handleError(ErrorCode error) {
    currentError = error;
    if (DEBUG_MODE) {
        Serial.print("Error: ");
        Serial.println(getErrorMessage());
    }
}

const char* ErrorHandler::getErrorMessage() {
    switch (currentError) {
        case ErrorCode::SCALE_NOT_FOUND:
            return "Scale not found";
        case ErrorCode::SCALE_READ_ERROR:
            return "Scale read error";
        case ErrorCode::WIFI_CONNECTION_FAILED:
            return "WiFi connection failed";
        case ErrorCode::MQTT_CONNECTION_FAILED:
            return "MQTT connection failed";
        case ErrorCode::GRINDER_ERROR:
            return "Grinder error";
        case ErrorCode::CUP_REMOVED:
            return "Cup removed during grinding";
        case ErrorCode::TIMEOUT:
            return "Operation timeout";
        default:
            return "No error";
    }
}
