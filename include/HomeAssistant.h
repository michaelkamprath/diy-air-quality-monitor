#ifndef __HomeAssistant__
#define __HomeAssistant__
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "Configuration.h"


class HomeAssistant {
private:
    Configuration& _config;
    WiFiClient _wifi;
    PubSubClient _client;
    String _stateTopic;
    String _deviceHash;

    void pupulateDeviceInformation(DynamicJsonDocument& json);
    void sendMQTTAQIDiscoveryMsg(void);
    void sendMQTTTempDiscoveryMsg(void);
public:
    HomeAssistant(
            Configuration& config
        );
    virtual ~HomeAssistant();

    void begin(bool hasBME680);
    void stop(void);
    void loop(void);

    void publishState(const String& stateJSONString);
};


#endif // __HomeAssistant__