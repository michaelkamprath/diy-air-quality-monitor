#include <EEPROM.h>
#include <WiFi.h>
#include "Configuration.h"
#include "Utilities.h"


const uint32_t EEPROM_SIGNATURE = 0xC0DEB009;
#define EEPROM_SIGNATURE_SIZE sizeof(uint32_t)
#define EEPROM_SIGNATURE_INDEX 0

#define UPLOAD_ENABLE_SIZE sizeof(uint8_t)
#define UPLOAD_ENABLE_INDEX (EEPROM_SIGNATURE_SIZE)

#define DEVICE_NAME_SIZE sizeof(char)*33
#define DEVICE_NAME_INDEX (EEPROM_SIGNATURE_SIZE+UPLOAD_ENABLE_SIZE)

#define UPLOAD_URL_SIZE sizeof(char)*257
#define UPLOAD_URL_INDEX (EEPROM_SIGNATURE_SIZE+UPLOAD_ENABLE_SIZE+DEVICE_NAME_SIZE)

#define UPLOAD_RATE_SIZE sizeof(uint16_t)
#define UPLOAD_RATE_INDEX (UPLOAD_URL_INDEX+UPLOAD_URL_SIZE)

#define LED_BRIGHTNESS_SIZE sizeof(uint8_t)
#define LED_BRIGHTNESS_INDEX (UPLOAD_RATE_INDEX+UPLOAD_RATE_SIZE)

#define WIFI_SSID_SIZE sizeof(char)*33      // max SSID length is 32 + 1 for terminating null
#define WIFI_SSID_INDEX (LED_BRIGHTNESS_INDEX+LED_BRIGHTNESS_SIZE)

#define WIFI_PASSWORD_SIZE sizeof(char)*65
#define WIFI_PASSWORD_INDEX (WIFI_SSID_INDEX+WIFI_SSID_SIZE)

#define MQTT_ENABLE_SIZE sizeof(uint8_t)
#define MQTT_ENABLE_INDEX (WIFI_PASSWORD_INDEX+WIFI_PASSWORD_SIZE)

#define MQTT_SERVER_SIZE sizeof(char)*129
#define MQTT_SERVER_INDEX (MQTT_ENABLE_INDEX+MQTT_ENABLE_SIZE)

#define MQTT_PORT_SIZE sizeof(uint16_t)
#define MQTT_PORT_INDEX (MQTT_SERVER_SIZE+MQTT_SERVER_INDEX)

#define MQTT_ACCOUNT_SIZE sizeof(char)*33
#define MQTT_ACCOUNT_INDEX (MQTT_PORT_INDEX+MQTT_PORT_SIZE)

#define MQTT_PASSWORD_SIZE sizeof(char)*33
#define MQTT_PASSWORD_INDEX (MQTT_ACCOUNT_SIZE+MQTT_ACCOUNT_INDEX)

#define MQTT_DISCO_PREFIX_SIZE sizeof(char)*33
#define MQTT_DISCO_PREFIX_INDEX (MQTT_PASSWORD_INDEX+MQTT_PASSWORD_SIZE)

#define EEPROM_SIZE ( \
                        EEPROM_SIGNATURE_SIZE \
                        + UPLOAD_ENABLE_SIZE \
                        + DEVICE_NAME_SIZE \
                        + UPLOAD_URL_SIZE \
                        + UPLOAD_RATE_SIZE \
                        + LED_BRIGHTNESS_SIZE \
                        + WIFI_SSID_SIZE \
                        + WIFI_PASSWORD_SIZE \
                        + MQTT_ENABLE_SIZE \
                        + MQTT_SERVER_SIZE \
                        + MQTT_PORT_SIZE \
                        + MQTT_ACCOUNT_SIZE \
                        + MQTT_PASSWORD_SIZE \
                        + MQTT_DISCO_PREFIX_SIZE \
                    )

