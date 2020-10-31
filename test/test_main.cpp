#ifdef UNIT_TEST
#include <Arduino.h>
#include <unity.h>
#include "test_Utilities.h"
#include "test_AirQualitySensor.h"


void setup() {
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    UNITY_BEGIN();
    RUN_TEST(test_calculatePartialOrderedAverage);
    RUN_TEST(test_convertEpochToString);
    RUN_TEST(test_getAQIStatusColor);
    UNITY_END();
}

void loop() {
}

#endif