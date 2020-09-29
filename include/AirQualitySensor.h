#ifndef __AirQualitySensor__
#define __AirQualitySensor__
#include <Arduino.h>

class AirQualitySensor {
private:
    uint32_t    _pm1p0; // PM1.0
    uint32_t    _pm2p5; // PM2.5
    uint32_t    _pm10;  // PM10

    uint8_t     _sensorStatus;

public:
    AirQualitySensor();
    virtual ~AirQualitySensor();

    // returns true if new data was fetched
    bool updateSensorReading(void);

    // Particulate MAtter readings
    uint32_t PM1p0( void ) const        { return _pm1p0; }
    uint32_t PM2p5( void ) const        { return _pm2p5; }
    uint32_t PM10( void ) const        { return _pm10; }
};

#endif