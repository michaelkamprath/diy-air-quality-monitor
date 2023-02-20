#include <Arduino.h>
#include <ArduinoJson.h>
#include "Webserver.h"
#include "Utilities.h"
#include "Application.h"


Webserver::Webserver(
    Configuration& config
) : _config(config),
    _server(80),
    _captivePortalResponse(),
    _rootPageViewCount(0)
{

}

Webserver::~Webserver()
{

}

void Webserver::logWebRequest(AsyncWebServerRequest *request, String* errorStr)
{
    if (errorStr == nullptr) {
        Serial.printf(
            "WEB: %s - %s - %s\n",
            request->client()->remoteIP().toString().c_str(),
            request->host().c_str(),
            request->url().c_str()
        );
    } else {
        Serial.printf(
            "WEB-ERROR (%s): %s - %s - %s\n",
            errorStr->c_str(),
            request->client()->remoteIP().toString().c_str(),
            request->host().c_str(),
            request->url().c_str()
        );
    }
}

void Webserver::startCaptivePortal(const IPAddress& serverIP)
{
    // build this string now, not later
    _captivePortalResponse = String(
        ""
        "<!DOCTYPE html><html><head>"
        "<title>DIY Air Quality Sensor Config</title>"
        "<meta http-equiv=\"Refresh\" content=\"0; url='http://")
            + toStringIp(serverIP)
            + String("/config.html'\" />"
        "</head><body></body></html>"
        );

    _server.reset();
    _server.on("/", HTTP_GET, std::bind(&Webserver::handleHotspotDectect, this, std::placeholders::_1));
    _server.on("/hotspot-detect.html", HTTP_GET, std::bind(&Webserver::handleHotspotDectect, this, std::placeholders::_1));
    _server.on("/generate_204", HTTP_GET, std::bind(&Webserver::handleHotspotDectect, this, std::placeholders::_1));
    _server.on("/config.html", HTTP_GET, std::bind(&Webserver::handleConfigPageRequest, this, std::placeholders::_1));
    _server.on("/update", HTTP_GET, std::bind(&Webserver::handSubmitConfigRequest, this, std::placeholders::_1));
    _server.onNotFound(std::bind(&Webserver::handleUnassignedPath, this, std::placeholders::_1));
    _server.begin();
}

void Webserver::startNormal(void)
{
    _server.reset();
    _server.on("/", HTTP_GET, std::bind(&Webserver::handleRootPageRequest, this, std::placeholders::_1));
    _server.on("/index.html", HTTP_GET, std::bind(&Webserver::handleRootPageRequest, this, std::placeholders::_1));
    _server.on("/stats", HTTP_GET, std::bind(&Webserver::handleStatsPageRequest, this, std::placeholders::_1));
    _server.on("/stats.html", HTTP_GET, std::bind(&Webserver::handleStatsPageRequest, this, std::placeholders::_1));
    _server.on("/json", HTTP_GET, std::bind(&Webserver::handleJsonRequest, this, std::placeholders::_1));
    _server.on("/config.html", HTTP_GET, std::bind(&Webserver::handleConfigPageRequest, this, std::placeholders::_1));
    _server.on("/update", HTTP_GET, std::bind(&Webserver::handSubmitConfigRequest, this, std::placeholders::_1));

    _server.onNotFound(std::bind(&Webserver::handleUnassignedPath, this, std::placeholders::_1));

    _server.begin();
}

void Webserver::handleHotspotDectect(AsyncWebServerRequest *request)
{
    this->logWebRequest(request);
    String apIPStr = toStringIp(WiFi.localIP());
    if (request->host().equals(apIPStr)) {
        this->handleConfigPageRequest(request);
    } else {
        // need to send a 200 response quickly so captive portal shows. Use an HTML redirect
        // to bring the user to config page.
        request->send(200, "text/html", _captivePortalResponse );
    }
}

void Webserver::handleUnassignedPath(AsyncWebServerRequest *request)
{
    String path(request->url());
    if (path.endsWith("/")) path += "index.html";

    // check to see if the URL is in the SPIFFS
    if (SPIFFS.exists(path)) {
        this->logWebRequest(request);
        request->send(SPIFFS, path, getContentType(path));
        return;
    }
    // it is truely not found. Send a 404
    String errStr("404");
    this->logWebRequest(request, &errStr);
    request->send(404, "text/plain", "Not found");
}

void Webserver::handleRootPageRequest(AsyncWebServerRequest *request)
{
  String root_file = "/index.html";

  this->logWebRequest(request);
  request->send(SPIFFS, root_file, getContentType(root_file));
  _rootPageViewCount++;
}

void Webserver::handleStatsPageRequest(AsyncWebServerRequest *request)
{
    String stats_file = "/stats.html";
    this->logWebRequest(request);
    request->send(
        SPIFFS,
        stats_file,
        getContentType(stats_file),
        false,
        std::bind(
            &Application::processStatsPageHTML,
            Application::getInstance(),
            std::placeholders::_1
        )
    );
}

