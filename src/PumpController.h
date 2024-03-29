#ifndef _PUMP_CONTROLLER_H_
#define _PUMP_CONTROLLER_H_

#include "HX711.h"
#include "SwTimer.h"
#include "EspressoMachine_types.h"
#include "lookupTable.hpp"
#include "thermistorTables.h"
#include "Config.h"
#include <queue>

#define PUMP_MODULE_PERIOD_MS          (10U)            // Period of pump module
#define PREFUSION_END_QUAL_COUNT       (3U)             // Prefusion will end after this many consecutive counts of required weight
#define TARGET_WEIGHT_QUAL_TIME_MS     (300)            // Post target weight time after weight is reached for this amount of time
#define PUMP_ON_QUAL_TIME_MS           (500U)           // Pump will be considered on after current is above threshold for this duration
#define BREW_TIMER_MAX_LEN_MS          (MIN_TO_MS(5U))  // Max amount of time to run brew timer
#define INIT_PUMP_FULL_POWER_TIME_MS   (S_TO_MS(15U))   // Amount of time to run pump at full power out of power up
#define PUMP_PREFUSION_TO_BREW_TIME_MS (500U)           // Time it takes to transition pump power from prefusion level to brew level
#define PUMP_BREW_TO_PREFUSION_TIME_MS (500U)           // Time it takes to transition pump power from brew level to prefusion level
#define PUMP_DUTY_UPDATE_PERIOD_MS     (1000U)          // Time it takes to update pump pwm to new value

struct AlertPayload
{
    std::string alertName;
    std::string alertPayload;
};

enum PumpCtlrState
{
    INIT,
    PREFUSION,
    BREW,
};

class Controller
{
    public:
        void beginController() {};
        void runController() {};

        bool alertPresent()
        {
            return (m_alertsQueue.size() > 0U);
        }

        void addAlert(AlertPayload& newAlert)
        {
            if ( m_alertsQueue.size() < m_maxAlerts )
            {
                m_alertsQueue.push(newAlert);
#if (DEBUG_MODE == 1U)
                printf("added alert, size of queue now: %d\n", m_alertsQueue.size());
#endif
            }
            else
            {
#if (DEBUG_MODE == 1U)
               printf("Alert queue filled up!\n");
#endif
            }
        }

        bool getAlert(AlertPayload& retrievedAlert)
        {
            bool alertPresent = false;

            if (!m_alertsQueue.empty())
            {
                alertPresent = true;
                retrievedAlert = m_alertsQueue.front();
                m_alertsQueue.pop();
#if (DEBUG_MODE == 1U)
                printf("Getting alert, size of queue now: %d, is empty: %d\n", m_alertsQueue.size(), m_alertsQueue.empty());
#endif
            }
            else
            {
#if (DEBUG_MODE == 1U)
                printf("Already empty\n\n");
#endif
            }

            return alertPresent;
        }

    private:
        std::queue<AlertPayload> m_alertsQueue;
        size_t m_maxAlerts {10U};
};

class PumpController final : public Controller
{
    public:
        PumpController(EspressoMachineNs::MachineCmdVals_S& machineCmdVals):
                        m_machineCmdVals(machineCmdVals) {}

        ~PumpController() {}

        void beginController();
        void runController1khz();
        void runController();

        float getWeight() const
        {
            return (m_weight_g);
        }

        float getTemperatureTank() const
        {
            return m_tankTemp_C;
        }

        float getTemperatureTankOutlet() const
        {
            return m_tankOutletTemp_C;
        }

        float getPumpDuty() const
        {
            return ((m_pumpPwm / (float)EspressoConfig::maxDuty_pumpCtlr) * 100.0F);
        }

        float getTemperatureTankReturn() const
        {
            return m_tankReturnTemp_C;
        }

        float getPumpCurrent() const
        {
            return m_pumpCurrentRms_A;
        }

        void setTareRequest()
        {
            m_tareRequested = true;
        }

        void brewTimerPause(void)
        {
            m_brewTimer.pause();
        }

        void brewTimerResume(void)
        {
            m_brewTimer.resume();
        }

        void brewTimerReset(void)
        {
            m_brewTimer.reset();
        }

        uint32_t brewTimerGetCount(void)
        {
            return m_brewTimer.getCount();
        }

    private:
        void readInputs();
        void processCommandRequests();
        void processController();
        void writeOutputs();
        void processAlerts();
        void autoRunBrewTimer();

        EspressoMachineNs::MachineCmdVals_S& m_machineCmdVals;
        Table1D thermistorLut {&thermTable_ntc3950_100K[0], sizeof(thermTable_ntc3950_100K)/sizeof(thermTable_ntc3950_100K[0])};
        static constexpr uint32_t m_kMaxTareCount {10U};  // Number of values to average for taring
        SwUpTimer m_brewTimer {BREW_TIMER_MAX_LEN_MS, PUMP_MODULE_PERIOD_MS};
        SwDownTimer m_targetWeightQualTimer {TARGET_WEIGHT_QUAL_TIME_MS, PUMP_MODULE_PERIOD_MS};
        SwDownTimer m_pumpOnQualTimer {PUMP_ON_QUAL_TIME_MS, PUMP_MODULE_PERIOD_MS};
        SwDownTimer m_initFullPowerTimer {INIT_PUMP_FULL_POWER_TIME_MS, PUMP_MODULE_PERIOD_MS};
        uint32_t m_prefusionEndQualCount {0U};
        float m_pumpRateChange {0.0F};
        float m_weight1_g {0.0F};
        float m_weight2_g {0.0F};
        float m_weight_g {0.0F};
        float m_offsetWeight1_g {0.0F};
        float m_offsetWeight2_g {0.0F};
        float m_pumpPwm {EspressoConfig::maxDuty_pumpCtlr};
        float m_pumpPwmTarget {EspressoConfig::maxDuty_pumpCtlr};
        uint32_t m_tareCount {0U};
        bool m_tareRequested {false};
        bool m_targetWeightAlertSet {false};
        bool m_prevPrefusionCmd {true};
        PumpCtlrState m_pumpState {PumpCtlrState::INIT};
        HX711 m_scale1 {};
        HX711 m_scale2 {};
        float m_tankTemp_C {25.0F};
        float m_tankOutletTemp_C {25.0F};
        float m_tankReturnTemp_C {25.0F};
        float m_pumpCurrent_A {0.0F};
        float m_pumpCurrentRms_A {0.0F};
        bool m_pumpOn {false};
        bool m_pumpPrevOn {false};
        bool m_prevBrewTimerCmd {false};
};



#endif  // _PUMP_CONTROLLER_H_