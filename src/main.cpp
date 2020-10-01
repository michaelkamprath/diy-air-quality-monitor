#include <Arduino.h>
#include "Application.h"

void setup()
{
  // Used for debug output only
  Serial.begin(9600);
  
  Application::getInstance()->setup();
}


uint32_t loopCounter = 0;
void loop()
{
  Application::getInstance()->loop();
}

