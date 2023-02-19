#ifndef __Utilities__
#define __Utilities__
#include <Arduino.h>
#include <Vector.h>

// Prints the passed buffer to serial in a human-readable format
void print_buffer( const uint8_t* buffer, uint8_t size);

// Calculates an average of a given subset of values of in Vector<uint16_t>, The averaging starts
// with the value at start_idx, and then decrements the index to average in more values until a 
// total of number_of_values or the full contents of the Vector are used. If the decrementing index 
// hits zero before the target number of values is achieved, then the idex will roll over oto the last
// item in the Vector and continue from there.
float calculatePartialOrderedAverage( const Vector<uint16_t>& data, size_t start_idx, size_t number_of_values );

String convertEpochToString(time_t epoch_time);

// returns the HTTP content type based on filename extension
String getContentType(String filename);

#endif // __Utilities__