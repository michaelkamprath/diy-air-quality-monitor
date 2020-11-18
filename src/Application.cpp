#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <Wire.h>
#include "time.h"
#include "Application.h"
#include "Utilities.h"

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
  : _boot_time(0),
    _last_update_time(0),
    _last_transmit_time(0),
    _sensor(AIR_QUALITY_SENSOR_UPDATE_SECONDS),
    _bme680(),
    _server(80),
#if MCU_BOARD_TYPE == MCU_TINYPICO
    _tinyPICO(),
#endif
    _loopCounter(0),
    _rootPageViewCount(0),
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
  setupLED();

  // start the WiFi
  Serial.print(F("Starting Wifi connection to SSID = "));
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

  if (!_bme680.begin(BME680_SENSOR_I2C_ADDRESS)) {
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
  _server.on("/stats", HTTP_GET, std::bind(&Application::handleStatsPageRequest, this, std::placeholders::_1));
  _server.on("/stats.html", HTTP_GET, std::bind(&Application::handleStatsPageRequest, this, std::placeholders::_1));
  _server.on("/json", HTTP_GET, std::bind(&Application::handleJsonRequest, this, std::placeholders::_1));
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

  // check to see if the URL is in the SPIFFS
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
  String root_file = "/index.html";

  Serial.printf("WEB: %s - %s\n", request->client()->remoteIP().toString().c_str(), request->url().c_str());
  request->send(SPIFFS, root_file, getContentType(root_file));
  _rootPageViewCount++;
}

void Application::handleStatsPageRequest(AsyncWebServerRequest *request)
{
  String stats_file = "/stats.html";
  Serial.printf("WEB: %s - %s\n", request->client()->remoteIP().toString().c_str(), request->url().c_str());
  request->send(SPIFFS, stats_file, getContentType(stats_file), false, std::bind(&Application::processStatsPageHTML, this, std::placeholders::_1));
}

void Application::handleJsonRequest(AsyncWebServerRequest *request)
{
  Serial.printf("WEB: %s - %s\n", request->client()->remoteIP().toString().c_str(), request->url().c_str());
  DynamicJsonDocument jsonPayload(1024);
  getJsonPayload(jsonPayload);

  String requestBody;
  serializeJson(jsonPayload, requestBody);

  request->send(200, "application/json", requestBody);
}

String Application::processStatsPageHTML(const String& var)
{
  if (var == "PERCENT") {
    return String("%");
  } else if (var == "WIFISSID") {
    return String(ssid);
  } else if (var == "IPADDRESS") {
    return WiFi.localIP().toString();
  } else if (var == "BOOTTIME") {
    return convertEpochToString(_boot_time);
  } else if (var == "LASTMEASURETIME") {
    if (_last_update_time == 0) {
      return String("None");
    }
    return convertEpochToString(_last_update_time);
  } else if (var == "LASTTRANSMIT") {
    if (_last_transmit_time == 0) {
      return String("None");
    }
    return convertEpochToString(_last_transmit_time);
  } else if (var == "HISTORYSIZE") {
    return String(_sensor.getHistoryCount());
  } else if (var == "HASBME680") {
    if (_hasBME680) {
      return String("True");
    } else {
      return String("False");
    }
  } else if (var == "MEASURERATE") {
    char s[32];
    snprintf(s, sizeof(s), "%d seconds", AIR_QUALITY_SENSOR_UPDATE_SECONDS);
    return String(s);
  } else if (var == "TRANSMITRATE") {
    char s[32];
    snprintf(s, sizeof(s), "%d seconds", AIR_QUALITY_SENSOR_UPDATE_SECONDS*AIR_QUALITY_DATA_TRANSMIT_MULTIPLE);
    return String(s);
  } else if (var == "TRANSMITURL") {
    if (telemetry_url == nullptr) {
      return String("None");
    } else {
      return String(telemetry_url);
    }
  } else if (var == "PDSTATUS") {
    return String(_sensor.statusParticleDetector());
  } else if (var == "LASERSTATUS") {
    return String(_sensor.statusLaser());
  } else if (var == "FANSTATUS") {
    return String(_sensor.statusFan());
  } else if (var == "ROOTVIEWCOUNT") {
    return String(_rootPageViewCount);
  }

  return String();
}

void Application::setupLED(void)
{
#if MCU_BOARD_TYPE == MCU_TINYPICO
  _tinyPICO.DotStar_SetPower(true);
  _tinyPICO.DotStar_Clear();
  _tinyPICO.DotStar_SetBrightness(STATUS_LED_BRIGHTNESS);
#elif MCU_BOARD_TYPE == MCU_EZSBC_IOT
// Set up the rgb led names
#define ledR  16
#define ledG  17
#define ledB  18
  // assign RGB led pins to channels
  ledcAttachPin(ledR,  1);
  ledcAttachPin(ledG,  2);
  ledcAttachPin(ledB,  3);
  // Initialize channels
  // channels 0-15, resolution 1-16 bits, freq limits depend on resolution
  // ledcSetup(uint8_t channel, uint32_t freq, uint8_t resolution_bits);
  ledcSetup(1, 12000, 8); // 12 kHz PWM, 8-bit resolution
  ledcSetup(2, 12000, 8);
  ledcSetup(3, 12000, 8);

  // clear the LED
  ledcWrite(1, 0);
  ledcWrite(2, 0);
  ledcWrite(3, 0);
#endif
}

