#ifndef __Application__
#define __Application__
#include "AirQualitySensor.h"

class Application {
private:
    time_t _last_update_time;
    AirQualitySensor _sensor;

    void printLocalTime(void);

public:
    Application();
    virtual ~Application();

    void loop(void);
};


#endif // __Application__