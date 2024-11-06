// web_server.cpp
#include "web_server.h"
#include <ESPAsyncWebServer.h>
#include <Update.h>
#include "io.h"
#include "sensor.h"
#include "tasks.h"      // Hinzugefügt
#include "utilities.h"
#include "globals.h"
#include "EEPROM.h"
#include "constants.h"
#include "config.h"

// HTML-Seiten einbinden
#include "index.h"
#include "update_page.h"
#include "settings.h"
#include "redirect.h"

AsyncWebServer server(80);

void handleRoot(AsyncWebServerRequest* request) {
    request->send(200, "text/html", MAIN_page);
}

void handleGetData(AsyncWebServerRequest* request) {
    readSensorData();
    float pressure = bme.readPressure() / 100.0F;
    float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
    dewpointDiff = currentTemperature - currentDewPoint;

    String fanValue = fanRunning ? "true" : "false";
    int timeLeft = 0;

    if (fanRunning && longrun) {
        timeLeft = (LONG_RUN_MINUTES * 60) - timeCounter;
    }

    int hours, minutes, seconds;
    secondsToHMS(timeLeft, hours, minutes, seconds);

    String timeValue = String(minutes) + " Minuten " + String(seconds) + " Sekunden verbleibend";
    String data = String(currentTemperature) + " °C," + String(currentHumidity) + " %," + String(currentDewPoint) + " °C," +
                  String(pressure) + " hPa," + String(altitude) + " m," + fanValue + "," + timeValue + "," +
                  String(maxDewPoint) + " °C," + String(dewpointDiff) + " °C";

    request->send(200, "text/plain", data);
}

void handleSettings(AsyncWebServerRequest* request) {
    request->send(200, "text/html", SETTINGS_page);
}

void handleUpdate(AsyncWebServerRequest* request) {
    request->send(200, "text/html", UPDATE_page);
}

void handleActivateFan(AsyncWebServerRequest* request) {
    activateFan();
    request->send(200);
}

void handleResetFan(AsyncWebServerRequest* request) {
    resetFan();
    request->send(200);
}

void handleLongrunFan(AsyncWebServerRequest* request) {
    longrunFan();
    request->send(200);
}

void handleUpload(AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
    if (!index) {
        vTaskDelete(Task1);
        vTaskDelete(Task2);
        disableCore0WDT();
        disableCore1WDT();

        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            Update.printError(Serial);
        }
    }

    if (Update.write(data, len) != len) {
        Update.printError(Serial);
    }

    if (final) {
        if (!Update.end(true)) {
            Update.printError(Serial);
        } else {
            request->redirect("/");
            ESP.restart();
        }
    }
}

void handleSet(AsyncWebServerRequest* request) {
    if (request->hasArg("dewpoint")) {
        maxDewPoint = request->arg("dewpoint").toFloat();
        EEPROM.put(ADR_DEW, maxDewPoint);
        EEPROM.commit();
    }
    request->redirect("/settings");
}

void setupWebServer() {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/read_data", HTTP_GET, handleGetData);
    server.on("/update", HTTP_GET, handleUpdate);
    server.on("/settings", HTTP_GET, handleSettings);

    server.on("/activate_fan", HTTP_POST, handleActivateFan);
    server.on("/reset_fan", HTTP_POST, handleResetFan);
    server.on("/longrun_fan", HTTP_POST, handleLongrunFan);

    server.on("/upload", HTTP_POST, [](AsyncWebServerRequest* request) {
        request->send(200);
    }, handleUpload);

    server.on("/set", HTTP_POST, handleSet);

    server.begin();
}
