#include <Arduino.h>
#include "AirQualitySensor.h"

AirQualitySensor* sensor = nullptr;

void setup()
{
  // Used for debug output only
  Serial.begin(9600);
  sensor = new AirQualitySensor();
}

#define AQM_BUFFER_SIZE 32

void loop()
{
  sensor->updateSensorReading();

  delay(10000);
}