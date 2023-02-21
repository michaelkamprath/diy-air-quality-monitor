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

String getContentType(String filename)
{
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) { res += String((ip >> (8 * i)) & 0xFF) + "."; }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

String mac2String(byte ar[]) {
  String s;
  for (byte i = 0; i < 6; ++i)
  {
    char buf[3];
    sprintf(buf, "%02X", ar[i]); // J-M-L: slight modification, added the 0 in the format for padding 
    s += buf;
    // if (i < 5) s += ':';
  }
  return s;
}