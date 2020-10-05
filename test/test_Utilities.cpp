#ifdef UNIT_TEST
#include <Arduino.h>
#include <unity.h>
#include "Utilities.h"
#include "test_Utilities.h"

void test_calculatePartialOrderedAverage( void ) {
    uint16_t test_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 20};
    Vector<uint16_t> test_data(test_array, 10);

    // Test 1 - Simplec case. Full vector is used from end to beginging.
    float test1_result = calculatePartialOrderedAverage(test_data, (size_t)9, (size_t)10);
    TEST_ASSERT_EQUAL_FLOAT( 5.6,test1_result);

    // Test 2 - Same data, start in middle
    float test2_result = calculatePartialOrderedAverage(test_data, (size_t)5, (size_t)10);
    TEST_ASSERT_EQUAL_FLOAT( 5.6,test2_result);

    // Test 3 - partial average with no scrolling
    float test3_result = calculatePartialOrderedAverage(test_data, (size_t)7, (size_t)6);
    TEST_ASSERT_EQUAL_FLOAT( 4.5,test3_result);

    // Test 4 - partial average with scrolling
    float test4_result = calculatePartialOrderedAverage(test_data, (size_t)1, (size_t)5);
    TEST_ASSERT_EQUAL_FLOAT( 7.2,test4_result);

}

#endif