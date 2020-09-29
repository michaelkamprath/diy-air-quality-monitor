#ifndef __Application__
#define __Application__
#include "AirQualitySensor.h"

class Application {
private:
    AirQualitySensor _sensor;

    void printLocalTime(void);

public:
    Application();
    virtual ~Application();

    void loop(void);
};


#endif // __Application__