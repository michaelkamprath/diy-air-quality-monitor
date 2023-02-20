#ifndef __Application__
#define __Application__
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <AirQualitySensor.h>
#include <Adafruit_BME680.h>
#include "Configuration.h"

#if MCU_BOARD_TYPE == MCU_TINYPICO
#include <TinyPICO.h>
#elif MCU_BOARD_TYPE == MCU_YD_ESP32_S3
#include <FastLED.h>
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
#elif MCU_BOARD_TYPE == MCU_YD_ESP32_S3
    CRGB _led;
#endif
    uint32_t _loopCounter;
    uint32_t _rootPageViewCount;
    bool _appSetup;
    bool _hasBME680;
    float _latestTemperature;
    float _latestPressure;
    float _latestHumidity;

    Configuration _config;
    DNSServer _dnsServer;
    IPAddress _captivePortalIP;
    bool _wifiCaptivePortalMode;
    bool _resetDeviceForNewWifi;

    void printLocalTime(void);
    void setupWebserver(void);
    void setupWebserverForCapturePortal(void);
    void connectWifi(void);
    void setupLED(void);
    void setLEDColorForAQI(float aqi_value);

    // web handlers
    String processStatsPageHTML(const String& var);
    String processConfigPageHTML(const String& var);

    String getAQIStatusColor(float aqi_value) const;
    void handleRootPageRequest(AsyncWebServerRequest *request);
    void getJsonPayload(DynamicJsonDocument &doc) const;
    void handleStatsPageRequest(AsyncWebServerRequest *request);
    void handSubmitConfigRequest(AsyncWebServerRequest *request);
    void handleJsonRequest(AsyncWebServerRequest *request);

    void handleHotspotDectect(AsyncWebServerRequest *request);
    void handleSetWifiInfo(AsyncWebServerRequest *request);
public:
    static Application* getInstance(void);

    Application();
    virtual ~Application();
    void setup(void);
    void loop(void);

    AirQualitySensor& sensor(void)          { return _sensor; }
    AsyncWebServer& server(void)            { return _server; }


    void handleConfigPageRequest(AsyncWebServerRequest *request);
    void handleUnassignedPath(AsyncWebServerRequest *request);
    const IPAddress& captivePortalIP(void) const        { return this->_captivePortalIP; }

};


#endif // __Application__