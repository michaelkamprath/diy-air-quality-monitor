# DIY Air Quality Monitor

This project contains the software for a DIY Air Quality Monitor based on the [Panasonic SN-GCJA5 air quality sensor](https://www.mouser.com/ProductDetail/667-SN-GCJA5). This software is designed to run on an ESP32 development board, such as the [TinyPICO](https://www.tinypico.com), and be built by [PlatformIO](https://platformio.org) in [Visual Code Studio](https://code.visualstudio.com).

To use this code, you just need to build and upload to the ESP32 development board of your choice. Three ESP32 boards are supported explicitly, though the code can easily modified to support other ESP32 boards (the biggest issue here is how the board's status LED is configured). The Panasonic SN-GCJA5 is connected to the ESP32 development board via it's serial connection pin (see table below).

When the software lauches for the first time, it is in configuration mode. To configure the device, use a mobile phone to connect to the WiFi SSID named "DIY Air Quality Monitor". You will then be presented with the deive's configuration page. At a minimum, set the WiFi credential that the DIY Air Quality Monitor should connect to. It is also strongly encouraged to name the device with a unique name. Once the  DIY Air Quality Monitor is connected to your WiFi, you may connect to it at the IP address your network gave it. The web pages that the  DIY Air Quality Monitor servers includes a view at the current air quality statistics, the configuration page for the deivce, and a internal statistics page.

## Reference Material

* [Panasonic SN-GCJA5 Product Specification](https://na.industrial.panasonic.com/products/sensors/air-quality-gas-flow-sensors/lineup/laser-type-pm-sensor/series/123557/model/123559)
* [Panasonic SN-GCJA5 Communications Specification](https://api.pim.na.industrial.panasonic.com/file_stream/main/fileversion/244938)

## Bill of Materials
To build this project, you will need the folloing components:

* [Panasonic SN-GCJA5 Sensor](https://www.mouser.com/ProductDetail/667-SN-GCJA5) - This is the particulate counter that the project is centered around.
* [GHR-05V-S Cable Housing](https://www.digikey.com/en/products/detail/jst-sales-america-inc/GHR-05V-S/807817) - You need to make a cable to connect the SN-GCJA5. This is the connector housing fo rthe end of the cable that connects to the sensor. You are free to determine how you connect the other end of the cable to the microntroller.
* [Pre-crimped Jumper Wire](https://www.digikey.com/en/products/detail/jst-sales-america-inc/AGHGH28K305/6009450) - These are pre-made cables that have the special connector for the SN-GCJA5 crimped onto both ends. You only need the connector on one end, and that end gets inserted into the cable housing from the previous line. At a minimum you need three of these, for the +5V, GND, and TX connection on the sensor. If you wish to make a cable that also enables the I2C connection to the sensor (even though this project does not use it), get 5 to fully populate the cable housing. See the SN-GCJA5 Product Specification for information on placement of each cable with respect to the connector n the sensor.
* ESP32 Microcontroller Board - This code is written to support various ESP32 microntorller development boards:
  * [TinyPICO ESP32 Microcontroller Board](https://www.tinypico.com/) - The TinyPICO is very versatile, has a good WiFi antenna, has PSRAM already installed (which this project uses), and the form factor can't be beat. The TinyPICO is the preferred ESP32 board for this project.
  * [EzSBC ESP32 Development Board](https://www.ezsbc.com/product/esp32-breakout-and-development-board/) - Another fine ESP32 development board. However, this EzSBC board does not come with PSRAM installed, which limits the historical data that can be retained by the microcontroller.
  * [YD-ESP32-S3](https://www.aliexpress.us/item/3256804451136917.html) - This is a off brand ESP32-S3 development board you can get off AliExpress. Fairly cheap, but fairly powerful (the web app runs noticably faster opn this ESP32-S3 board). This board does have PSRAM. To use the onboard WS2812B RGB LED, you need to bridge the `RGB` solder gap. If you want to power the `SN-GCJA5` sensor with the 5V USB power pin on the board, you also need to bridge the `IN-OUT` solder gap.
* *OPTIONAL* [Dupont Cable Housing](https://www.ebay.com/itm/100Pcs-1P-Dupont-Jumper-Wire-Cable-Housing-Female-Pin-Connector-2-54-mm-Pitch/112299848779) - My prefered way of connectng the sensor to the ESP32 development board is to use female dupont connectors to the pins that get soldered to the ESP32 development board. Here are the housings to make those connectors on the other end of the cable you need to build to connect the SN-GCJA5. Alternatively, you can solder the cable wires directly to the ESP32 development board pin holes.
* *OPTIONAL* [Female Pin Dupont Connector](https://www.ebay.com/itm/US-Stock-100pcs-Female-Pin-Dupont-Connector-Gold-Plated-2-54mm-Pitch/371912445248) - My prefered way of connectng the sensor to the ESP32 development board is to use female dupont connectors to the pins that get soldered to the ESP32 development board. Here are the female connectors needed to make the connectors.
* *OPTIONAL* [BME680 Environment Sensor Board](https://www.digikey.com/products/en?mpart=3660&v=1528) - You can optionally attach a BME680 sensor to this project to additionally get temperature, pressure, and humidity measurements along with the air quality measurement that the SN-GCJA5 provides. Note that you will need some 26 to 30 AWG hook up wire to construct the cables needed to connect the BME680 to to the ESP32 development board, but you can use on either end of those cables the dupont connectors that you are ordering SN-GCJA5.

## Pin Connections
Refer to your ESP32 development board's pinout to determine the exact location of the indicated ESP32 pin.

### Panasonic SN-GCJA5
This code's default confuration for the serial `RX` pin that will be used to recieve data form the air quality sensor is listed below. Note the the `RX` can be configured in the `platformio.ini` file by setting the `SERIAL_RX_PIN` build flag for the device configuration you are using.

| ESP32 Pin | ESP32-S3 Pin | Sesnor Pin | Description |
|:-:|:-:|:-:|:--|
| `5V` | `5V` | 5 | The Panasonic SN-GCJA5 uses 5V power. On most ESP32 boards this is marked as either `5V` or `Vusb`. |
| `GND` | `GND` |  4 | Ground |
| `IO33` | `IO2` |  1 | The Panasonic SN-GCJA5 serial TX line (so RX on the ESP32). Note that this serial line operates at 3.3V, so it is voltage safe for the ESP32 |,

### BME680 Environment Sensor Board
| ESP32 Pin | ESP32-S3 Pin | Sesnor Pin | Description |
|:-:|:-:|:-:|:--|
| `3v3` | `3v3` |  `Vcc` | The BME680 runs off of 3.3V. |
| `GND` | `GND` |  `GND` | Ground |
| `IO22` | `IO18` |  `SCL` | The I2C clock line |
| `IO21`| `IO17` | `SDA` | The I2C data line |

# Usage
The included software for the ESP32 development board launches a web server that hosts a few pages which allow interaction with the air quality monitor. About a minute after the device connects to the configured WiFi (this is required for the Panasonic SN-GCJA5 start up sequence), the webserver will be come active and you may visit the pages at the IP address assigned by the DHCP server of your WiFi service. It is recomended (though not required) that you implement a static IP address for your ESP32 development board in your DHCP server's configuration.

## Viewing Status
You can view the air quality measurement by visiting the root web page of your ESP32 device's IP address, that is `http://your.device.ip/`.

## Configuration
A configuration page exists that allows you to alter some of the air quality monitor's behavior. This page is availabe at `http://your.device.ip/config.html`. The following configuration options are  available:

* **WiFi SSID** - The name of the WiFi network that the device should connect to.
* **WiFi Password** - The password of the WiFi network that the device should connect to.
* **Sensor name** - The name for this monitor. Allows the source of teletry to be identified for either the JSON POST or the MQTT service.
* **LED Brightness** - Allows adjusting the brightness of the air quality indicator LED on the ESP32 development board.
* **Enable MQTT Connection** - Enables the MQTT service to allow the device to communicate with Home Assistant.
* **MQTT Server** - The MQTT server address.
* **MQTT Port** - The MQTT server port. Defaults to 1883.
* **MQTT Account** - The account name for connecting to the MQTT server. Leave blank if there is no account.
* **MQTT Password** - The password for connecting to the MQTT server. Leave blank if there is no password.
* **Discovery Prefix** - The discovery prefix for MQTT. Defaults to `homeassistant`. Only change if you know what you are doing.
* **Enable posting JSON telemetry** - Enables the [JSON Push](#json-push) of teletry.
* **JSON telementry server URL** - The URL endpoint that JSON-packaged telemetry data should be POSTed to.
* **Upload rate** - The minimum number of seconds in between telemetry transmission for the JSON Push method. Also used for the MQTT update rate.

## Data Collection

### Home Assistant
This air quality monitor can be used as a sensor for [Home Assistant](https://www.home-assistant.io) or with a MQTT-based service. This is done by setting up an MQTT server connection in the configuration page of the sensor's web interface.  At a minimum you need to anable the MQTT connection and set the MQTT server address. You may also need to set the MQTT server login and password if your MQTT server requires that. Finally, by default the DIY Air Quality Monitor is configured to be automatically discovered by Home Assistant if your Home Assistant service is connected to the same MQTT server. If you know what you are doing, you can change the discovery prefix, but note that the discovery config will still be formatted for Home Assistant even if you change the prefix.

### JSON Push
This code has the option to POST all of the granular and detailed data collected from the connected sensors to a data collection service in a JSON format for later analysis. This is done by editing the JSON Telemetry options in the configuration web page hosted by the ESP32 development board. A recommended data collection service is the [Simple JSON Collector Service](https://github.com/michaelkamprath/simple-json-collector-service).

Note that this setting can be edited live on the device by visting the `http://your.device.ip/config.html` web paged served by the ESP32 device.

### Grafana
A Grafa-based data collection service can be set up to pull data from the air quality monitor. See [the grafana directory](grafana/) for more information.

# TODO
The following features are planned. Listed in no particular order.

1. ~~Allow different rates for data collection from sensor and data posting to telemetry service. Note that the data collection rate must be greater than or equal to the telemetry posting rate.~~
2. ~~Give options for look-back window of average AQI in the web UI~~
   * Add cookie to remember user's last selected averaging window.
3. ~~Add support for the BME680 sensor, which would give gas, pressure, temperature & humidity readings.~~
   * Make the units used for the display of the temperature (celsius or fahrenheit) configurable
4. Add support for an ePaper display that does the following:
   * Display the average AQI (configurable look back window)
   * Display a warning based on the color code of the AQI
   * Display the BME 680 sensor data, if attached.
   * Display the web UI URL
5. ~~Create a web UI to set up and configure the monitor, replacing the `Configuration.h` file. Would depend on ePaper display to display the temporary WiFi AP the user needs to connect to to configure.~~
6. Add ability to download history as a CSV from web UI.
