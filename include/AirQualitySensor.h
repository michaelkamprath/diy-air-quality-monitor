#ifndef __AirQualitySensor__
#define __AirQualitySensor__
#include <Arduino.h>

class AirQualitySensor {
private:
    uint32_t    _pm1p0; // PM1.0
    uint32_t    _pm2p5; // PM2.5
    uint32_t    _pm10;  // PM10
    uint16_t    _particleCount0p5um;
    uint16_t    _particleCount1p0um;
    uint16_t    _particleCount2p5um;
    uint16_t    _particleCount5p0um;
    uint16_t    _particleCount7p5um;
    uint16_t    _particleCount10um;
    uint8_t     _sensorStatus;

public:
    AirQualitySensor();
    virtual ~AirQualitySensor();

    void begin(void);

    // returns true if new data was fetched
    bool updateSensorReading(void);

    // Particulate MAtter readings
    float PM1p0( void ) const               { return (float)_pm1p0/1000.0; }
    float PM2p5( void ) const               { return (float)_pm2p5/1000.0; }
    float PM10( void ) const                { return (float)_pm10/1000.0; }

    uint16_t particalCount0p5(void) const   { return _particleCount0p5um; }
    uint16_t particalCount1p0(void) const   { return _particleCount1p0um; }
    uint16_t particalCount2p5(void) const   { return _particleCount2p5um; }
    uint16_t particalCount5p0(void) const   { return _particleCount5p0um; }
    uint16_t particalCount7p5(void) const   { return _particleCount7p5um; }
    uint16_t particalCount10(void) const    { return _particleCount10um; }

    uint8_t statusParticleDetector(void) const;
    uint8_t statusLaser(void) const;
    uint8_t statusFan(void) const;

   
};

#endif