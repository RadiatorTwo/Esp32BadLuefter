// utilities.cpp
#include <Arduino.h>  // Hinzugefügt für uint32_t
#include "utilities.h"

void secondsToHMS(int seconds, int& h, int& m, int& s) {
    uint32_t t = seconds;
    s = t % 60;
    t = (t - s) / 60;
    m = t % 60;
    t = (t - m) / 60;
    h = t;
}

float calculateDewPoint(float temperature, float humidity) {
    return temperature - ((100.0f - humidity) / 5.0f);
}
