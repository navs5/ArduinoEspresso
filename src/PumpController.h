#ifndef _PUMP_CONTROLLER_H_
#define _PUMP_CONTROLLER_H_

#include "HX711.h"
#include "SwTimer.h"
#include "EspressoMachine_types.h"
#include <queue>

#define TARGET_WEIGHT_QUAL_TIME_MS     (300)
#define BREW_TIMER_MAX_LEN_MS          (MIN_TO_MS(5U))
#define TIMER_TIC_PERIOD_MS            (10U)

struct AlertPayload
{
    std::string alertName;
    std::string alertPayload;
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
                printf("added alert, size of queue now: %d\n", m_alertsQueue.size());
            }
            else
            {
                printf("Alert queue filled up!\n");
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

                printf("Getting alert, size of queue now: %d, is empty: %d\n", m_alertsQueue.size(), m_alertsQueue.empty());
            }
            else
            {
                printf("Already empty\n\n");
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
        void runController();

        float getWeight() const
        {
            return (m_weight_g);
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

        EspressoMachineNs::MachineCmdVals_S& m_machineCmdVals;
        static constexpr uint32_t m_kMaxTareCount {10U};  // Number of values to average for taring
        SwUpTimer m_brewTimer {BREW_TIMER_MAX_LEN_MS, TIMER_TIC_PERIOD_MS};
        SwDownTimer m_targetWeightQualTimer {TARGET_WEIGHT_QUAL_TIME_MS, TIMER_TIC_PERIOD_MS};
        float m_weight1_g {0.0F};
        float m_weight2_g {0.0F};
        float m_weight_g {0.0F};
        float m_offsetWeight1_g {0.0F};
        float m_offsetWeight2_g {0.0F};
        uint32_t m_tareCount {0U};
        bool m_tareRequested {false};
        bool m_targetWeightAlertSet {false};
        HX711 m_scale1 {};
        HX711 m_scale2 {};
};



#endif  // _PUMP_CONTROLLER_H_