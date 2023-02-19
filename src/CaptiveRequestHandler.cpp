#include "CaptiveRequestHandler.h"
#include "Utilities.h"

#include <FS.h>
#if USE_LittleFS != 0
  #include <LittleFS.h>
  #define SPIFFS LittleFS
#else
  #include <SPIFFS.h>
#endif

void CaptiveRequestHandler::handleRequest(AsyncWebServerRequest *request) {
    String cp_file = "/captive_portal.html";
    Serial.printf("WEB: %s - %s\n", request->client()->remoteIP().toString().c_str(), request->url().c_str());
    request->send(SPIFFS, cp_file, getContentType(cp_file));
}