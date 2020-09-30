#include <Arduino.h>
#include "Application.h"

Application* app = nullptr;

void setup()
{
  // Used for debug output only
  Serial.begin(9600);
  app = new Application();

}


uint32_t loopCounter = 0;
void loop()
{
  loopCounter++;

  // slow down the call to the app loop some. 
  if (loopCounter%1000 == 0) {
    app->loop();
  }
}

