// sensor.h
#ifndef SENSOR_H
#define SENSOR_H

#include <Adafruit_BME280.h>

extern Adafruit_BME280 bme;  // Hinzugefügt

bool initSensor();
void readSensorData();

#endif // SENSOR_H
