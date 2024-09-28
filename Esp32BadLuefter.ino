//===============================================================
// Includes
//===============================================================
#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include "EEPROM.h"
#include "index.h"
#include "update_page.h"
#include "redirect.h"
#include "settings.h"

//===============================================================
// Tasks
//===============================================================
TaskHandle_t Task1;
TaskHandle_t Task2;

//===============================================================
// Konstanten
//===============================================================
#define FAN_PIN 27
#define CHECK_COUNT 3
#define STD_MAX_DEW (20.0)
#define ACT_INTERVAL 30
#define CHECK_DELAY_SENSOR 2000
#define CHECK_DELAY_WIFI 10000
#define LONG_RUN_MINUTES 60
#define SEALEVELPRESSURE_HPA (1013.25)

#define ADR_DEW 0

const char *host = "BadLuefter";
const char *ssid = "***REMOVED***";
const char *password = "***REMOVED***";

//===============================================================
// Variablen
//===============================================================
float currentTemperature;
float currentHumidity;
float currentDewPoint;
bool fanRunning;
int timeCounter;
int intervalCounter;
int checkHumidityCount;
bool longrun;
int maxDewPoint;
float dewpointDiff;

//===============================================================
// Netzwerk Konfiguration
//===============================================================
IPAddress local_IP(192, 168, 1, 10);
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 1, 2);    //optional
IPAddress secondaryDNS(192, 168, 1, 1);  //optional

//===============================================================
// Funktionsklassen
//===============================================================
AsyncWebServer server(80);
Adafruit_BME280 bme;

//===============================================================
// Hilfsfunktionen
//===============================================================
void secondsToHMS(int seconds, int &h, int &m, int &s) {
  uint32_t t = seconds;
  s = t % 60;
  t = (t - s) / 60;
  m = t % 60;
  t = (t - m) / 60;
  h = t;
}

//===============================================================
// WiFi Funktionen
//===============================================================
void checkWifiConnection() {
  // Falls keine Verbindung, versuchen wiederherzustellen.
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Reconnecting...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);

    // Wait for reconnection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("\nReconnected to the WiFi network");
  } else {
    Serial.println("Wifi connected");
  }
}

//===============================================================
// HTML Funktionen
//===============================================================
void handleRoot(AsyncWebServerRequest *request) {
  request->send(200, "text/html", MAIN_page);
}

void handleGetData(AsyncWebServerRequest *request) {
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float maxdewpoint = EEPROM.read(ADR_DEW);
  float dewpoint = calculateDewPoint(temperature, humidity);
  float dewpointdiff = temperature - dewpoint;
  float pressure = bme.readPressure() / 100.0F;
  float height = bme.readAltitude(SEALEVELPRESSURE_HPA);

  String fanValue;
  int timeLeft = 0;

  if (fanRunning) {
    if (longrun) {
      fanValue = "true";
      timeLeft = (LONG_RUN_MINUTES * 60) - timeCounter;
    } else {
      timeLeft = 0;
    }
  } else {
    fanValue = "false";
  }

  int hours;
  int minutes;
  int seconds;

  secondsToHMS(timeLeft, hours, minutes, seconds);

  String timeValue = String(minutes) + " Minuten " + String(seconds) + " Sekunden verbleibend";
  String adcValue = String(temperature) + " °C," + String(humidity) + " %," + String(dewpoint) + " °C," + String(pressure) + " hPa," + String(height) + " m," + fanValue + "," + timeValue + "," + String(maxdewpoint) + " °C," + dewpointdiff + " °C";
  request->send(200, "text/plain", adcValue);
}

void handleSettings(AsyncWebServerRequest *request) {
  request->send(200, "text/html", SETTINGS_page);
}

void handleUpdate(AsyncWebServerRequest *request) {
  request->send(200, "text/html", UPDATE_page);
}

void handleActivateFan(AsyncWebServerRequest *request) {
  activate_fan();
  request->send(200);
}

void handleResetFan(AsyncWebServerRequest *request) {
  reset_fan();
  request->send(200);
}

void handleLongrunFan(AsyncWebServerRequest *request) {
  longrun_fan();
  request->send(200);
}

static void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index) {
    Serial.println("Starte Update");
    Serial.println("Beende Task auf Core 0");
    vTaskDelete(Task1);
    Serial.println("Beende Task auf Core 1");
    vTaskDelete(Task2);
    disableCore0WDT();
    disableCore1WDT();

    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
    } else {
      Serial.println("Update läuft");
    }
  }

  if (Update.write(data, len) != len) {
    Update.printError(Serial);
  } else {
    Serial.print("Schreibe Bytes mit Länge: ");
    Serial.println(len);
  }

  if (final) {
    if (!Update.end(true)) {
      Update.printError(Serial);
    } else {
      Serial.println("Update erfolgreich!");
      request->redirect("/");
      ESP.restart();
    }
  }
}

