#include <Arduino.h>
#include "Application.h"

void setup()
{
  // Used for debug output only
  Serial.begin(9600);
  // wait some time for the serial conenction to stabilize.
  delay(500);
  Serial.print(F("\nStarting DIY Air Quality Monitor ("));
  Serial.print(F(SENSOR_NAME));
  Serial.println(F(")..."));
  Application::getInstance()->setup();
}


uint32_t loopCounter = 0;
void loop()
{
  Application::getInstance()->loop();
}

