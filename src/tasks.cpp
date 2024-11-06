// tasks.cpp
#include <Arduino.h>
#include "tasks.h"
#include "constants.h"
#include "globals.h"
#include "io.h"
#include "sensor.h"
#include "utilities.h"
#include "WiFi.h"
#include "config.h"

TaskHandle_t Task1;
TaskHandle_t Task2;

void checkHumidity() {
    if (fanRunning) {
        intervalCounter += (CHECK_DELAY_SENSOR / 1000);
        timeCounter += (CHECK_DELAY_SENSOR / 1000);

        if (intervalCounter >= ACT_INTERVAL) {
            digitalWrite(FAN_PIN, HIGH);
            delay(1000);
            digitalWrite(FAN_PIN, LOW);
            intervalCounter = 0;
        }

        if (longrun) {
            if (timeCounter >= LONG_RUN_MINUTES * 60) {
                longrun = false;
                fanRunning = false;
                intervalCounter = 0;
            }
        } else {
            readSensorData();
            dewpointDiff = currentTemperature - currentDewPoint;

            if (dewpointDiff > maxDewPoint) {
                longrun = false;
                fanRunning = false;
                intervalCounter = 0;
                timeCounter = 0;
            }
        }
    } else {
        readSensorData();
        dewpointDiff = currentTemperature - currentDewPoint;

        if (dewpointDiff <= maxDewPoint) {
            if (checkHumidityCount >= CHECK_COUNT) {
                activateFan();
                checkHumidityCount = 0;
            } else {
                checkHumidityCount++;
            }
        } else {
            checkHumidityCount = 0;
        }
    }
}

void checkWifiConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        WiFi.disconnect();
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
        }
    }
}

void SensorCheckTask(void* pvParameters) {
    unsigned long previousMillis = 0;

    for (;;) {
        unsigned long currentMillis = millis();

        if (currentMillis - previousMillis >= CHECK_DELAY_SENSOR) {
            previousMillis = currentMillis;
            checkHumidity();
        }
        delay(100);
    }
}

void WiFiReconnectTask(void* pvParameters) {
    unsigned long previousMillis = 0;

    for (;;) {
        unsigned long currentMillis = millis();

        if (currentMillis - previousMillis >= CHECK_DELAY_WIFI) {
            previousMillis = currentMillis;
            checkWifiConnection();
        }
        delay(100);
    }
}

void startTasks() {
    xTaskCreatePinnedToCore(
        SensorCheckTask,
        "SensorCheckTask",
        10000,
        NULL,
        1,
        &Task1,
        0);

    xTaskCreatePinnedToCore(
        WiFiReconnectTask,
        "WiFiReconnectTask",
        10000,
        NULL,
        1,
        &Task2,
        1);
}
