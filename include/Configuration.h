#ifndef __Configuration__
#define __Configuration__

// defines the URL of a RESTful service that sensor measurements should be POSTed to in JSON form.
// Set to nullptr if you do no wish to POST measurement JSON payloads.
#ifndef TELEMETRY_URL
#define TELEMETRY_URL    nullptr
#endif

// Defines the WiFi access point this device should connected to. 
#ifndef WIFI_SSID
#define WIFI_SSID        "YOUR_WIFI_SSID"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD    "YOUR_WIFI_PASSWORD"
#endif

// Defines the name this device should be given. This name is used to identify this device
// in both the web UI it serves and the JSON payloads posted to TELEMETRY_URL.
#ifndef SENSOR_NAME
#define SENSOR_NAME     "YOUR_DEVICE_NAME"
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

// Defines the number of AIR_QUALITY_SENSOR_UPDATE_SECONDS cycle that must occur between
// each data transmission to the TELEMETRY_URL. Has no net effect if TELEMETRY_URL is
// a nullptr. When transmitting data, only the measurement from the current cycle is
// transmitted. Must be an integer.
#ifndef AIR_QUALITY_DATA_TRANSMIT_MULTIPLE
#define AIR_QUALITY_DATA_TRANSMIT_MULTIPLE   30
#endif

// Sets the brightness level of the on-board RGB LED. Should be a integer between 0 (off) and
// 255 (full brightness). Hex values are fine.
#ifndef STATUS_LED_BRIGHTNESS
#define STATUS_LED_BRIGHTNESS   0x80
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
#ifndef MCU_BOARD_TYPE
#define MCU_BOARD_TYPE MCU_TINYPICO
#endif


#endif // __Configuration__
