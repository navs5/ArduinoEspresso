#ifndef _CLOUD_STREAM_H_
#define _CLOUD_STREAM_H_

#include "SwTimer.h"

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

class CloudStream
{
    public:
        CloudStream(const String nodeName, 
                    std::function<void(char*, uint8_t*, unsigned int)> callback,
                    SwDownTimer& sensorValsTimer, 
                    SwDownTimer& infoTimer) : 
                                              m_nodeName(nodeName),
                                              m_callback(callback),
                                              m_sensorValsTimer(sensorValsTimer),
                                              m_infoTimer(infoTimer),
                                              m_espWifiClient(),
                                              m_client(m_espWifiClient) {}
        ~CloudStream() {}

        void begin();
        void runCloudStream(JsonDocument& sensorJsonDoc, JsonDocument& infoJsonDoc);
        void setCallback(std::function<void(char*, uint8_t*, unsigned int)> callback)
        {
            m_callback = callback;
        }

    private:
        void reconnect();

        const String m_nodeName; 
        std::function<void(char*, uint8_t*, unsigned int)> m_callback;
        SwDownTimer& m_sensorValsTimer;
        SwDownTimer& m_infoTimer;
        WiFiClient m_espWifiClient;
        PubSubClient m_client;
};



#endif  // _CLOUD_STREAM_H_