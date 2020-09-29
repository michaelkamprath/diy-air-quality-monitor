#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "Application.h"
#include "time.h"

// the telemetry_url is a RESTful service where all the measurements collected will be POSTed 
// to as a JSON object. You could post the data to a Google Sheet, or use a simple service
// that collects and saves the JSON objects posts to as a JSON Lines formatted file. 
const char* telemetry_url = nullptr;
const char* ssid     = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;


Application::Application()
  : _sensor()
{
  // start the WiFi
  Serial.print(F("Sarting Wifi connection to SSID = "));
  Serial.print(ssid);
  Serial.print(F(" "));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.print(F("\nWiFi connected with ip address = "));
  Serial.print(WiFi.localIP());
  Serial.print(F("\n"));

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();  

  // start the sensor
  _sensor.begin();
}

Application::~Application()
{

}

void Application::printLocalTime(void)
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.print("Day of week: ");
  Serial.println(&timeinfo, "%A");
  Serial.print("Month: ");
  Serial.println(&timeinfo, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&timeinfo, "%d");
  Serial.print("Year: ");
  Serial.println(&timeinfo, "%Y");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");

  Serial.println("Time variables");
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  Serial.println(timeHour);
  char timeWeekDay[10];
  strftime(timeWeekDay,10, "%A", &timeinfo);
  Serial.println(timeWeekDay);
  Serial.println();
}

void Application::loop(void)
{
  Serial.println(F("Fetching current sensor data."));
  time_t timestamp;
  time(&timestamp);

  if (_sensor.updateSensorReading()) {
    DynamicJsonDocument doc(1024);
    doc["timestamp"] = timestamp;
    doc["mass_density"]["pm1.0"] = _sensor.PM1p0();
    doc["mass_density"]["pm2.5"] = _sensor.PM2p5();
    doc["mass_density"]["pm10"] = _sensor.PM10();
    doc["particle_count"]["0.5um"] = _sensor.particalCount0p5();
    doc["particle_count"]["1.0um"] = _sensor.particalCount1p0();
    doc["particle_count"]["2.5um"] = _sensor.particalCount2p5();
    doc["particle_count"]["5.0um"] = _sensor.particalCount5p0();
    doc["particle_count"]["7.5um"] = _sensor.particalCount7p5();
    doc["particle_count"]["10um"] = _sensor.particalCount10();
    doc["sensor_status"]["partical_detector"] = _sensor.statusParticleDetector();
    doc["sensor_status"]["laser"] = _sensor.statusLaser();
    doc["sensor_status"]["fan"] = _sensor.statusFan();


    Serial.print(F("    json payload = "));
    serializeJson(doc, Serial);
    Serial.print(F("\n"));

    if ((WiFi.status() == WL_CONNECTED) && (telemetry_url != nullptr)) {
      HTTPClient http;
      String requestBody;

      http.begin(telemetry_url);  
      http.addHeader("Content-Type", "application/json");

      serializeJson(doc, requestBody);
      int httpResponseCode = http.POST(requestBody);
      if (httpResponseCode>0) {
        String response = http.getString();

        Serial.print(F("    POSTED data to telemetry service with response code = "));                
        Serial.print(httpResponseCode);  
        Serial.print(" and response = \"");
        Serial.print(response);
        Serial.print("\"\n");
      }
    }
  }
}