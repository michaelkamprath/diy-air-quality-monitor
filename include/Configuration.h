#ifndef __Configuration__
#define __Configuration__

//
// Default Configuration Values
//
// These are not intended to be editted here, but the values can be overriden in the
// platformio.ini file.
//

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

// Defines the legnth of time between measurements are taken from the air quality sensor.
// Measurements are retained for the purposes of calculating the averages. Be mindful of
// how much RAM must be used to retain 1 days's worth of measurements, but more measurements
// yield better averages. Due to sensor limitations, the fastest measurement rate possible is
// every second. This value must be an integer.
#ifndef AIR_QUALITY_SENSOR_UPDATE_SECONDS
#define AIR_QUALITY_SENSOR_UPDATE_SECONDS   2
#endif

// Defines the delay between repeated attempts to reconnect to Wifi access point.
#ifndef AIR_QUALITY_SENSOR_WIFI_RECONNECT_DELAY_SECONDS
#define AIR_QUALITY_SENSOR_WIFI_RECONNECT_DELAY_SECONDS 60
#endif

// Defines the number of seconds that must occur between each data transmission to the
// TELEMETRY_URL. Has no net effect if TELEMETRY_URL is empty.
// When transmitting data, only the measurement from the most recent cycle is
// transmitted. Must be an integer.
#ifndef AIR_QUALITY_DATA_TRANSMIT_SECONDS
#define AIR_QUALITY_DATA_TRANSMIT_SECONDS   90
#endif

// Sets the brightness level of the on-board RGB LED. This is an index value as follows:
//      0 = off
//      1 = Low (85/255)
//      2 = Medium (170/255)
//      3 = Full (255/555)
#ifndef STATUS_LED_BRIGHTNESS
#define STATUS_LED_BRIGHTNESS  2
#endif

// BME680 I2C Address. The default address for the AdaFruit Libary is 0x77, and this is what the AdaFruit BME680 board is set to.
// Most "home brew" BME680 boards use the address of 0x76. Change the value here according to how your BME680 sensor is
// configured. If you are not using a BME680 censor, then this address can be any value.
#ifndef BME680_SENSOR_I2C_ADDRESS
#define BME680_SENSOR_I2C_ADDRESS   0x77
#endif

// Selects which ESP32 microcontroller is being used. Note that for boards that do not have PSRAM, it is recommended to
// set the AIR_QUALITY_SENSOR_UPDATE_SECONDS value to at least 5.
#define MCU_TINYPICO 1
#define MCU_EZSBC_IOT 2
#define MCU_YD_ESP32_S3 3
#ifndef MCU_BOARD_TYPE
#define MCU_BOARD_TYPE MCU_TINYPICO
#endif

#include <Arduino.h>

class Configuration {
private:
    bool _json_upload_enabled;
    String _server_url;
    String _sensor_name;
    uint16_t _upload_rate;
    uint8_t _led_brightness;

public:
    Configuration();
    virtual ~Configuration();

    const String& getServerURL(void) const;
    void setServerURL(const String& url);

    bool getJSONUploadEnabled(void) const;
    void setJSONUploadEnabled(bool enabled);

    const String& getSensorName(void) const;
    void setSensorName(const String& name);

    uint16_t getJSONUploadRateSeconds(void) const;
    void setJSONUploadRateSeconds(uint16_t seconds);

    uint8_t getLEDBrightnessValue(void) const;
    uint8_t getLEDBrightnessIndex(void) const;
    void setLEDBrightnessIndex(uint8_t index);
};


#endif // __Configuration__
