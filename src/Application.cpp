#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "time.h"
#include "Application.h"

// the telemetry_url is a RESTful service where all the measurements collected will be POSTed 
// to as a JSON object. You could post the data to a Google Sheet, or use a simple service
// that collects and saves the JSON objects posts to as a JSON Lines formatted file. 
const char* telemetry_url = nullptr;
const char* ssid     = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;

// this is the sensor name that will be in the telemetry JSON
const char* sensor_name = "YOUR_SENSORNAME";

//
// Application
//

Application* Application::gApp = nullptr;

Application* Application::getInstance(void)
{
  if (gApp == nullptr) {
    gApp = new Application();
  }

  return gApp;
}
Application::Application()
  : _sensor(),
    _server(80),
    _loopCounter(0),
    _appSetup(false)
{

}

Application::~Application()
{

}

void Application::setup(void)
{
  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("ERROR: Could not mount SPIFFs");
    return;
  }

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
  time(&_boot_time);

  // start the sensor
  _sensor.begin();

  setupWebserver();

  _appSetup = true;
}
void Application::printLocalTime(void)
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void Application::setupWebserver(void)
{
  _server.on("/", HTTP_GET, std::bind(&Application::handleRootPageRequest, this, std::placeholders::_1));
  _server.on("/index.html", HTTP_GET, std::bind(&Application::handleRootPageRequest, this, std::placeholders::_1));
  _server.onNotFound(std::bind(&Application::handleUnassignedPath, this, std::placeholders::_1));

  _server.begin();
}

String Application::getContentType(String filename)
{
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

void Application::handleUnassignedPath(AsyncWebServerRequest *request)
{
  String path(request->url());
  if (path.endsWith("/")) path += "index.html";

  // first check to see if the URL is in the SPIFFS
  if (SPIFFS.exists(path)) {
    Serial.printf("WEB: %s - %s\n", request->client()->remoteIP().toString().c_str(), path.c_str());
    request->send(SPIFFS, path, getContentType(path));
    return;
  }

  // it is truely not found. Send a 404
  Serial.printf("WEB: %s - %s - UNKNOWN PATH\n", request->client()->remoteIP().toString().c_str(), request->url().c_str());
  request->send(404, "text/plain", "Not found");
}

void Application::handleRootPageRequest(AsyncWebServerRequest *request)
{
  Serial.printf("WEB: %s - %s\n", request->client()->remoteIP().toString().c_str(), request->url().c_str());
  request->send(SPIFFS, "/index.html", String(), false, std::bind(&Application::processRootPageHTML, this, std::placeholders::_1));
}

String Application::processRootPageHTML(const String& var)
{
  if(var == "AQI24HOUR") {
    return String(Application::getInstance()->sensor().tenMinuteAirQualityIndex(), 1);
  } else if (var == "SENSORNAME") {
    return String(sensor_name);
  } else if (var == "COLORCLASS") {
    float aqi = Application::getInstance()->sensor().tenMinuteAirQualityIndex();
    if (aqi <= 50) {
      return String("aqi-green");
    } else if (aqi <= 100) {
      return String("aqi-yellow");
    } else if (aqi <= 150) {
      return String("aqi-orange");
    } else if (aqi <= 200) {
      return String("aqi-red");
    } else if (aqi <= 300) {
      return String("aqi-purple");
    } else {
      return String("aqi-maroon");
    }
  }
  return String();
}

void Application::loop(void)
{
  // slow down loop calls for the sensor
  _loopCounter++;
  if (_loopCounter%1000 != 0) {
    return;
  }

  time_t timestamp;
  time(&timestamp);

  if ((timestamp - _last_update_time) < AIR_QUALITY_SENSOR_UPDATE_SECONDS) {
    return;
  }

  Serial.println(F("Fetching current sensor data."));
  _last_update_time = timestamp;

  if (_sensor.updateSensorReading()) {
    float current_avg_pm2p5 = _sensor.averagePM2p5(AIR_QUALITY_SENSOR_UPDATE_SECONDS);
    float ten_minutes_avg_pm2p5 = _sensor.averagePM2p5(60*10);
    float one_hour_avg_pm2p5 = _sensor.averagePM2p5(60*60);
    float one_day_avg_pm2p5 = _sensor.averagePM2p5(60*60*24);

    DynamicJsonDocument doc(1024);
    doc["timestamp"] = timestamp;
    doc["sensor_id"] = sensor_name;
    doc["uptime"] = (timestamp - _boot_time);
    doc["mass_density"]["pm1p0"] = _sensor.PM1p0();
    doc["mass_density"]["pm2p5"] = _sensor.PM2p5();
    doc["mass_density"]["pm10"] = _sensor.PM10();
    doc["particle_count"]["0p5um"] = _sensor.particalCount0p5();
    doc["particle_count"]["1p0um"] = _sensor.particalCount1p0();
    doc["particle_count"]["2p5um"] = _sensor.particalCount2p5();
    doc["particle_count"]["5p0um"] = _sensor.particalCount5p0();
    doc["particle_count"]["7p5um"] = _sensor.particalCount7p5();
    doc["particle_count"]["10um"] = _sensor.particalCount10();
    doc["sensor_status"]["partical_detector"] = _sensor.statusParticleDetector();
    doc["sensor_status"]["laser"] = _sensor.statusLaser();
    doc["sensor_status"]["fan"] = _sensor.statusFan();
    doc["air_quality_index"]["average_pm2p5_current"] = current_avg_pm2p5;
    doc["air_quality_index"]["average_pm2p5_10min"] = ten_minutes_avg_pm2p5;
    doc["air_quality_index"]["average_pm2p5_1hour"] = one_hour_avg_pm2p5;
    doc["air_quality_index"]["average_pm2p5_24hour"] = one_day_avg_pm2p5;
    doc["air_quality_index"]["aqi_current"] = _sensor.airQualityIndex(current_avg_pm2p5);
    doc["air_quality_index"]["aqi_10min"] = _sensor.airQualityIndex(ten_minutes_avg_pm2p5);
    doc["air_quality_index"]["aqi_1hour"] = _sensor.airQualityIndex(one_hour_avg_pm2p5);
    doc["air_quality_index"]["aqi_24hour"] = _sensor.airQualityIndex(one_day_avg_pm2p5);

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
        response.trim();

        Serial.print(F("    POSTED data to telemetry service with response code = "));                
        Serial.print(httpResponseCode);  
        Serial.print(" and response = \"");
        Serial.print(response);
        Serial.print("\"\n");
      }
    } else if (telemetry_url != nullptr) {
      Serial.print(F("    ERROR - WiFi status is "));
      Serial.print(WiFi.status());
      Serial.print(F(", attempting to reconnect."));
      if (WiFi.reconnect()) {
        Serial.print(F("    WiFi reconnected with IP address = "));
        Serial.print(WiFi.localIP());
        Serial.print(F("\n"));
      } else {
        Serial.println(F("    ERROR - failed to reconnect WiFi."));
      }
    }
  }
}