#include <Arduino.h>
#include "Application.h"

Application* app = nullptr;

void setup()
{
  // Used for debug output only
  Serial.begin(9600);
  app = new Application();

}



void loop()
{
  app->loop();

  delay(60000);
}

