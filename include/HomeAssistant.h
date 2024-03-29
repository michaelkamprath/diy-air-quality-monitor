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
    String _commandTopic;
    String _ledBrightnessTopic;
    String _deviceHash;

    void populateDeviceInformation(DynamicJsonDocument& json);
    void reconnectClient(void);
    void sendSensorDiscoveryMessage(
            String name,
            String entity_prefix,
            String device_class,
            String icon,
            String value_template,
            String unit_of_measurement = ""
        );
    void sendLEDBrightnessDiscoveryMessage(void);
    void sendDeviceDiscoveryMsgs(bool hasBME680);

    void mqttCallback(char* topic, byte* message, unsigned int length);
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