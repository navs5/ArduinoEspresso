#ifndef _PUMP_CONTROLLER_H_
#define _PUMP_CONTROLLER_H_

#include "HX711.h"
#include "SwTimer.h"
#include "EspressoMachine_types.h"

#define BREW_TIMER_MAX_LEN_MS          (MIN_TO_MS(5U))
#define TIMER_TIC_PERIOD_MS            (10U)

class Controller
{
    public:
        virtual void beginController() = 0;
        virtual void runController() = 0;
};

class PumpController : public Controller
{
    public:
        PumpController(EspressoMachineNs::MachineCmdVals_S& machineCmdVals):
                        m_machineCmdVals(machineCmdVals) {}

        ~PumpController() {}

        void beginController();
        void runController();

        float getWeight() const
        {
            return (m_weight1_g+m_weight2_g);
        }

        bool shotComplete() const
        {
            return ((m_weight1_g+m_weight2_g) >= m_targetWeight_g);
        }

        void setTareRequest()
        {
            m_tareRequested = true;
        }

        void setTargetWeight(float weight_g)
        {
            weight_g = m_targetWeight_g;
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

        EspressoMachineNs::MachineCmdVals_S& m_machineCmdVals;
        static constexpr uint32_t m_kMaxTareCount {10U};  // Number of values to average for taring
        SwUpTimer m_brewTimer {BREW_TIMER_MAX_LEN_MS, TIMER_TIC_PERIOD_MS};
        float m_weight1_g {0.0F};
        float m_weight2_g {0.0F};
        float m_targetWeight_g {0.0F};
        float m_offsetWeight1_g {0.0F};
        float m_offsetWeight2_g {0.0F};
        uint32_t m_tareCount {0U};
        bool m_tareRequested {false};
        HX711 m_scale1 {};
        HX711 m_scale2 {};
};



#endif  // _PUMP_CONTROLLER_H_