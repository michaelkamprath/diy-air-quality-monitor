#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <Wire.h>
#include "time.h"
#include "Application.h"
#include "Configuration.h"

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;
 
const char* telemetry_url = TELEMETRY_URL;
const char* ssid     = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// this is the sensor name that will be in the telemetry JSON
const char* sensor_name = SENSOR_NAME;

#define SEALEVELPRESSURE_HPA (1013.25)
#define UNSET_ENVIRONMENT_VALUE -301.0

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
  : _sensor(AIR_QUALITY_SENSOR_UPDATE_SECONDS),
    _bme680(),
    _server(80),
    _tinyPICO(),
    _loopCounter(0),
    _appSetup(false),
    _hasBME680(false),
    _latestTemperature(UNSET_ENVIRONMENT_VALUE),
    _latestPressure(UNSET_ENVIRONMENT_VALUE),
    _latestHumidity(UNSET_ENVIRONMENT_VALUE)
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

  if (!_bme680.begin()) {
    Serial.println(F("NOTE - Could not find BME680 sensor. Will not create additional environment readings."));
  } else {
    Serial.println(F("Found BME680 sensor"));
    _hasBME680 = true;
    _bme680.setTemperatureOversampling(BME680_OS_8X);
    _bme680.setHumidityOversampling(BME680_OS_2X);
    _bme680.setPressureOversampling(BME680_OS_4X);
    _bme680.setIIRFilterSize(BME680_FILTER_SIZE_3);
    _bme680.setGasHeater(320, 150); // 320*C for 150 ms
  }

  // start the sensor
  _sensor.begin();

  setupWebserver();

  _tinyPICO.DotStar_SetPower(true);
  _tinyPICO.DotStar_Clear();
  _tinyPICO.DotStar_SetBrightness(STATUS_LED_BRIGHTNESS);

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

  // first check if path is a file one shouldn't access directly
  if (path != "/index_bme680.html") {
    // now check to see if the URL is in the SPIFFS
    if (SPIFFS.exists(path)) {
      Serial.printf("WEB: %s - %s\n", request->client()->remoteIP().toString().c_str(), path.c_str());
      request->send(SPIFFS, path, getContentType(path));
      return;
    }
  }
  // it is truely not found. Send a 404
  Serial.printf("WEB: %s - %s - UNKNOWN PATH\n", request->client()->remoteIP().toString().c_str(), request->url().c_str());
  request->send(404, "text/plain", "Not found");
}

bool Application::showEnvironmentRootPage(void) const
{
  return (_hasBME680 && (_latestTemperature != UNSET_ENVIRONMENT_VALUE));
}

void Application::handleRootPageRequest(AsyncWebServerRequest *request)
{
  String root_file = showEnvironmentRootPage() ? "/index_bme680.html" : "/index.html";

  Serial.printf("WEB: %s - %s\n", request->client()->remoteIP().toString().c_str(), request->url().c_str());
  request->send(SPIFFS, root_file, String(), false, std::bind(&Application::processRootPageHTML, this, std::placeholders::_1));
}

String Application::processRootPageHTML(const String& var)
{
  if(var == "AQI24HOUR") {
    return String(_sensor.tenMinuteAirQualityIndex(), 1);
  } else if (var == "SENSORNAME") {
    return String(sensor_name);
  } else if (var == "COLORCLASS") {
    switch (AirQualitySensor::getAQIStatusColor(_sensor.tenMinuteAirQualityIndex())) {
      case AQI_GREEN:
        return String("aqi-green");
        break;
      case AQI_YELLOW:
        return String("aqi-yellow");
        break;
      case AQI_ORANGE:
        return String("aqi-orange");
        break;
      case AQI_RED:
        return String("aqi-red");
        break;
      case AQI_PURPLE:
        return String("aqi-purple");
        break;
      default:
      case AQI_MAROON:
        return String("aqi-maroon");
        break;
    }
  } else if (var == "TEMPERATURE") {
    float degreesF = _latestTemperature*9.0/5.0 + 32.0;
    return String(degreesF, 1);
  } else if (var == "PRESSURE") {
    return String(_latestPressure, 1);
  } else if (var == "HUMIDITY") {
    return String(_latestHumidity, 1);
  }
  return String();
}

void Application::setDotStarColorForAQI(float aqi_value)
{
  switch (AirQualitySensor::getAQIStatusColor(aqi_value)) {
    case AQI_GREEN:
      _tinyPICO.DotStar_SetPixelColor(0, 0xFF, 0);
      break;
    case AQI_YELLOW:
      _tinyPICO.DotStar_SetPixelColor(0xFF, 0xFF, 0);
      break;
    case AQI_ORANGE:
      _tinyPICO.DotStar_SetPixelColor(0xFF, 0x80, 0);
      break;
    case AQI_RED:
      _tinyPICO.DotStar_SetPixelColor(0xFF, 0, 0);
      break;
    case AQI_PURPLE:
      _tinyPICO.DotStar_SetPixelColor(0x7F, 0, 0xFF);
      break;
    case AQI_MAROON:
      _tinyPICO.DotStar_SetPixelColor(0x80, 0, 0);
      break;
    default:
      _tinyPICO.DotStar_Clear();
      break;
  }
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

  unsigned long bme680EndTime = 0;
  if (_hasBME680) {
    // Tell BME680 to begin measurement.
    bme680EndTime = _bme680.beginReading();
    if (bme680EndTime == 0) {
      Serial.println(F("    ERROR - Failed to begin BME680 reading"));
    }
  }
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

    // check in on BME 680 
    if (_hasBME680 && (bme680EndTime > 0)) {
      if (_bme680.endReading()) {
        _latestTemperature = _bme680.temperature;        // °C
        _latestPressure = _bme680.pressure / 100.0;      // hPa
        _latestHumidity = _bme680.humidity;              // %
        doc["environment"]["temperature"] = _latestTemperature;
        doc["environment"]["pressure"] = _latestPressure;
        doc["environment"]["humidity"] = _latestHumidity;
        doc["environment"]["gas_resistance"] = _bme680.gas_resistance;  // ohms
      } else {
        Serial.println(F("    ERROR could not finish BME68 reaing."));
        _latestTemperature = UNSET_ENVIRONMENT_VALUE;
        _latestPressure = UNSET_ENVIRONMENT_VALUE;
        _latestHumidity = UNSET_ENVIRONMENT_VALUE;
      }
    }

    Serial.print(F("    json payload = "));
    serializeJson(doc, Serial);
    Serial.print(F("\n"));

    // set the DOT star color
    setDotStarColorForAQI(doc["air_quality_index"]["aqi_10min"]);

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
      } else {
        Serial.printf("    ERROR when posting JSON = %d\n", httpResponseCode);
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
    } else {
      Serial.println(F("    Did not upload telemetry due to no telemetry URL being defined."));
    }
  }
}