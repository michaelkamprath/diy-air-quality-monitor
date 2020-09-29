#include "AirQualitySensor.h"
#include "Utilities.h"

//
// Sensor Type = Panasonic SN-GCJA5
//   https://na.industrial.panasonic.com/products/sensors/air-quality-gas-flow-sensors/lineup/laser-type-pm-sensor/series/123557/model/123559
//

#define AQM_BUFFER_SIZE 32
#define AQMSerial Serial1

AirQualitySensor::AirQualitySensor()
    :   _pm1p0(0),
        _pm2p5(0),
        _pm10(0),
        _particleCount0p5um(0),
        _particleCount1p0um(0),
        _particleCount2p5um(0),
        _particleCount5p0um(0),
        _particleCount7p5um(0),
        _particleCount10um(0),
       _sensorStatus(0)
{


}

AirQualitySensor::~AirQualitySensor()
{

}

void AirQualitySensor::begin(void)
{
    // start hardware serial. RX is pin 33 on TinyPico. Don't really need TX.
    AQMSerial.begin(9600, SERIAL_8E1, 33, 32 );

    // The Panasonic SN-GCJA5 takes 28 seconds to get power up and normalize.
    // we will wait 28 seconds here.
    Serial.println(F("Waiting 28 seconds for sensor to power up and initialize"));
    delay(28000);
}

bool AirQualitySensor::updateSensorReading(void)
{
    uint8_t buffer[AQM_BUFFER_SIZE];
    int recieveCount = 0;
    while( AQMSerial.available() && (recieveCount < AQM_BUFFER_SIZE) ) {
        buffer[recieveCount] = AQMSerial.read();
        recieveCount++; 
    }
    if (recieveCount != AQM_BUFFER_SIZE) {
        Serial.print(F("ERROR: did not receive full data set from sensor. Bytes recieved = "));
        Serial.print(recieveCount);
        Serial.print(F("\n"));
        return false;
    }

    if ((buffer[0] != 0x02) || (buffer[AQM_BUFFER_SIZE-1] != 0x03)) {
        Serial.println(F("ERROR: data received from sensor did not have proper start or stop byte."));
        Serial.print(F("    Stary byte = 0x"));
        Serial.print(buffer[0], HEX);
        Serial.print(F(", stop byte = 0x"));
        Serial.print(buffer[AQM_BUFFER_SIZE-1], HEX);
        Serial.print(F("\n"));
        // This error likely occurs because we got out of synch with the sensor's internal update cycle.
        // We will read one more byte in order to slowly get into the right phase with the sensor.
        if (AQMSerial.available()) {
            AQMSerial.read();
        }
        return false;
    }

    // TODO confirm the XOR byte to ensure no transmission errors.
    
    Serial.print(F("    Received data = "));
    print_buffer(buffer, recieveCount);

    // calculate values
    _pm1p0 = (uint_fast32_t)buffer[4]*256*256*256 + (uint_fast32_t)buffer[3]*256*256 + (uint_fast32_t)buffer[2]*256 + buffer[1];
    _pm2p5 = (uint_fast32_t)buffer[8]*256*256*256 + (uint_fast32_t)buffer[7]*256*256 + (uint_fast32_t)buffer[6]*256 + buffer[5];
    _pm10 = (uint_fast32_t)buffer[12]*256*256*256 + (uint_fast32_t)buffer[11]*256*256 + (uint_fast32_t)buffer[10]*256 + buffer[9];
    _particleCount0p5um = (uint16_t)buffer[14]*256 + buffer[13];
    _particleCount1p0um = (uint16_t)buffer[16]*256 + buffer[15];
    _particleCount2p5um = (uint16_t)buffer[18]*256 + buffer[17];
    _particleCount5p0um = (uint16_t)buffer[22]*256 + buffer[21];
    _particleCount7p5um = (uint16_t)buffer[24]*256 + buffer[23];
    _particleCount10um = (uint16_t)buffer[26]*256 + buffer[25];

    _sensorStatus = buffer[29];

    Serial.print(F("    PM1.0 = "));
    Serial.print(_pm1p0);
    Serial.print(F(", PM2.5 = "));
    Serial.print(_pm2p5);
    Serial.print(F(", PM10 = "));
    Serial.print(_pm10);
    Serial.print(F("\n"));

    return true;
}

uint8_t AirQualitySensor::statusParticleDetector(void) const
{
    return (_sensorStatus&0x30) >> 4;
}

uint8_t AirQualitySensor::statusLaser(void) const
{
    return (_sensorStatus&0x0C) >> 2;
}

uint8_t AirQualitySensor::statusFan(void) const
{
    return (_sensorStatus&0x03);
}