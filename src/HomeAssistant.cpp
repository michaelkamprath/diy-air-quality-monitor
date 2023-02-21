#include <WiFi.h>
#include <WiFiAP.h>
#include "HomeAssistant.h"
#include "Utilities.h"


HomeAssistant::HomeAssistant(
    Configuration& config
) : _config(config),
    _wifi(),
    _client(_wifi),
    _stateTopic(String("diy_air_quality_sensor/") + _config.getSensorName() + String("/state")),
    _deviceHash()
{
    _client.setBufferSize(2048);

    byte mac[6];
    WiFi.macAddress(mac);
    _deviceHash = mac2String(mac);

}

HomeAssistant::~HomeAssistant()
{

}

void HomeAssistant::begin(bool hasBME680)
{
    stop();
    _client.setServer("video.kamprath.home", 1883);
    Serial.print(F("Connecting to MQTT"));
    while (!_client.connected()) {
        Serial.print(".");

        if (_client.connect("DIY Air Quality Sensor", nullptr, nullptr)) {
            Serial.println(F("\nConnected to MQTT"));
            sendMQTTAQIDiscoveryMsg();
        } else {
            Serial.println(F("ERROR - failed MQTT connections with state "));
            Serial.println(_client.state());
            delay(2000);
        }
    }
}

void HomeAssistant::stop(void)
{
    if (_client.connected()) {
        _client.disconnect();
    }
}

void HomeAssistant::publishState(const String& stateJSONString)
{
    if (_client.connected()) {
        Serial.println(F("Publishing state to MQTT"));
        _client.publish(_stateTopic.c_str(), stateJSONString.c_str());
    } else {
        Serial.println(F("ERROR - failed to publish state to MQTT with connection state = "));
        Serial.println(_client.state());
    }
}

void HomeAssistant::loop()
{
    _client.loop();
}

void HomeAssistant::pupulateDeviceInformation(DynamicJsonDocument& json)
{
    JsonObject device = json.createNestedObject("device");
    JsonArray identifiers = device.createNestedArray("identifiers");
    device["name"] = "DIY Air Quality Sensor";
    // TODO - add device hash to this identifier
    identifiers.add(String("diy_air_quality_sensor_") + _deviceHash);
}

void HomeAssistant::sendMQTTAQIDiscoveryMsg(void)
{
    String entity_id = String("10_minute_aqi_") + _deviceHash;
    String discoveryTopic = String("homeassistant/sensor/") + entity_id + String("/config");

    DynamicJsonDocument doc(1024);
    String config_str;

    doc["name"] = "10 Minute Air Quality Index";
    doc["object_id"] = entity_id;
    doc["unique_id"] =  entity_id;
    doc["state_topic"] = _stateTopic;
    doc["device_class"] = "aqi";
    doc["force_update"] = true;
    doc["value_template"] = "{{ value_json.air_quality_index.average_pm2p5_10min.value|default(0) }}";
    pupulateDeviceInformation(doc);


    size_t n = serializeJson(doc, config_str);

    if (_client.publish(discoveryTopic.c_str(), config_str.c_str(), true)) {
        Serial.println(F("MQTT published discovery topic"));
    } else {
        Serial.print(F("ERROR - Failed to publish MQTT discovery topic. client state = "));
        Serial.println(_client.state());
    }
}

void HomeAssistant::sendMQTTTempDiscoveryMsg(void)
{

    String entity_id = String("temperature_f_") + _deviceHash;
    String discoveryTopic = String("homeassistant/sensor/") + entity_id + String("/config");

    DynamicJsonDocument doc(1024);
    String config_str;

    doc["name"] = "10 Minute Air Quality Index";
    doc["object_id"] = entity_id;
    doc["unique_id"] =  entity_id;
    doc["state_topic"] = _stateTopic;
    doc["device_class"] = "temperature";
    doc["unit_of_measurement"] = "°F";
    doc["force_update"] = true;
    doc["value_template"] = "{{ value_json.environment.temperature_f.value|default(0) }}";
    pupulateDeviceInformation(doc);

    size_t n = serializeJson(doc, config_str);

    if (_client.publish(discoveryTopic.c_str(), config_str.c_str(), true)) {
        Serial.println(F("MQTT published discovery topic"));
    } else {
        Serial.print(F("ERROR - Failed to publish MQTT discovery topic. client state = "));
        Serial.println(_client.state());
    }
}