#ifdef UNIT_TEST
#include <Arduino.h>
#include <unity.h>
#include "AirQualitySensor.h"
#include "test_AirQualitySensor.h"

void test_getAQIStatusColor( void ) {
    TEST_ASSERT_EQUAL_INT(AQI_GREEN, AirQualitySensor::getAQIStatusColor(1));
    TEST_ASSERT_EQUAL_INT(AQI_GREEN, AirQualitySensor::getAQIStatusColor(50));
    TEST_ASSERT_EQUAL_INT(AQI_YELLOW, AirQualitySensor::getAQIStatusColor(51));
    TEST_ASSERT_EQUAL_INT(AQI_YELLOW, AirQualitySensor::getAQIStatusColor(100));
    TEST_ASSERT_EQUAL_INT(AQI_ORANGE, AirQualitySensor::getAQIStatusColor(101));
    TEST_ASSERT_EQUAL_INT(AQI_ORANGE, AirQualitySensor::getAQIStatusColor(150));
    TEST_ASSERT_EQUAL_INT(AQI_RED, AirQualitySensor::getAQIStatusColor(151));
    TEST_ASSERT_EQUAL_INT(AQI_RED, AirQualitySensor::getAQIStatusColor(200));
    TEST_ASSERT_EQUAL_INT(AQI_PURPLE, AirQualitySensor::getAQIStatusColor(201));
    TEST_ASSERT_EQUAL_INT(AQI_PURPLE, AirQualitySensor::getAQIStatusColor(300));
    TEST_ASSERT_EQUAL_INT(AQI_MAROON, AirQualitySensor::getAQIStatusColor(301));
    TEST_ASSERT_EQUAL_INT(AQI_MAROON, AirQualitySensor::getAQIStatusColor(500));
}

#endif