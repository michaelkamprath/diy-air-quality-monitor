Grafana Dashboard
=================

This folder contains docker compose configuration to configure
grafana/influxdb/telegraf to query (every 10 seconds) your AQI
monitors, and to chart the AQI over time.

Setup
-----

1. Edit `config.env` and adjust the list of URLs to point at your homebrew sensors

2. Run `docker-compose up -d`

Note: There is no authentication for querying and posting data to `influxdb`, the
usernames and passwords (in the docker-compose and grafana config) are currently
ignored.

User Interface
--------------

By default the interface will run on port 3000 of the machine you are running
docker on, e.g. http://localhost:3000 - once loaded you should be able to see
AQI dashboards.

A couple of pre-built dashboards are setup by default, you can use these as
as starting points to create your own custom dashboard

Logging
-------

To see logs for grafana, influxdb and telegraf, use:

```
docker-compose logs --tail=20 -f
```

Notes
-----

In order to be able to compute the AQI, the dashboards utilize the mean PM2.5, and
then convert to AQI. This is performed via the Flux InfluxDB language rather than
using the InfluxQL query language.

Of note is that flux support in grafana is in development, and is flaky at best,
particularly the "explore" option.

An example query performed is:

```
// Interpolate a value in a range to a new range
interpolate = (lowVal, highVal, lowOut, highOut, value) => lowOut + (highOut-lowOut)*(value-lowVal)/(highVal-lowVal)

// Calculate the AQI from a pm2.5 count from
// https://www.epa.gov/sites/production/files/2016-04/documents/2012_aqi_factsheet.pdf
calcAQI = (val) => if not exists val then val
      else if val <= 12.0 then interpolate(lowVal:0.0, highVal:12.0, lowOut:0.0, highOut:50.0, value:val)
      else if val <= 35.4 then interpolate(lowVal:12.0, highVal:35.4, lowOut:50.0, highOut:100.0, value:val)
      else if val <= 55.4 then interpolate(lowVal:35.4, highVal:55.4, lowOut:100.0, highOut:150.0, value:val)
      else if val <= 150.4 then interpolate(lowVal:55.4, highVal:150.4, lowOut:150.0, highOut:200.0, value:val)
      else if val <= 250.4 then interpolate(lowVal:150.4, highVal:250.4, lowOut:200.0, highOut:300.0, value:val)
      else if val <= 350.4 then interpolate(lowVal:250.4, highVal:350.4, lowOut:300.0, highOut:400.0, value:val)
      else interpolate(lowVal:350.4, highVal:500.0, lowOut:400.0, highOut:500.0, value:val)

// Map row values to AQI
mapToAQI = (tables=<-) =>
  tables
    |> map(fn: (r) => ({ r with _value: calcAQI(val: r._value)}))

// Query for the PM2.5 count and conert to AQI
from(bucket: "telegraf")
  |> range(start: v.timeRangeStart, stop:v.timeRangeStop)
  |> filter(fn: (r) => r._measurement == "homebrew_aqi" )
  |> filter(fn: (r) => r._field == "mass_density_pm2p5")
  |> group(columns: ["sensor_id"], mode: "by")
  |> aggregateWindow(every: 10m, fn: mean)
  |> mapToAQI()
  |> yield()
  
 ```
 