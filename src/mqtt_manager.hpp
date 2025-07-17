#pragma once

#include <WiFi.h>
#include <PubSubClient.h>
#include "config.hpp"
#include "error_handler.hpp"

class MQTTManager {
public:
    MQTTManager() : client(wifiClient) {
        lastReconnectAttempt = 0;
    }

    void setup() {
        client.setServer(MQTT_BROKER, MQTT_PORT);
    }

    void connect() {
        if (!WiFi.isConnected()) {
            setupWiFi();
        }

        if (!client.connected()) {
            long now = millis();
            if (now - lastReconnectAttempt > MQTT_RETRY_INTERVAL) {
                lastReconnectAttempt = now;
                if (reconnect()) {
                    lastReconnectAttempt = 0;
                }
            }
        }
    }

    void publishWeight(float weight) {
        if (client.connected()) {
            char msg[10];
            snprintf(msg, sizeof(msg), "%.1f", weight);
            client.publish("coffee-scale/weight", msg);
        }
    }

    void publishStatus(const char* status) {
        if (client.connected()) {
            client.publish("coffee-scale/status", status);
        }
    }

    void publish(const char* topic, const char* message) {
        if (client.connected()) {
            client.publish(topic, message);
        }
    }

    void loop() {
        if (client.connected()) {
            client.loop();
        }
    }

private:
    WiFiClient wifiClient;
    PubSubClient client;
    long lastReconnectAttempt;

    void setupWiFi() {
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            attempts++;
        }

        if (WiFi.status() != WL_CONNECTED) {
            ErrorHandler::handleError(ErrorCode::WIFI_CONNECTION_FAILED);
        }
    }

    bool reconnect() {
        if (client.connect(MQTT_CLIENT_ID)) {
            return true;
        } else {
            ErrorHandler::handleError(ErrorCode::MQTT_CONNECTION_FAILED);
            return false;
        }
    }
};
