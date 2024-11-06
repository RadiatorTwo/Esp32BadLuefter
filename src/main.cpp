// main.cpp
#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "config.h"
#include "constants.h"
#include "globals.h"
#include "utilities.h"
#include "io.h"
#include "sensor.h"
#include "tasks.h"
#include "web_server.h"
#include "EEPROM.h"

void setup() {
    Serial.begin(115200);
    Serial.println("Gerät startet...");

    // EEPROM initialisieren
    if (!EEPROM.begin(64)) {
        Serial.println("Fehler beim Initialisieren des EEPROMs");
        while (true) {
            delay(1000);
        }
    }

    // Einstellungen aus EEPROM lesen
    EEPROM.get(ADR_DEW, maxDewPoint);
    if (isnan(maxDewPoint) || maxDewPoint <= 0) {
        maxDewPoint = STD_MAX_DEW;
    }
    Serial.println("Max Dew Point gesetzt auf: " + String(maxDewPoint));

    // Pins initialisieren
    pinMode(FAN_PIN, OUTPUT);
    digitalWrite(FAN_PIN, LOW);

    // Sensor initialisieren
    if (!initSensor()) {
        Serial.println("Fortfahren mit Boot ohne Sensor.");
    }

    // WiFi konfigurieren
    if (!WiFi.config(LOCAL_IP, GATEWAY, SUBNET, PRIMARY_DNS, SECONDARY_DNS)) {
        Serial.println("Fehler beim Konfigurieren der statischen IP");
    }

    Serial.print("Verbinde mit ");
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.print("Verbunden mit ");
    Serial.println(WIFI_SSID);
    Serial.print("IP-Adresse: ");
    Serial.println(WiFi.localIP());

    if (!MDNS.begin(HOSTNAME)) {
        Serial.println("Fehler beim Starten des MDNS Responders!");
        while (true) {
            delay(1000);
        }
    }
    Serial.println("mDNS Responder gestartet");

    // Webserver initialisieren
    setupWebServer();

    // Tasks starten
    startTasks();
}

void loop() {
    delay(1);
}
