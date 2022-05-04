#ifndef __Application__
#define __Application__
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <AirQualitySensor.h>
#include <Adafruit_BME680.h>
#include "Configuration.h"

#if MCU_BOARD_TYPE == MCU_TINYPICO
#include <TinyPICO.h>
#endif


class Application {
private:
    static Application* gApp;

    time_t _boot_time;
    time_t _last_update_time;
    time_t _last_transmit_time;
    time_t _last_wifi_reconnect_time;

    AirQualitySensor _sensor;
    Adafruit_BME680 _bme680;
    AsyncWebServer _server;
#if MCU_BOARD_TYPE == MCU_TINYPICO
    TinyPICO _tinyPICO;
#endif
    uint32_t _loopCounter;
    uint32_t _rootPageViewCount;
    bool _appSetup;
    bool _hasBME680;
    float _latestTemperature;
    float _latestPressure;
    float _latestHumidity;

    void printLocalTime(void);
    void setupWebserver(void);

    void setupLED(void);
    void setLEDColorForAQI(float aqi_value);

    // web handlers
    String getContentType(String filename);
    String processStatsPageHTML(const String& var);

    String getAQIStatusColor(float aqi_value) const;
    void handleRootPageRequest(AsyncWebServerRequest *request);
    void getJsonPayload(DynamicJsonDocument &doc) const;
    void handleStatsPageRequest(AsyncWebServerRequest *request);
    void handleJsonRequest(AsyncWebServerRequest *request);
    void handleUnassignedPath(AsyncWebServerRequest *request);
public:
    static Application* getInstance(void);

    Application();
    virtual ~Application();
    void setup(void);
    void loop(void);

    AirQualitySensor& sensor(void)          { return _sensor; }
    AsyncWebServer& server(void)            { return _server; }
};


#endif // __Application__