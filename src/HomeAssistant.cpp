#include <WiFi.h>
#include <WiFiAP.h>
#include "HomeAssistant.h"
#include "Utilities.h"
#include "Application.h"

HomeAssistant::HomeAssistant(
    Configuration& config
) : _config(config),
    _wifi(),
    _client(_wifi),
    _stateTopic(),
    _commandTopic(),
    _ledBrightnessTopic(),
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
    if (_config.getMQTTEnabled()) {
        String topic_prefix = String("diy_air_quality_sensor/") + _config.getSensorID();
        _stateTopic = topic_prefix + String("/state");
        _commandTopic = topic_prefix + String("/command");
        _ledBrightnessTopic = topic_prefix + String("/led_brightness");
        reconnectClient();
        sendDeviceDiscoveryMsgs(hasBME680);
    }
}

void HomeAssistant::reconnectClient(void) {
    const char *account_ptr = _config.getMQTTAccount().length() > 0 ? _config.getMQTTAccount().c_str() : nullptr;
    const char *password_ptr = _config.getMQTTPassword().length() > 0 ? _config.getMQTTPassword().c_str() : nullptr;
    _client.setServer(
        _config.getMQTTServer().c_str(),
        _config.getMQTTPort()
    );
    Serial.print(F("Connecting to MQTT"));
    while (!_client.connected()) {
        Serial.print(".");

        if (_client.connect("DIY Air Quality Sensor", account_ptr, password_ptr)) {
            // keep alive for at least 1 hour
            _client.setKeepAlive(3600);
            Serial.println(_stateTopic);
            _client.setCallback(std::bind(&HomeAssistant::mqttCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            _client.subscribe(_commandTopic.c_str());
            _client.subscribe(_ledBrightnessTopic.c_str());
            Serial.print(F("\nConnected to MQTT with state topic = "));
            Serial.println(_stateTopic);
        } else {
            Serial.println(F("\nERROR - failed MQTT connections with state "));
            Serial.println(_client.state());
            delay(2000);
        }
    }
}

void HomeAssistant::stop(void)
{
    if (_client.connected()) {
        _client.unsubscribe(_commandTopic.c_str());
        _client.unsubscribe(_ledBrightnessTopic.c_str());
        _client.setCallback(nullptr);
        _client.disconnect();
        _stateTopic = String();
    }
}

void HomeAssistant::publishState(const String& stateJSONString)
{
    if (_config.getMQTTEnabled() && (_stateTopic.length() > 0)) {
        // first try to reconnect if needed
        if (!_client.connected()) {
            reconnectClient();
        }

        if (_client.connected()) {
            Serial.println(F("Publishing state to MQTT"));
            _client.publish(_stateTopic.c_str(), stateJSONString.c_str());
        } else {
            Serial.println(F("ERROR - failed to publish state to MQTT with connection state = "));
            Serial.println(_client.state());
            Application::getInstance()->resetMQTTConnection();
        }
    }
}

void HomeAssistant::loop()
{
    if (_config.getMQTTEnabled()) {
        if (!_client.connected()) {
            reconnectClient();
        }
        _client.loop();
    }
}

void HomeAssistant::populateDeviceInformation(DynamicJsonDocument& json)
{
    JsonObject device = json.createNestedObject("device");
    JsonArray identifiers = device.createNestedArray("identifiers");
    JsonArray connections = device.createNestedArray("connections");
    device["name"] = _config.getSensorName();
    device["model"] = "DIY Air Quality Sensor";
    device["configuration_url"] = String("http://") + WiFi.localIP().toString() + "/config.html";
#if MCU_BOARD_TYPE == MCU_TINYPICO
    device["hw_version"] = "TinyPICO";
#elif MCU_BOARD_TYPE == MCU_EZSBC_IOT
    device["hw_version"] = "EzSBC ESP32U-01";
#elif MCU_BOARD_TYPE == MCU_YD_ESP32_S3
    device["hw_version"] = "YD-ESP32-S3";
#endif
    JsonArray conn0 = connections.createNestedArray();
    conn0.add("ip");
    conn0.add(WiFi.localIP().toString());
    JsonArray conn1 = connections.createNestedArray();
    conn1.add("mac");
    conn1.add(WiFi.macAddress());
    identifiers.add(String("diy_air_quality_sensor_") + _deviceHash);
}

void HomeAssistant::sendSensorDiscoveryMessage(
    String name,
    String entity_prefix,
    String device_class,
    String icon,
    String value_template,
    String unit_of_measurement
) {
    String entity_id = entity_prefix + _deviceHash;
    String discoveryTopic = _config.getMQTTDiscoveryPrefix()
                    + String("/sensor/")
                    + entity_id
                    + String("/config");

    DynamicJsonDocument doc(1500);
    String config_str;

    doc["name"] = name;
    doc["object_id"] = entity_id;
    doc["unique_id"] =  entity_id;
    doc["state_topic"] = _stateTopic;
    doc["device_class"] = device_class;
    doc["icon"] = icon;
    doc["force_update"] = true;
    doc["value_template"] = value_template;

    if (unit_of_measurement.length() > 0) {
        doc["unit_of_measurement"] = unit_of_measurement;
    }
    populateDeviceInformation(doc);
    size_t n = serializeJson(doc, config_str);

    if (_client.publish(discoveryTopic.c_str(), config_str.c_str(), true)) {
        Serial.printf("MQTT published discovery topic for entity %s\n", entity_id.c_str());
    } else {
        Serial.print(F("ERROR - Failed to publish MQTT discovery topic. client state = "));
        Serial.println(_client.state());
    }
}

void HomeAssistant::sendLEDBrightnessDiscoveryMessage(void) {
    String entity_id = "led_brightness_" + _deviceHash;
    String discoveryTopic = _config.getMQTTDiscoveryPrefix()
                    + String("/select/")
                    + entity_id
                    + String("/config");

    DynamicJsonDocument doc(1500);
    String config_str;
    doc["name"] = "LED Brightness";
    doc["object_id"] = entity_id;
    doc["unique_id"] = entity_id;
    doc["icon"] = "mdi:led-on";
    doc["entity_category"] = "config";
    doc["command_topic"] = _ledBrightnessTopic;
    JsonArray options = doc.createNestedArray("options");
    options.add("Off");
    options.add("Low");
    options.add("Medium");
    options.add("Bright");
    doc["state_topic"] = _stateTopic;
    doc["value_template"] =  "{{ value_json.config.led_brightness}}";
    populateDeviceInformation(doc);
    size_t n = serializeJson(doc, config_str);

    if (_client.publish(discoveryTopic.c_str(), config_str.c_str(), true)) {
        Serial.printf("MQTT published discovery topic for entity %s\n", entity_id.c_str());
    } else {
        Serial.print(F("ERROR - Failed to publish MQTT discovery topic. client state = "));
        Serial.println(_client.state());
    }
}

void HomeAssistant::sendDeviceDiscoveryMsgs(bool hasBME680)
{
    sendSensorDiscoveryMessage(
            "Air Quality Index",
            "10_minute_aqi_",
            "aqi",
            "mdi:air-filter",
            "{{ value_json.air_quality_index.aqi_10min.value|round(1)|default(0) }}",
            "AQI"
        );
    sendSensorDiscoveryMessage(
            "2.5 µm Particulate Density",
            "10_minute_pm25_",
            "pm25",
            "mdi:weather-dust",
            "{{ value_json.air_quality_index.average_pm2p5_10min.value|round(2)|default(0) }}",
            "µg/m³"
        );
    sendLEDBrightnessDiscoveryMessage();
    if (hasBME680) {
        sendSensorDiscoveryMessage(
                "Temperature",
                "temperature_f_",
                "temperature",
                "mdi:thermometer",
                "{{ value_json.environment.temperature_f.value|round(1)|default(0) }}",
                 "°F"
            );
        sendSensorDiscoveryMessage(
                "Pressure",
                "pressure_",
                "pressure",
                "mdi:gauge",
                "{{ value_json.environment.pressure.value|round(2)|default(0) }}",
                 "hPa"
            );
        sendSensorDiscoveryMessage(
                "Humidity",
                "humidity_",
                "humidity",
                "mdi:cloud-percent-outline",
                "{{ value_json.environment.humidity.value|round(1)|default(0) }}",
                 "%"
            );
    }

}
void HomeAssistant::mqttCallback(char* topic, byte* message, unsigned int length) {
    Serial.print(F("MQTT: Message arrived on topic: "));
    Serial.print(topic);
    Serial.print(F(". Message: "));
    String messageTemp;

    for (int i = 0; i < length; i++) {
        Serial.print((char)message[i]);
        messageTemp += (char)message[i];
    }
    Serial.println();

    if (String(topic) == _commandTopic) {
        if (messageTemp == "reboot") {
            Serial.println(F("Rebooting device"));
            Application::getInstance()->reboot();
        } else if (messageTemp == "reconnect_mqtt") {
            Serial.println(F("Reconnecting to MQTT"));
            Application::getInstance()->resetMQTTConnection();
        } else if (messageTemp == "reconnect_wifi") {
            Serial.println(F("Reconnecting to WiFi"));
            Application::getInstance()->resetWifiConnection();
        }
    } else if (String(topic) == _ledBrightnessTopic) {
        int brightness = 0;
        if (messageTemp == "Off") {
            brightness = 0;
        } else if (messageTemp == "Low") {
            brightness = 1;
        } else if (messageTemp == "Medium") {
            brightness = 2;
        } else if (messageTemp == "Bright") {
            brightness = 3;
        }
        Serial.printf("Setting LED brightness to %d\n", brightness);
        _config.setLEDBrightnessIndex(brightness);
        Application::getInstance()->setupLED();
    }
}