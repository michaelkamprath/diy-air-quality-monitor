# DIY Air Quality Monitor

This project contains the software for a DIY air quality monitor based on the Panasonic SN-GCJA5 air quality sensor. This software is designed to run on a [TinyPICO](https://www.tinypico.com) ESP32 development board and be built by PlatformIO in Visual Code Studio.

To use this code, first edit `include/Configuration.h` as needed, then build and upload to the TinyPICO. The Panasonic SN-GCJA5 is connected to the TinyPICO via it's serial connection at pin 33.

*Currently this project very much a work in progress.*

## Reference Material

* [Panasonic SN-GCJA5 Product Specification](https://na.industrial.panasonic.com/products/sensors/air-quality-gas-flow-sensors/lineup/laser-type-pm-sensor/series/123557/model/123559)
* [Panasonic SN-GCJA5 Communications Specification](https://b2b-api.panasonic.eu/file_stream/pids/fileversion/8814)

## Bill of Materials
To build this project, you will need the folloing components:

* [Panasonic SN-GCJA5 Sensor](https://www.mouser.com/ProductDetail/667-SN-GCJA5) - This is the particulate counter that the project is centered around.
* [GHR-05V-S Cable Housing](https://www.digikey.com/en/products/detail/jst-sales-america-inc/AGHGH28K305/6009450) - You need to make a cable to connect the SN-GCJA5. This is the connector housing fo rthe end of the cable that connects to the sensor. You are free to determine how you connect the other end of the cable to the microntroller.
* [Pre-crimped Jumper Wire](https://www.digikey.com/en/products/detail/jst-sales-america-inc/AGHGH28K305/6009450) - These are pre-made cables that have the special connector for the SN-GCJA5 crimped onto both ends. You only need the connector on one end, and that end gets inserted into the cable housing from the previous line. At a minimum you need three of these, for the +5V, GND, and TX connection on the sensor. If you wish to make a cable that also enables the I2C connection to the sensor (even though this project does not use it), get 5 to fully pupulate the cable housing. See the SN-GCJA5 Product Specification for information on placement of each cable with respect to the connector n the sensor.
* [TinyPICO ESP32 Microcontroller Board](https://unexpectedmaker.com/shop/tinypico) - This is the ESP32 microcontroller that will control the sensor and provide WiFi connectivity. In truth, with some modifications of the code here, you could probably use any ESP32 board or even an ESP8266. However, the TinyPICO is very versatile, has a good WiFi antenna, has PSRAM already installed (which this project uses), and the form factor can't be beat.
* [Dupont Cable Housing](https://www.ebay.com/itm/100Pcs-1P-Dupont-Jumper-Wire-Cable-Housing-Female-Pin-Connector-2-54-mm-Pitch/112299848779) - My prefered way of connectng the sensor to the TinyPICO is to use female dupont connectors to the pins that get soldered to the TinyPICO. Here are the housings to make those connectors on the other end of the cable you need to build to connect the SN-GCJA5.
* [Female Pin Dupont Connector](https://www.ebay.com/itm/US-Stock-100pcs-Female-Pin-Dupont-Connector-Gold-Plated-2-54mm-Pitch/371912445248) - My prefered way of connectng the sensor to the TinyPICO is to use femal dupont connectors to the pins that get soldered to the TinyPICO. Here are the female connectors needed to make the connectors.
* *OPTIONAL* [BME680 Environment Sensor Board](https://www.digikey.com/products/en?mpart=3660&v=1528) - You can optionally attach a BME680 sensor to this project to additionally get temperature, pressure, and humidity measurements along with the air quality measurement that the SN-GCJA5 provides. Note that you will need some 26 to 30 AWG hook up wire to construct the cables needed to connect the BME680 to to the TinyPICO, but you can use on either end of those cables the dupont connectors that you are ordering SN-GCJA5.

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