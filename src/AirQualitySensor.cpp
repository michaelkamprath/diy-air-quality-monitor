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
        _sensorStatus(0)
{
    // start hardware serial. RX is pin 33 on TinyPico. Don't really need TX.
    AQMSerial.begin(9600, SERIAL_8E1, 33, 32 );

    // The Panasonic SN-GCJA5 takes 28 seconds to get power up and normalize.
    // we will wait 28 seconds here.
    Serial.println(F("Waiting 28 seconds for sensor to power up and initialize"));
    delay(28000);

}

AirQualitySensor::~AirQualitySensor()
{

}

bool AirQualitySensor::updateSensorReading(void)
{
    Serial.println(F("Fetching current sensor data."));
    uint8_t buffer[AQM_BUFFER_SIZE];
    int recieveCount = 0;
    while( Serial1.available() && (recieveCount < AQM_BUFFER_SIZE) ) {
        buffer[recieveCount] = Serial1.read();
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
        return false;
    }

    // TODO check XOR 
    Serial.print(F("    Received data = "));
    print_buffer(buffer, recieveCount);

    // calculate values
    _pm1p0 = (uint_fast32_t)buffer[4]*256*256*256 + (uint_fast32_t)buffer[3]*256*256 + (uint_fast32_t)buffer[2]*256 + buffer[1];
    _pm2p5 = (uint_fast32_t)buffer[8]*256*256*256 + (uint_fast32_t)buffer[7]*256*256 + (uint_fast32_t)buffer[6]*256 + buffer[5];
    _pm10 = (uint_fast32_t)buffer[12]*256*256*256 + (uint_fast32_t)buffer[11]*256*256 + (uint_fast32_t)buffer[10]*256 + buffer[9];

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