//===============================================================
// Sensor/Relais Funktionen
//===============================================================
void checkHumidity() {
  Serial.println("Checking Humidity levels");

  if (fanRunning) {
    intervalCounter = intervalCounter + (CHECK_DELAY_SENSOR / 1000);
    timeCounter = timeCounter + (CHECK_DELAY_SENSOR / 1000);

    if (intervalCounter >= ACT_INTERVAL) {
      Serial.println("Fan Keep Alive");
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
      } else {
        int timeLeft = (LONG_RUN_MINUTES * 60) - timeCounter;
        Serial.print("Fan still running for ");
        Serial.print(timeLeft);
        Serial.println(" seconds.");
      }
    } else {
      currentTemperature = bme.readTemperature();
      currentHumidity = bme.readHumidity();
      currentDewPoint = calculateDewPoint(currentTemperature, currentHumidity);
      dewpointDiff = currentTemperature - currentDewPoint;

      if (dewpointDiff > maxDewPoint) {
        longrun = false;
        fanRunning = false;
        intervalCounter = 0;
        timeCounter = 0;
      }
    }
  } else {
    currentTemperature = bme.readTemperature();
    currentHumidity = bme.readHumidity();
    currentDewPoint = calculateDewPoint(currentTemperature, currentHumidity);
    dewpointDiff = currentTemperature - currentDewPoint;

    if (dewpointDiff <= maxDewPoint) {
      //Der Lüfter soll an gehen, wenn die letzten X Messungen über dem Grenzwert sind.
      if (checkHumidityCount >= CHECK_COUNT) {
        Serial.println("Activating Fan");
        checkHumidityCount = 0;
        activate_fan();
      } else {
        checkHumidityCount = checkHumidityCount + 1;
      }
    } else {
      checkHumidityCount = 0;
    }
  }
}

void activate_fan() {
  digitalWrite(FAN_PIN, HIGH);  // GET /H turns the LED on
  delay(1000);
  digitalWrite(FAN_PIN, LOW);
  fanRunning = true;
  timeCounter = 0;
}

void reset_fan() {
  longrun = false;
  fanRunning = false;
}

void longrun_fan() {
  digitalWrite(FAN_PIN, HIGH);  // GET /H turns the LED on
  delay(1000);
  digitalWrite(FAN_PIN, LOW);
  longrun = true;
  fanRunning = true;
  timeCounter = 0;
}

float calculateDewPoint(float temperature, float humidity) {
  return temperature - ((100 - humidity) / 5);
}

//===============================================================
// Setup
//===============================================================
void setup() {
  Serial.begin(115200);

  Serial.println("Starting Device");

  Serial.println("Init EEPROM");
  if (!EEPROM.begin(64))  // size in Byte
  {
    Serial.println("failed to initialise EEPROM. Stopped");
    delay(1000000);
  }

  Serial.println("Reading Settings from EEPROM");
  maxDewPoint = EEPROM.read(ADR_DEW);

  if (maxDewPoint <= 0) {
    maxDewPoint = STD_MAX_DEW;
  }

  Serial.println("Set Max Humidity to: " + String(maxDewPoint));

  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);

  unsigned status;
  longrun = false;
  fanRunning = false;
  checkHumidityCount = 0;

  status = bme.begin(0x76);

  //Alle Werte einmal auslesen für die Kalibrierung
  bme.readTemperature();
  bme.readHumidity();
  bme.readPressure() / 100.0F;
  bme.readAltitude(SEALEVELPRESSURE_HPA);

  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
    Serial.println("SensorID was: 0x");
    Serial.println(bme.sensorID(), 16);
    Serial.println("Continuing with boot");
  }

  Serial.println();
  Serial.println();

  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure static IP address");
  }

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (!MDNS.begin(host)) {  //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }

  Serial.println("mDNS responder started");

  server.on("/", handleRoot);

  server.on("/read_data", handleGetData);

  server.on("/update", handleUpdate);
  server.on("/settings", handleSettings);

  server.on("/activate_fan", handleActivateFan);
  server.on("/reset_fan", handleResetFan);
  server.on("/longrun_fan", handleLongrunFan);

  /*handling uploading firmware file */
  server.on(
    "/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
      request->send(200);
    },
    handleUpload);

  server.on("/set", HTTP_POST, [](AsyncWebServerRequest *request) {
    Serial.println("SetFunctionCalled");
    if (request->hasArg("dewpoint")) {
      Serial.println("Param found: dewpoint");
      maxDewPoint = request->arg("dewpoint").toInt();
      EEPROM.write(ADR_DEW, maxDewPoint);
      EEPROM.commit();
      Serial.println("Set DewPoint Value: " + String(maxDewPoint));
    }

    request->redirect("/settings");
  });

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    SensorCheckTask,   /* Task function. */
    "SensorCheckTask", /* name of task. */
    10000,             /* Stack size of task */
    NULL,              /* parameter of the task */
    1,                 /* priority of the task */
    &Task1,            /* Task handle to keep track of created task */
    0);                /* pin task to core 0 */

  xTaskCreatePinnedToCore(
    WiFiReconnectTask,   /* Task function. */
    "WiFiReconnectTask", /* name of task. */
    10000,               /* Stack size of task */
    NULL,                /* parameter of the task */
    1,                   /* priority of the task */
    &Task2,              /* Task handle to keep track of created task */
    1);                  /* pin task to core 0 */

  delay(500);

  server.begin();
}

//===============================================================
// Sensor Check Code auf Core 0
//===============================================================
void SensorCheckTask(void *pvParameters) {
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

//===============================================================
// Wifi connection Check Code auf Core 1
//===============================================================
void WiFiReconnectTask(void *pvParameters) {
  unsigned long previousMillis = 0;

  for (;;) {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= CHECK_DELAY_WIFI) {
      previousMillis = currentMillis;

      checkWifiConnection();
    }

    // Add a small delay to prevent the task from hogging the CPU
    delay(100);
  }
}

//===============================================================
// Der Loop verarbeitet die HTTP anfragen.
//===============================================================
void loop() {
  delay(1);
}
