// tasks.h
#ifndef TASKS_H
#define TASKS_H

#include <Arduino.h>

extern TaskHandle_t Task1;  // Hinzugefügt
extern TaskHandle_t Task2;  // Hinzugefügt

void startTasks();

#endif // TASKS_H
