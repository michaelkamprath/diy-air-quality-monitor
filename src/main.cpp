#include <Arduino.h>
#include "Application.h"

void setup()
{
  // Used for debug output only
  Serial.begin(9600);
  // wait some time for the serial conenction to stabilize.
  delay(500);
  Serial.println(F("\nStarting DIY Air Quality Monitor ..."));
  Application::getInstance()->setup();
}


uint32_t loopCounter = 0;
void loop()
{
  Application::getInstance()->loop();
}

