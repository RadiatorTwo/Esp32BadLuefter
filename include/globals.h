// globals.h
#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>

extern float currentTemperature;
extern float currentHumidity;
extern float currentDewPoint;
extern float dewpointDiff;
extern float maxDewPoint;
extern bool fanRunning;
extern bool longrun;
extern int timeCounter;
extern int intervalCounter;
extern int checkHumidityCount;

#endif // GLOBALS_H
