// globals.cpp
#include "globals.h"
#include "constants.h"  // Hinzugefügt für STD_MAX_DEW

float currentTemperature = 0.0f;
float currentHumidity = 0.0f;
float currentDewPoint = 0.0f;
float dewpointDiff = 0.0f;
float maxDewPoint = STD_MAX_DEW;
bool fanRunning = false;
bool longrun = false;
int timeCounter = 0;
int intervalCounter = 0;
int checkHumidityCount = 0;
