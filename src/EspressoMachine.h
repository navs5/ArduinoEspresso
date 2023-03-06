#ifndef _ESSPRESSO_MACHINE_H_
#define _ESSPRESSO_MACHINE_H_

#include "CloudStream.h"
#include "PumpController.h"
#include "SwTimer.h"

#define BREW_TIMER_MAX_LEN_MS          (MIN_TO_MS(5U))
#define SENSOR_VALS_CYCLE_PERIOD_MS    (250U)
#define MACHINE_INFO_CYCLE_PERIOD_MS   (10000)
#define TIMER_TIC_PERIOD_MS            (1U)

struct MachineCmdVals
{
    bool tareRequest;
    bool brewTimerPause;
    bool brewTimerReset;
    uint32_t setPressure_bar;
    uint32_t setTemperature_C;
    uint32_t setWeight_g;
    MachineCmdVals() : tareRequest(false),
                       brewTimerPause(true),
                       brewTimerReset(false),
                       setPressure_bar(0.0F),
                       setTemperature_C(0.0F),
                       setWeight_g(0.0F) {}
};

class EspressoMachine
{
    public:
        EspressoMachine() : m_brewTimer(BREW_TIMER_MAX_LEN_MS, TIMER_TIC_PERIOD_MS),
                            m_sensorValsTimer(SENSOR_VALS_CYCLE_PERIOD_MS, TIMER_TIC_PERIOD_MS),
                            m_infoTimer(MACHINE_INFO_CYCLE_PERIOD_MS, TIMER_TIC_PERIOD_MS),
                            m_pumpController1(),
                            m_cloudStream1(m_espressoMachineName, commandsCallback,
                                            m_sensorValsTimer, m_infoTimer) {}

        ~EspressoMachine() {}

        void begin();
        void runMachine1kHz();

        static MachineCmdVals m_machineCmdVals;

    private:
        static void commandsCallback(char* topic, uint8_t* message, unsigned int length);
        void processCommandRequests();
        void packageSensorData(JsonDocument& jsonDoc);
        void packageInfoData(JsonDocument& jsonDoc);

        const String m_espressoMachineName = "espresso1";
        SwUpTimer m_brewTimer;
        SwDownTimer m_sensorValsTimer;
        SwDownTimer m_infoTimer;
        PumpController m_pumpController1;
        CloudStream m_cloudStream1;
};



#endif //_ESSPRESSO_MACHINE_H_