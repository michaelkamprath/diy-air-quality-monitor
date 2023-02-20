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
#include "Webserver.h"

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

    Configuration _config;
    Webserver _webServer;
    AirQualitySensor _sensor;
    Adafruit_BME680 _bme680;
#if MCU_BOARD_TYPE == MCU_TINYPICO
    TinyPICO _tinyPICO;
#elif MCU_BOARD_TYPE == MCU_YD_ESP32_S3
    CRGB _led;
#endif
    uint32_t _loopCounter;
    bool _appSetup;
    bool _hasBME680;
    float _latestTemperature;
    float _latestPressure;
    float _latestHumidity;

    DNSServer _dnsServer;
    bool _wifiCaptivePortalMode;
    bool _resetDeviceForNewWifi;

    void printLocalTime(void);
    void connectWifi(void);

    void setLEDColorForAQI(float aqi_value);

    String getAQIStatusColor(float aqi_value) const;
public:
    static Application* getInstance(void);

    Application();
    virtual ~Application();
    void setup(void);
    void loop(void);

    AirQualitySensor& sensor(void)          { return _sensor; }
    Webserver& webServer(void)              { return _webServer; }

    void getJsonPayload(DynamicJsonDocument &doc) const;
    void resetWifiConnection(void);
    void setupLED(void);

    // web handlers
    String processStatsPageHTML(const String& var);

};


#endif // __Application__