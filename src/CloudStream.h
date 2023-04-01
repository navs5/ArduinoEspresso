#ifndef _CLOUD_STREAM_H_
#define _CLOUD_STREAM_H_

#include "SwTimer.h"
#include "PumpController.h"

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

#define SENSOR_VALS_CYCLE_PERIOD_MS    (250U)
#define MACHINE_INFO_CYCLE_PERIOD_MS   (10000)
#define TIMER_TIC_PERIOD_MS            (10U)

class CloudStream
{
    public:
        CloudStream(const String nodeName, 
                    std::function<void(char*, uint8_t*, unsigned int)> callback,
                    const SwUpTimer& brewTimer,
                    const PumpController& pumpController) : 
                                                            m_nodeName(nodeName),
                                                            m_callback(callback),
                                                            m_sensorValsTimer(SENSOR_VALS_CYCLE_PERIOD_MS, TIMER_TIC_PERIOD_MS),
                                                            m_infoTimer(MACHINE_INFO_CYCLE_PERIOD_MS, TIMER_TIC_PERIOD_MS),
                                                            m_brewTimer(brewTimer),
                                                            m_espWifiClient(),
                                                            m_client(m_espWifiClient),
                                                            m_pumpController(pumpController) {}
        ~CloudStream() {}

        void begin();
        void runCloudStream();
        void setCallback(std::function<void(char*, uint8_t*, unsigned int)> callback)
        {
            m_callback = callback;
        }

    private:
        void reconnect();
        void packageSensorData(JsonDocument& jsonDoc);
        void packageInfoData(JsonDocument& jsonDoc);

        const String m_nodeName; 
        std::function<void(char*, uint8_t*, unsigned int)> m_callback;
        SwDownTimer m_sensorValsTimer;
        SwDownTimer m_infoTimer;
        const SwUpTimer& m_brewTimer;
        WiFiClient m_espWifiClient;
        PubSubClient m_client;
        const PumpController& m_pumpController;
};



#endif  // _CLOUD_STREAM_H_