void Application::getJsonPayload(DynamicJsonDocument &doc) const {
  time_t timestamp;
  time(&timestamp);

  float current_avg_pm2p5 = _sensor.averagePM2p5(AIR_QUALITY_SENSOR_UPDATE_SECONDS);
  float ten_minutes_avg_pm2p5 = _sensor.averagePM2p5(60*10);
  float one_hour_avg_pm2p5 = _sensor.averagePM2p5(60*60);
  float one_day_avg_pm2p5 = _sensor.averagePM2p5(60*60*24);

  doc["timestamp"] = _last_update_time;
  doc["sensor_id"] = sensor_name;
  doc["uptime"] = (timestamp - _boot_time);
  doc["has_environment_sensor"] = _hasBME680;
  doc["wifi"]["ip_address"] = WiFi.localIP().toString();
  doc["wifi"]["mac_address"] = WiFi.macAddress();
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
  doc["air_quality_index"]["aqi_10min"] =  _sensor.airQualityIndex(ten_minutes_avg_pm2p5);
  doc["air_quality_index"]["aqi_1hour"] = _sensor.airQualityIndex(one_hour_avg_pm2p5);
  doc["air_quality_index"]["aqi_24hour"] = _sensor.airQualityIndex(one_day_avg_pm2p5);
  if (_hasBME680) {
    doc["environment"]["temperature"] = _latestTemperature;
    doc["environment"]["temperature_f"] = _latestTemperature * 9.0/5.0 + 32.0;
    doc["environment"]["pressure"] = _latestPressure;
    doc["environment"]["humidity"] = _latestHumidity;
    doc["environment"]["gas_resistance"] = _bme680.gas_resistance;  // ohms
  }

  Serial.print(F("    json payload = "));
  serializeJson(doc, Serial);
  Serial.print(F("\n"));
}

void Application::setLEDColorForAQI(float aqi_value)
{
  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;

  switch (AirQualitySensor::getAQIStatusColor(aqi_value)) {
    case AQI_GREEN:
      green = 0xFF;
      break;
    case AQI_YELLOW:
      red = 0xFF;
      green = 0xFF;
      break;
    case AQI_ORANGE:
      red = 0xFF;
      green = 0x80;
      break;
    case AQI_RED:
      red = 0xFF;
      break;
    case AQI_PURPLE:
      red = 0x7F;
      blue = 0xFF;
      break;
    case AQI_MAROON:
      red = 0x80;
      break;
    default:
      break;
  }
#if MCU_BOARD_TYPE == MCU_TINYPICO
  _tinyPICO.DotStar_SetPixelColor(red, green, blue);
#elif MCU_BOARD_TYPE == MCU_EZSBC_IOT
#define CALC_LED_DUTY_CYCLE(cv) (255 - (uint32_t)cv*STATUS_LED_BRIGHTNESS/255)
  // write red component to channel 1, etc.
  ledcWrite(1, CALC_LED_DUTY_CYCLE(red));
  ledcWrite(2, CALC_LED_DUTY_CYCLE(green));
  ledcWrite(3, CALC_LED_DUTY_CYCLE(blue));
#endif
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
  if (!_sensor.updateSensorReading()) {
    return;
  }

  // check in on BME 680
  if (_hasBME680 && (bme680EndTime > 0)) {
    if (_bme680.endReading()) {
      _latestTemperature = _bme680.temperature;        // °C
      _latestPressure = _bme680.pressure / 100.0;      // hPa
      _latestHumidity = _bme680.humidity;              // %
    } else {
      Serial.println(F("    ERROR could not finish BME680 reading."));
      _latestTemperature = UNSET_ENVIRONMENT_VALUE;
      _latestPressure = UNSET_ENVIRONMENT_VALUE;
      _latestHumidity = UNSET_ENVIRONMENT_VALUE;
    }
  }
  float ten_minutes_avg_pm2p5 = _sensor.averagePM2p5(60*10);
  float aqi_10min = _sensor.airQualityIndex(ten_minutes_avg_pm2p5);
  setLEDColorForAQI(aqi_10min);

  if (  (telemetry_url == nullptr)
      ||(timestamp - _last_transmit_time) < AIR_QUALITY_SENSOR_UPDATE_SECONDS*AIR_QUALITY_DATA_TRANSMIT_MULTIPLE)
  {
    return;
  }

  _last_transmit_time = timestamp;

  if (WiFi.status() == WL_CONNECTED) {
    DynamicJsonDocument doc(1024);
    getJsonPayload(doc);

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
  } else {
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