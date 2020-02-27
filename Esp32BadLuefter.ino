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

#include "index.h"
#include "update.h"
#include "redirect.h"

//===============================================================
// Tasks
//===============================================================
TaskHandle_t Task1;

//===============================================================
// Konstanten
//===============================================================
#define FAN_PIN 27
#define MAX_HUMID 65
#define COUNT_HUMID 5
#define FAN_MINUTES 10
#define ACT_INTERVAL 30
#define CHECK_DELAY 1000

#define SEALEVELPRESSURE_HPA (1013.25)

const char* host = "esp32";
const char* ssid     = "WiFi_SSID";
const char* password = "WiFi_PASSWORD";

//===============================================================
// Variablen
//===============================================================
float currentHumidity;
bool fanRunning;
int timeCounter;
int intervalCounter;
int humidityCount;

//===============================================================
// Netzwerk Konfiguration
//===============================================================
IPAddress local_IP(192, 168, 1, 10);
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 1, 2);   //optional
IPAddress secondaryDNS(192, 168, 1, 1); //optional

//===============================================================
// Funktionsklassen
//===============================================================
AsyncWebServer server(80);
Adafruit_BME280 bme;

//===============================================================
// HTML Funktionen
//===============================================================
void handleRoot(AsyncWebServerRequest *request)
{
  request->send(200, "text/html", MAIN_page);
}

void handleGetData(AsyncWebServerRequest *request)
{
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;
  float height = bme.readAltitude(SEALEVELPRESSURE_HPA);

  String fanValue;
  int timeLeft = 0;

  if (fanRunning)
  {
    fanValue = "true";
    timeLeft = (FAN_MINUTES * 60) - timeCounter;
  }
  else
  {
    fanValue = "false";
  }

  String adcValue = String(temperature) + " °C," + String(humidity) + " %," + String(pressure) + " hPa," + String(height) + " m," + fanValue + "," + String(timeLeft);
  request->send(200, "text/plain", adcValue);
}

void handleUpdate(AsyncWebServerRequest *request)
{
  request->send(200, "text/html", UPDATE_page);
}

void handleActivateFan(AsyncWebServerRequest *request)
{
  activate_fan();
  request->send(200);
}

void handleResetFan(AsyncWebServerRequest *request)
{
  reset_fan();
  request->send(200);
}

static void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  if (!index){
    Serial.println("Starte Update");
    Serial.println("Beende Task auf Core 0");
    vTaskDelete(Task1);
    disableCore0WDT();
    
    if (!Update.begin(UPDATE_SIZE_UNKNOWN))
    {
      Update.printError(Serial);
    }
    else
    {
      Serial.println("Update läuft");
    }
  }

  if (Update.write(data, len) != len)
  {
    Update.printError(Serial);
  }
  else
  {
    Serial.print("Schreibe Bytes mit Länge: ");
    Serial.println(len);
  }

  if (final) {
    if (!Update.end(true))
    {
      Update.printError(Serial);
    }
    else
    {
      Serial.println("Update erfolgreich!");
      request->redirect("/");
      ESP.restart();
    }
  }
}

//===============================================================
// Sensor/Relais Funktionen
//===============================================================
void checkHumidity()
{
  if (fanRunning)
  {
    intervalCounter = intervalCounter + 1;
    timeCounter = timeCounter + 1;

    if (intervalCounter >= ACT_INTERVAL)
    {
      digitalWrite(FAN_PIN, HIGH);
      delay(1000);
      digitalWrite(FAN_PIN, LOW);

      intervalCounter = 0;
    }

    if (timeCounter >= FAN_MINUTES * 60)
    {
      fanRunning = false;
      intervalCounter = 0;
    }
    else
    {
      int timeLeft = (FAN_MINUTES * 60) - timeCounter;
      Serial.print("Fan still running for ");
      Serial.print(timeLeft);
      Serial.println(" seconds.");
    }
  }
  else
  {
    currentHumidity = bme.readHumidity();

    if (currentHumidity >= MAX_HUMID)
    {
      //Der Lüfter soll an gehen, wenn die letzten X Messungen über dem Grenzwert sind.
      if (humidityCount >= COUNT_HUMID)
      {
        Serial.println("Activating Fan");
        humidityCount = 0;
        activate_fan();
      }
      else
      {
        humidityCount = humidityCount + 1;
      }
    }
    else
    {
      humidityCount = 0;
    }
  }
}

void activate_fan()
{
  digitalWrite(FAN_PIN, HIGH);               // GET /H turns the LED on
  delay(1000);
  digitalWrite(FAN_PIN, LOW);
  fanRunning = true;
  timeCounter = 0;
}

void reset_fan()
{
  fanRunning = false;
}

//===============================================================
// Setup
//===============================================================
void setup()
{
  Serial.begin(115200);

  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);

  unsigned status;
  fanRunning = false;
  humidityCount = 0;

  status = bme.begin(0x76);

  //Alle Werte einmal auslesen für die Kalibrierung
  bme.readTemperature();
  bme.readHumidity();
  bme.readPressure() / 100.0F;
  bme.readAltitude(SEALEVELPRESSURE_HPA);
  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
    Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(), 16);
    while (1) delay(10);
  }

  Serial.println();
  Serial.println();

  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
  {
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

  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  server.on("/", handleRoot);

  server.on("/read_data", handleGetData);

  server.on("/update", handleUpdate);

  server.on("/activate_fan", handleActivateFan);
  server.on("/reset_fan", handleResetFan);

  /*handling uploading firmware file */
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){
        request->send(200);
    }, handleUpload);

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    Task2code,   /* Task function. */
    "Task1",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task1,      /* Task handle to keep track of created task */
    0);          /* pin task to core 0 */

  delay(500);

  server.begin();
}

//===============================================================
// Sensor Check Code auf Core 0
//===============================================================
void Task2code( void * pvParameters ) {
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {
    checkHumidity();
    delay(CHECK_DELAY);
  }
}

//===============================================================
// Der Loop verarbeitet die HTTP anfragen.
//===============================================================
void loop() {
  delay(1);
}
