#ifndef _CLOUD_STREAM_H_
#define _CLOUD_STREAM_H_

#include "SwTimer.h"
#include "PumpController.h"
#include "EspressoMachine_types.h"

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

#define SENSOR_VALS_CYCLE_PERIOD_MS    (500U)
#define MACHINE_INFO_CYCLE_PERIOD_MS   (15000)
#define TIMER_TIC_PERIOD_MS            (10U)


class CloudStream
{
    public:
        CloudStream(const std::string nodeName,
                    PumpController& pumpController):
                                                m_nodeName(nodeName),
                                                m_pumpController(pumpController) {}
        ~CloudStream() {}

        void begin();
        void runCloudStream();

    private:
        void reconnect();
        void packageSensorData(JsonDocument& jsonDoc);
        void packageInfoData(JsonDocument& jsonDoc);
        void processCommandRequests(void);
        void postAlerts();
        void postPeriodicMessages();

        WiFiClient m_espWifiClient {};
        PubSubClient m_client {m_espWifiClient};
        SwDownTimer m_sensorValsTimer {SENSOR_VALS_CYCLE_PERIOD_MS, TIMER_TIC_PERIOD_MS};
        SwDownTimer m_infoTimer {MACHINE_INFO_CYCLE_PERIOD_MS, TIMER_TIC_PERIOD_MS};
        static constexpr size_t m_maxMsgsPerRun {1U};
        size_t m_msgsSentCurrentRun {0U};
        const std::string m_nodeName;
        PumpController& m_pumpController;
};



#endif  // _CLOUD_STREAM_H_