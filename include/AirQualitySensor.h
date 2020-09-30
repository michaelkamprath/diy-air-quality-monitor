#ifndef __AirQualitySensor__
#define __AirQualitySensor__
#include <Arduino.h>
#include <Array.h>

#define AIR_QUALITY_SENSOR_UPDATE_SECONDS   60
#define AIR_QUALITY_SENSOR_HISTORY_SIZE     86400/AIR_QUALITY_SENSOR_UPDATE_SECONDS

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

    Array<uint16_t,AIR_QUALITY_SENSOR_HISTORY_SIZE>    _pm2p5_history;
    size_t _pm2p5_history_insertion_idx;
public:
    AirQualitySensor();
    virtual ~AirQualitySensor();

    void begin(void);

    // returns true if new data was fetched
    bool updateSensorReading(void);

    // Particulate MAtter readings
    uint32_t PM1p0( void ) const               { return _pm1p0; }
    uint32_t PM2p5( void ) const               { return _pm2p5; }
    uint32_t PM10( void ) const                { return _pm10; }

    uint16_t particalCount0p5(void) const   { return _particleCount0p5um; }
    uint16_t particalCount1p0(void) const   { return _particleCount1p0um; }
    uint16_t particalCount2p5(void) const   { return _particleCount2p5um; }
    uint16_t particalCount5p0(void) const   { return _particleCount5p0um; }
    uint16_t particalCount7p5(void) const   { return _particleCount7p5um; }
    uint16_t particalCount10(void) const    { return _particleCount10um; }

    uint8_t statusParticleDetector(void) const;
    uint8_t statusLaser(void) const;
    uint8_t statusFan(void) const;

   // average value needed to calculat AQI
   float averagePM2p5( void ) const;
   float airQualityIndex( void ) const;
   int32_t airQualityIndexLookbackWindowSeconds( void ) const;
};

#endif