Configuration::Configuration()
:   _json_upload_enabled(TELEMETRY_DEFAULT_STATUS),
    _server_url(TELEMETRY_URL),
    _sensor_name(SENSOR_NAME)
{
    EEPROM.begin(EEPROM_SIZE);

    // check to see if this is a new install
    uint32_t signature;
    EEPROM.readBytes(EEPROM_SIGNATURE_INDEX, &signature, EEPROM_SIGNATURE_SIZE);

    char buffer[EEPROM_SIZE+1];
    if (signature != EEPROM_SIGNATURE) {
        Serial.println(F("EEPROM signature is invalid. Initializing EEPROM contents."));
        // initialize the EEPROM
        EEPROM.writeBytes(EEPROM_SIGNATURE_INDEX, &EEPROM_SIGNATURE, EEPROM_SIGNATURE_SIZE);

        buffer[0] = TELEMETRY_DEFAULT_STATUS;
        EEPROM.writeBytes(UPLOAD_ENABLE_INDEX, buffer, UPLOAD_ENABLE_SIZE);

        strcpy(buffer, SENSOR_NAME);
        buffer[DEVICE_NAME_SIZE-1] = 0;
        EEPROM.writeBytes(DEVICE_NAME_INDEX, buffer, strlen(buffer)+1);

        strcpy(buffer, TELEMETRY_URL);
        buffer[UPLOAD_URL_SIZE-1] = 0;
        EEPROM.writeBytes(UPLOAD_URL_INDEX, buffer, strlen(buffer)+1);

        uint16_t upload_rate = AIR_QUALITY_DATA_TRANSMIT_SECONDS;
        EEPROM.writeBytes(UPLOAD_RATE_INDEX, &upload_rate, UPLOAD_RATE_SIZE);

        buffer[0] = STATUS_LED_BRIGHTNESS;
        EEPROM.writeBytes(LED_BRIGHTNESS_INDEX, buffer, LED_BRIGHTNESS_SIZE);

        // the follow are all writing empty strings
        memset(buffer,0,EEPROM_SIZE+1);
        EEPROM.writeBytes(WIFI_SSID_INDEX, buffer, WIFI_SSID_SIZE);
        EEPROM.writeBytes(WIFI_PASSWORD_INDEX, buffer, WIFI_PASSWORD_SIZE);
        EEPROM.writeBytes(MQTT_SERVER_INDEX, buffer, MQTT_SERVER_SIZE);
        EEPROM.writeBytes(MQTT_ACCOUNT_INDEX, buffer, MQTT_ACCOUNT_SIZE);
        EEPROM.writeBytes(MQTT_PASSWORD_INDEX, buffer, MQTT_PASSWORD_SIZE);

        String defaultDiscoPrefix = "homeassistant";
        if (defaultDiscoPrefix.length()+1 <= MQTT_DISCO_PREFIX_SIZE) {
            EEPROM.writeBytes(
                MQTT_DISCO_PREFIX_INDEX,
                defaultDiscoPrefix.c_str(),
                defaultDiscoPrefix.length()+1
            );
        } else {
            EEPROM.writeBytes(MQTT_DISCO_PREFIX_INDEX, buffer, MQTT_DISCO_PREFIX_SIZE);
        }

        buffer[0] = false;
        EEPROM.writeBytes(MQTT_ENABLE_INDEX, buffer, MQTT_ENABLE_SIZE);

        uint16_t port_value = 1883;
        EEPROM.writeBytes(MQTT_PORT_INDEX, &port_value, MQTT_PORT_SIZE);



        EEPROM.commit();
    }

    // load values from EEPROM
    EEPROM.readBytes(UPLOAD_ENABLE_INDEX, &this->_json_upload_enabled, UPLOAD_ENABLE_SIZE);

    EEPROM.readBytes(DEVICE_NAME_INDEX, buffer, DEVICE_NAME_SIZE);
    this->_sensor_name = String(buffer);

    EEPROM.readBytes(UPLOAD_URL_INDEX, buffer, UPLOAD_URL_SIZE);
    this->_server_url = String(buffer);

    EEPROM.readBytes(UPLOAD_RATE_INDEX, &this->_upload_rate, UPLOAD_RATE_SIZE);
    EEPROM.readBytes(LED_BRIGHTNESS_INDEX, &this->_led_brightness, LED_BRIGHTNESS_SIZE);

    EEPROM.readBytes(WIFI_SSID_INDEX, buffer, WIFI_SSID_SIZE);
    this->_wifiSSID = String(buffer);
    EEPROM.readBytes(WIFI_PASSWORD_INDEX, buffer, WIFI_PASSWORD_SIZE);
    this->_wifiPW = String(buffer);

    EEPROM.readBytes(MQTT_ENABLE_INDEX, &this->_mqtt_enable, MQTT_ENABLE_SIZE);
    EEPROM.readBytes(MQTT_SERVER_INDEX, buffer, MQTT_SERVER_SIZE);
    this->_mqtt_server = String(buffer);
    EEPROM.readBytes(MQTT_PORT_INDEX, &this->_mqtt_port, MQTT_PORT_SIZE);
    EEPROM.readBytes(MQTT_ACCOUNT_INDEX, buffer, MQTT_ACCOUNT_SIZE);
    this->_mqtt_account = String(buffer);
    EEPROM.readBytes(MQTT_PASSWORD_INDEX, buffer, MQTT_PASSWORD_SIZE);
    this->_mqtt_password = String(buffer);
    EEPROM.readBytes(MQTT_DISCO_PREFIX_INDEX, buffer, MQTT_DISCO_PREFIX_SIZE);
    this->_mqtt_disco_pefix = String(buffer);


    Serial.println(F("Initial configuration:"));
    Serial.printf("  WiFi SSID = %s\n", this->_wifiSSID.c_str());
    Serial.printf("  Sensor Name = %s\n", this->_sensor_name.c_str());
    Serial.printf("  LED brightness index = %d\n", this->_led_brightness);
    Serial.printf("  JSON Upload enabled = %d\n", this->_json_upload_enabled);
    Serial.printf("  JSON Upload URL = %s\n", this->_server_url.c_str());
    Serial.printf("  JSON Upload rate = %d seconds\n", this->_upload_rate);
    Serial.print(F("  MQTT Enabled = ")); Serial.println(this->_mqtt_enable);
    Serial.print(F("  MQTT Server = ")); Serial.println(this->_mqtt_server);
    Serial.print(F("  MQTT Port = ")); Serial.println(this->_mqtt_port);
    Serial.print(F("  MQTT Discovery Prefix = ")); Serial.println(this->_mqtt_disco_pefix);
}