void Webserver::handleConfigPageRequest(AsyncWebServerRequest *request)
{
    String config_file = "/config.html";
    this->logWebRequest(request);
    AsyncWebServerResponse* response = request->beginResponse(
        SPIFFS,
        config_file,
        getContentType(config_file),
        false,
        std::bind(
            &Webserver::processConfigPageHTML,
            this,
            std::placeholders::_1
        )
    );
    request->send(response);
}

void Webserver::handleJsonRequest(AsyncWebServerRequest *request)
{
    this->logWebRequest(request);
    DynamicJsonDocument jsonPayload(2048);
    Application::getInstance()->getJsonPayload(jsonPayload);

    String requestBody;
    serializeJson(jsonPayload, requestBody);

    request->send(200, "application/json", requestBody);
}

void Webserver::handSubmitConfigRequest(AsyncWebServerRequest *request)
{
    this->logWebRequest(request);
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam("enable-json")) {
        String check_value = request->getParam("enable-json")->value();
        if (check_value == "on") {
            this->_config.setJSONUploadEnabled(true);
        } else {
            this->_config.setJSONUploadEnabled(false);
        }
    } else {
        // if this parameter is not present, that means the checkbox has no value (not)
        this->_config.setJSONUploadEnabled(false);
    }
    Serial.printf(
        "  The JSON telemetry upload has been %s\n",
        this->_config.getJSONUploadEnabled() ? "ENABLED" : "DISABLED"
    );

    if (request->hasParam("server-url")) {
        String server_url = request->getParam("server-url")->value();
        this->_config.setServerURL(server_url);
        Serial.printf(
            "  The JSON telemetry upload URL has been set to: %s\n",
            this->_config.getServerURL().c_str()
        );
    }

    if (request->hasParam("sensor-name")) {
        String sensor_name = request->getParam("sensor-name")->value();
        this->_config.setSensorName(sensor_name);
        Serial.printf(
            "  The sensor name has been set to: %s\n",
            this->_config.getSensorName().c_str()
        );
    }

    if (request->hasParam("upload-rate")) {
        String rate_str = request->getParam("upload-rate")->value();
        int16_t rate_val = rate_str.toInt();
        this->_config.setJSONUploadRateSeconds(rate_val);
        Serial.printf(
            "  The JSON upload rate has been set to %d seconds\n",
            this->_config.getJSONUploadRateSeconds()
        );
    }

    if (request->hasParam("led-brightness")) {
        String value_str = request->getParam("led-brightness")->value();
        int16_t value = value_str.toInt();
        this->_config.setLEDBrightnessIndex(value);
        Serial.printf(
            "  The LED brightness index has been set to %d\n",
            this->_config.getLEDBrightnessIndex()
        );
        Application::getInstance()->setupLED();
    }

    bool wifi_updated = false;
    if (request->hasParam("wifi-ssid")) {
        String wifi_ssid = request->getParam("wifi-ssid")->value();
        if (!this->_config.getWifiSSID().equals(wifi_ssid)) {
            this->_config.setWiFiSSID(wifi_ssid);
            Serial.printf(
                "  The WiFi SSID has been set to: %s\n",
                this->_config.getWifiSSID().c_str()
            );
            wifi_updated = true;
        }
    }

    if (request->hasParam("wifi-password")) {
        String wifi_pw = request->getParam("wifi-password")->value();
        if (!this->_config.getWifiPassword().equals(wifi_pw)) {
            this->_config.setWiFiPassword(wifi_pw);
            Serial.printf(
                "  The WiFi password has been set to: %s\n",
                this->_config.getWifiPassword().c_str()
            );
            wifi_updated = true;
        }
    }
    // updating this flag is delayed until after all configuration is saved
    // since this is happing asynchronously and we don't want the WiFi reconnect
    // to happen before the configuration is saved.
    if (wifi_updated) {
        Application::getInstance()->resetWifiConnection();
    }

    request->redirect("/config.html");
}

String Webserver::processConfigPageHTML(const String& var)
{
    if (var == "ENABLE_CHECKED") {
        if (this->_config.getJSONUploadEnabled()) {
            return String("checked");
        } else {
            return String();
        }
    } else if (var == "SERVER_URL") {
        return this->_config.getServerURL();
    } else if (var == "SENSOR_NAME") {
        return this->_config.getSensorName();
    } else if (var == "UPLOADRATE") {
        return String(this->_config.getJSONUploadRateSeconds());
    } else if (var == "LED_OFF") {
        if (this->_config.getLEDBrightnessIndex() == 0) {
            return String("selected");
        } else {
            return String("");
        }
    } else if (var == "LED_LOW") {
        if (this->_config.getLEDBrightnessIndex() == 1) {
            return String("selected");
        } else {
            return String("");
        }
    } else if (var == "LED_MED") {
        if (this->_config.getLEDBrightnessIndex() == 2) {
            return String("selected");
        } else {
            return String("");
        }
    } else if (var == "LED_HI") {
        if (this->_config.getLEDBrightnessIndex() == 3) {
            return String("selected");
        } else {
            return String("");
        }
    } else if (var == "WIFI_SSID") {
        return this->_config.getWifiSSID();
    } else if (var == "WIFI_PASSWORD") {
        return this->_config.getWifiPassword();
    }

    return String();
}