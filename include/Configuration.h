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

// Defines the name this device should be given. This bame is used to identify this device
// in both the web UI it serves and the JSON payloads posted to TELEMETRY_URL.
#ifndef SENSOR_NAME
#define SENSOR_NAME     "YOUR_DEVICE_NAME"
#endif

// Defines thge legnth of time in between updates (measured in seconds)
#ifndef AIR_QUALITY_SENSOR_UPDATE_SECONDS
#define AIR_QUALITY_SENSOR_UPDATE_SECONDS   15
#endif

// Sets the brightness level of the TinyPICO's on-board RGB LED. Should be a number between 0 (off)
// 255 (full brightness). Hex values are fine.
#ifndef STATUS_LED_BRIGHTNESS
#define STATUS_LED_BRIGHTNESS   0x80
#endif

#endif // __Configuration__