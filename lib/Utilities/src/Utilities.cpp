#include <time.h>
#include "Utilities.h"

void print_buffer( const uint8_t* buffer, uint8_t bufffer_size)
{
    Serial.print("{ ");
    for (int16_t i = 0; i < bufffer_size; i++ ) {
        Serial.print(F("0x"));
        Serial.print(buffer[i], HEX);
        if (i < bufffer_size - 1) {
            Serial.print(F(", "));
        } else {
            Serial.println(F(" }"));
        }
    }
}

float calculatePartialOrderedAverage( const Vector<uint16_t>& data, size_t start_idx, size_t number_of_values ) 
{
    // this averaging makes the grand assumption that all measurements suceed. That is, it 
    // does not account for measurement holes caused by intermitent sensor failures. For the
    // purposes of what this value is used for, which is to determine when the AQI is based on 
    // a 24 hour average, thiscaveat isn't really that important. Just acknowledging it exists.

    size_t curr_idx = start_idx;
    size_t value_count = 0;
    uint32_t running_sum = 0;

    while ((value_count < number_of_values) && (value_count < data.size())) {
        running_sum += data.at(curr_idx);
        value_count++;
        if (curr_idx == 0) {
            curr_idx = data.size() - 1;
        } else {
            curr_idx--;
        }
    }

    return (float)running_sum/(float)value_count;
}

String convertEpochToString(time_t epoch_time)
{
    struct tm  ts;
    char buf[80];

    ts = *localtime(&epoch_time);
    strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);

    return String(buf);
}