Configuration::~Configuration()
{
}

const String& Configuration::getServerURL(void) const
{
    return this->_server_url;
}

void Configuration::setServerURL(const String& url)
{
    this->_server_url = url;
    this->_server_url.trim();
    this->_server_url.remove(UPLOAD_URL_SIZE-2);
    EEPROM.writeBytes(UPLOAD_URL_INDEX, this->_server_url.c_str(), this->_server_url.length()+1);
    EEPROM.commit();
}

bool Configuration::getJSONUploadEnabled(void) const
{
    return this->_json_upload_enabled;
}

void Configuration::setJSONUploadEnabled(bool enabled)
{
    this->_json_upload_enabled = enabled;
    EEPROM.writeBytes(UPLOAD_ENABLE_INDEX, &this->_json_upload_enabled, UPLOAD_ENABLE_SIZE);
    EEPROM.commit();
}

const String& Configuration::getSensorName(void) const
{
    return this->_sensor_name;
}

String Configuration::getSensorID(void) const
{
    byte mac[6];
    WiFi.macAddress(mac);
    String macHash = mac2String(mac);

    if (this->getSensorName().length() == 0) {
        return String("diy-air-quality-monitor-") + macHash;
    } else {
        return convertNameToID(this->getSensorName()) + String("-") + macHash;
    }
}

void Configuration::setSensorName(const String& name)
{
    this->_sensor_name = name;
    this->_sensor_name.trim();
    this->_sensor_name.remove(DEVICE_NAME_SIZE-2);
    EEPROM.writeBytes(DEVICE_NAME_INDEX, this->_sensor_name.c_str(), this->_sensor_name.length()+1);
    EEPROM.commit();
}

uint16_t Configuration::getJSONUploadRateSeconds(void) const
{
    return this->_upload_rate;
}

void Configuration::setJSONUploadRateSeconds(uint16_t seconds)
{
    this->_upload_rate = seconds;
    EEPROM.writeBytes(UPLOAD_RATE_INDEX, &this->_upload_rate, UPLOAD_RATE_SIZE);
    EEPROM.commit();
}

uint8_t Configuration::getLEDBrightnessValue(void) const
{
    switch (this->getLEDBrightnessIndex()) {
        // off
        case 0:
            return 0;
        // low
        case 1:
            return 85;
        // medium
        default:
        case 2:
            return 170;
        case 3:
            return 255;
    }
}

