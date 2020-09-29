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