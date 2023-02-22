#ifndef __Webserver__
#define __Webserver__
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "Configuration.h"

class Webserver {
private:
    Configuration& _config;
    AsyncWebServer _server;
    String _captivePortalResponse;

    uint32_t _rootPageViewCount;

    void handleHotspotDectect(AsyncWebServerRequest *request);
    void handleUnassignedPath(AsyncWebServerRequest *request);
    void handleRootPageRequest(AsyncWebServerRequest *request);
    void handleStatsPageRequest(AsyncWebServerRequest *request);
    void handleConfigPageRequest(AsyncWebServerRequest *request);
    void handleJsonRequest(AsyncWebServerRequest *request);
    void handSubmitConfigRequest(AsyncWebServerRequest *request);

    String processConfigPageHTML(const String& var);

    void logWebRequest(AsyncWebServerRequest *request, String* errorStr = nullptr);

public:
    Webserver(
            Configuration& config
        );
    virtual ~Webserver();

    void startCaptivePortal(const IPAddress& serverIP);
    void startNormal(void);
    void stop(void);

    uint32_t getRootViewCount(void) const       { return _rootPageViewCount; }
};

#endif //__Webserver__