uint8_t Configuration::getLEDBrightnessIndex(void) const
{
    return this->_led_brightness;
}

void Configuration::setLEDBrightnessIndex(uint8_t index)
{
    this->_led_brightness = (index < 4 ? index : 2);
    EEPROM.writeBytes(LED_BRIGHTNESS_INDEX, &this->_led_brightness, LED_BRIGHTNESS_SIZE);
    EEPROM.commit();
}

String Configuration::getLEDBrightnessName(void) const
{
    switch (this->getLEDBrightnessIndex()) {
        // off
        case 0:
            return String("Off");
        // low
        case 1:
            return String("Low");
        // medium
        default:
        case 2:
            return String("Medium");
        case 3:
            return String("Bright");
    }
}

const String& Configuration::getWifiSSID(void) const {
    return this->_wifiSSID;
}

void Configuration::setWiFiSSID(const String& ssid) {
    if (ssid.length() < WIFI_SSID_SIZE-2) {
        this->_wifiSSID = ssid;
        EEPROM.writeBytes(WIFI_SSID_INDEX, this->_wifiSSID.c_str(), this->_wifiSSID.length()+1);
        EEPROM.commit();
    }
}

const String& Configuration::getWifiPassword(void) const
{
    return this->_wifiPW;
}

void Configuration::setWiFiPassword(const String& password)
{
    if (password.length() < WIFI_PASSWORD_SIZE-2) {
        this->_wifiPW = password;
        EEPROM.writeBytes(WIFI_PASSWORD_INDEX, this->_wifiPW.c_str(), this->_wifiPW.length()+1);
        EEPROM.commit();
    }
}

// MQTT
bool Configuration::getMQTTEnabled(void) const
{
    return this->_mqtt_enable;
}

void Configuration::setMQTTEnabled(bool enabled)
{
    this->_mqtt_enable = enabled;
    EEPROM.writeBytes(MQTT_ENABLE_INDEX, &this->_mqtt_enable, MQTT_ENABLE_SIZE);
    EEPROM.commit();
}

const String& Configuration::getMQTTServer(void) const
{
    return this->_mqtt_server;
}
void Configuration::setMQTTServer(const String& server)
{
    if (server.length() < MQTT_SERVER_SIZE-2) {
        this->_mqtt_server = server;
        EEPROM.writeBytes(MQTT_SERVER_INDEX, this->_mqtt_server.c_str(), this->_mqtt_server.length()+1);
        EEPROM.commit();
    }
}

uint16_t Configuration::getMQTTPort(void) const
{
    return this->_mqtt_port;
}

void Configuration::setMQTTPort(uint16_t port)
{
    this->_mqtt_port = port;
    EEPROM.writeBytes(MQTT_PORT_INDEX, &this->_mqtt_port, MQTT_PORT_SIZE);
    EEPROM.commit();
}


const String& Configuration::getMQTTAccount(void) const
{
    return this->_mqtt_account;
}

void Configuration::setMQTTAccount(const String& account)
{
    if (account.length() < MQTT_ACCOUNT_SIZE-2) {
        this->_mqtt_account = account;
        EEPROM.writeBytes(MQTT_ACCOUNT_INDEX, this->_mqtt_account.c_str(), this->_mqtt_account.length()+1);
        EEPROM.commit();
    }
}

const String& Configuration::getMQTTPassword(void) const
{
    return this->_mqtt_password;
}

void Configuration::setMQTTPassword(const String& password)
{
    if (password.length() < MQTT_PASSWORD_SIZE-2) {
        this->_mqtt_password = password;
        EEPROM.writeBytes(MQTT_PASSWORD_INDEX, this->_mqtt_password.c_str(), this->_mqtt_password.length()+1);
        EEPROM.commit();
    }
}

const String& Configuration::getMQTTDiscoveryPrefix(void) const
{
    return this->_mqtt_disco_pefix;
}

void Configuration::setMQTTDiscoveryPrefix(const String& prefix)
{
    if (prefix.length() < MQTT_DISCO_PREFIX_SIZE-2) {
        this->_mqtt_disco_pefix = prefix;
        EEPROM.writeBytes(MQTT_DISCO_PREFIX_INDEX, this->_mqtt_disco_pefix.c_str(), this->_mqtt_disco_pefix.length()+1);
        EEPROM.commit();
    }
}