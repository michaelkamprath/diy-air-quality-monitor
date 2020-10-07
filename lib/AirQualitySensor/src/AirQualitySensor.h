#ifndef __AirQualitySensor__
#define __AirQualitySensor__
#include <Arduino.h>
#include <Vector.h>

typedef enum {
    AQI_GREEN,
    AQI_YELLOW,
    AQI_ORANGE,
    AQI_RED,
    AQI_PURPLE,
    AQI_MAROON
} AQIStatusColor;

class AirQualitySensor {
private:
    uint32_t    _sensor_refresh_seconds;

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

    float   _avgPM2p5_Current;
    float   _avgPM2p5_10Min;
    float   _avgPM2p5_1Hour;
    float   _avgPM2p5_24Hour;
  
    uint16_t*           _vectorStorage;
    Vector<uint16_t>    _pm2p5_history;
    size_t              _pm2p5_history_insertion_idx;
public:
    AirQualitySensor(uint32_t sensor_refresh_seconds);
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

   // returns PM2.5 average value for the prior window_size_seconds seconds
   float averagePM2p5( int32_t window_size_seconds ) const;

   // return AQI for the given average PM2.5
   float airQualityIndex( float avg_pm2p5 ) const;

   // convenience functions
   float currentAirQualityIndex(void) const         { return airQualityIndex(_avgPM2p5_Current); }
   float tenMinuteAirQualityIndex(void) const       { return airQualityIndex(_avgPM2p5_10Min); }
   float oneHourAirQualityIndex(void) const         { return airQualityIndex(_avgPM2p5_1Hour); }
   float oneDayAirQualityIndex(void) const          { return airQualityIndex(_avgPM2p5_24Hour); }

   // 
   // static utilty functions
   //
    static AQIStatusColor getAQIStatusColor(float aqi_value);    
};

#endif