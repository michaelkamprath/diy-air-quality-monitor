#include <EEPROM.h>
#include "Configuration.h"


const uint32_t EEPROM_SIGNATURE = 0xC0DEB005;
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

#define WIFI_SSID_SIZE sizeof(char)*129
#define WIFI_SSID_INDEX (LED_BRIGHTNESS_INDEX+LED_BRIGHTNESS_SIZE)

#define WIFI_PASSWORD_SIZE sizeof(char)*129
#define WIFI_PASSWORD_INDEX (WIFI_SSID_INDEX+WIFI_SSID_SIZE)

#define EEPROM_SIZE ( \
                        EEPROM_SIGNATURE_SIZE \
                        + UPLOAD_ENABLE_SIZE \
                        + DEVICE_NAME_SIZE \
                        + UPLOAD_URL_SIZE \
                        + UPLOAD_RATE_SIZE \
                        + LED_BRIGHTNESS_SIZE \
                        + WIFI_SSID_SIZE \
                        + WIFI_PASSWORD_SIZE \
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

        memset(buffer,0,EEPROM_SIZE+1);
        EEPROM.writeBytes(WIFI_SSID_INDEX, buffer, WIFI_SSID_SIZE);
        EEPROM.writeBytes(WIFI_PASSWORD_INDEX, buffer, WIFI_PASSWORD_SIZE);

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

    Serial.println(F("Initial configuration:"));
    Serial.printf("  WiFi SSID = %s\n", this->_wifiSSID.c_str());
    Serial.printf("  JSON Upload enabled = %d\n", this->_json_upload_enabled);
    Serial.printf("  JSON Upload URL = %s\n", this->_server_url.c_str());
    Serial.printf("  JSON Upload rate = %d seconds\n", this->_upload_rate);
    Serial.printf("  Sensor Name = %s\n", this->_sensor_name.c_str());
    Serial.printf("  LED brightness index = %d\n", this->_led_brightness);
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