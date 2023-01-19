#include "Configuration.h"

Configuration::Configuration()
:   _json_upload_enabled(TELEMETRY_DEFAULT_STATUS),
    _server_url(TELEMETRY_URL),
    _sensor_name(SENSOR_NAME)
{
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
}

bool Configuration::getJSONUploadEnabled(void) const
{
    return this->_json_upload_enabled;
}

void Configuration::setJSONUploadEnabled(bool enabled)
{
    this->_json_upload_enabled = enabled;
}

const String& Configuration::getSensorName(void) const
{
    return this->_sensor_name;
}

void Configuration::setSensorName(const String& name)
{
    this->_sensor_name = name;
}