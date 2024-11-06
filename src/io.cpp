// io.cpp
#include <Arduino.h>
#include "constants.h"
#include "globals.h"

void activateFan() {
    digitalWrite(FAN_PIN, HIGH);
    delay(1000);
    digitalWrite(FAN_PIN, LOW);
    fanRunning = true;
    timeCounter = 0;
}

void resetFan() {
    longrun = false;
    fanRunning = false;
}

void longrunFan() {
    digitalWrite(FAN_PIN, HIGH);
    delay(1000);
    digitalWrite(FAN_PIN, LOW);
    longrun = true;
    fanRunning = true;
    timeCounter = 0;
}
