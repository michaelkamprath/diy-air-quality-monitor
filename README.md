# DIY Air Quality Monitor

This project contains the software for a DIY air quality monitor based on the Panasonic SN-GCJA5 air quality sensor. This software is designed to run on a [TinyPICO](https://www.tinypico.com) ESP32 development board and be built by PlatformIO in Visual Code Studio.

To use this code, first edit `include/Configuration.h` as needed, then build and upload to the TinyPICO. The Panasonic SN-GCJA5 is connected to the TinyPICO via it's serial connection at pin 33.

*Currently this project very much a work in progress.*

## TODO
The following features are planned. Listied in no particular order.

1. Allow different rates for data collection from sensor and data posting to telemetry service. Note that the data collection rate must be greater than or equal to the telemetry posting rate.
2. Give options for look-back window of average AQI in the web UI
3. ~~Add support for the BME680 sensor, which would give gas, pressure, temperature & humidity readings.~~
4. Add support for an ePaper display that does the following:
   * Display the average AQI (configurable look back window)
   * Display a warning based on the color code of the AQI
   * Display the BME 680 sensor data, if attached.
   * Display the web UI URL
5. Create a web UI to set up and configure the monitor, replacing the `Configuration.h` file. Would depend on ePaper display to display the temporary WiFi AP the user needs to connect to to configure.
6. Add ability to download history as a CSV from web UI.