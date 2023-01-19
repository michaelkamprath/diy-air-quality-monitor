#include <EEPROM.h>
#include "Configuration.h"


// determines if the JSCON telemetry uplod is by default on or off
#ifndef TELEMETRY_DEFAULT_STATUS
#define TELEMETRY_DEFAULT_STATUS false
#endif

// defines the URL of a RESTful service that sensor measurements should be POSTed to in JSON form.
// Set to nullptr if you do no wish to POST measurement JSON payloads.
#ifndef TELEMETRY_URL
#define TELEMETRY_URL    ""
#endif

// Defines the name this device should be given. This name is used to identify this device
// in both the web UI it serves and the JSON payloads posted to TELEMETRY_URL.
#ifndef SENSOR_NAME
#define SENSOR_NAME     ""
#endif


const uint32_t EEPROM_SIGNATURE = 0xFEEDC0DE;
#define EEPROM_SIGNATURE_SIZE sizeof(uint32_t)
#define EEPROM_SIGNATURE_INDEX 0

#define UPLOAD_ENABLE_SIZE sizeof(uint8_t)
#define UPLOAD_ENABLE_INDEX (EEPROM_SIGNATURE_SIZE)

#define DEVICE_NAME_SIZE sizeof(char)*33
#define DEVICE_NAME_INDEX (EEPROM_SIGNATURE_SIZE+UPLOAD_ENABLE_SIZE)

#define UPLOAD_URL_SIZE sizeof(char)*257
#define UPLOAD_URL_INDEX (EEPROM_SIGNATURE_SIZE+UPLOAD_ENABLE_SIZE+DEVICE_NAME_SIZE)

#define EEPROM_SIZE (EEPROM_SIGNATURE_SIZE+UPLOAD_ENABLE_SIZE+DEVICE_NAME_SIZE+UPLOAD_URL_SIZE)

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

        EEPROM.commit();
    }

    // load values from EEPROM
    EEPROM.readBytes(UPLOAD_ENABLE_INDEX, &this->_json_upload_enabled, UPLOAD_ENABLE_SIZE);

    EEPROM.readBytes(DEVICE_NAME_INDEX, buffer, DEVICE_NAME_SIZE);
    this->_sensor_name = String(buffer);

    EEPROM.readBytes(UPLOAD_URL_INDEX, buffer, UPLOAD_URL_SIZE);
    this->_server_url = String(buffer);

    Serial.println(F("Initial configuration:"));
    Serial.printf("  JSON Upload enabled = %d\n", this->_json_upload_enabled);
    Serial.printf("  JSON Upload URL = %s\n", this->_server_url.c_str());
    Serial.printf("  Sensor Name = %s\n", this->_sensor_name.c_str());
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
    this->_sensor_name.remove(DEVICE_NAME_SIZE-2);
    EEPROM.writeBytes(DEVICE_NAME_INDEX, this->_sensor_name.c_str(), this->_sensor_name.length()+1);
    EEPROM.commit();
}