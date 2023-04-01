#ifndef _ESSPRESSO_MACHINE_H_
#define _ESSPRESSO_MACHINE_H_

#include "CloudStream.h"
#include "PumpController.h"
#include "SwTimer.h"

#define BREW_TIMER_MAX_LEN_MS          (MIN_TO_MS(5U))
#define TIMER_TIC_PERIOD_MS            (10U)

struct MachineCmdVals
{
    bool tareRequest;
    bool brewTimerPause;
    bool brewTimerReset;
    uint32_t setPressure_bar;
    uint32_t setTemperature_C;
    uint32_t setWeight_g;
};

class EspressoMachine
{
    public:
        EspressoMachine() = default;
        ~EspressoMachine() {}

        void begin();
        void runMachine100Hz();

        static MachineCmdVals m_machineCmdVals;

    private:
        static void commandsCallback(char* topic, uint8_t* message, unsigned int length);
        void processCommandRequests();
        void packageSensorData(JsonDocument& jsonDoc);
        void packageInfoData(JsonDocument& jsonDoc);

        const String m_espressoMachineName = "espresso1";
        SwUpTimer m_brewTimer {BREW_TIMER_MAX_LEN_MS, TIMER_TIC_PERIOD_MS};
        PumpController m_pumpController1 {};
        CloudStream m_cloudStream1 {m_espressoMachineName, commandsCallback,
                                    m_brewTimer, m_pumpController1};
};



#endif //_ESSPRESSO_MACHINE_H_