#ifndef __Application__
#define __Application__
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AirQualitySensor.h>
#include <TinyPICO.h>

typedef enum {
    AQI_GREEN,
    AQI_YELLOW,
    AQI_ORANGE,
    AQI_RED,
    AQI_PURPLE,
    AQI_MAROON
} AQIStatusColor;

class Application {
private:
    static Application* gApp;

    time_t _boot_time;
    time_t _last_update_time;
    AirQualitySensor _sensor;
    AsyncWebServer _server;
    TinyPICO _tinyPICO;
    uint32_t _loopCounter;
    bool _appSetup;

    void printLocalTime(void);
    void setupWebserver(void);

    AQIStatusColor getAQIStatusColor(float aqi_value) const;
    void setDotStarColorForAQI(float aqi_value);

    // web handlers
    String getContentType(String filename);
    String processRootPageHTML(const String& var);
    void handleRootPageRequest(AsyncWebServerRequest *request);
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