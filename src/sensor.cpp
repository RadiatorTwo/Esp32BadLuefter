// sensor.cpp
#include "sensor.h"
#include "globals.h"
#include "constants.h"
#include "utilities.h"

Adafruit_BME280 bme;

bool initSensor() {
    bool status = bme.begin(0x76);
    if (!status) {
        Serial.println("BME280 Sensor nicht gefunden!");
        return false;
    }
    // Initiale Sensorwerte lesen zur Kalibrierung
    bme.readTemperature();
    bme.readHumidity();
    bme.readPressure() / 100.0F;
    bme.readAltitude(SEALEVELPRESSURE_HPA);
    return true;
}

void readSensorData() {
    currentTemperature = bme.readTemperature();
    currentHumidity = bme.readHumidity();
    currentDewPoint = calculateDewPoint(currentTemperature, currentHumidity);
}
