#ifndef _PUMP_CONTROLLER_H_
#define _PUMP_CONTROLLER_H_

#include "HX711.h"

class Controller
{
    public:
        virtual void beginController() = 0;
        virtual void runController() = 0;
};

class PumpController : public Controller
{
    public:
        PumpController() {}
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

    private:
        void readInputs();
        void processController();
        void writeOutputs();

        static constexpr uint32_t m_kMaxTareCount {10U};  // Number of values to average for taring
        float m_weight1_g {0.0F};
        float m_weight2_g {0.0F};
        float m_targetWeight_g {0.0F};
        float m_offsetWeight1_g {0.0F};
        float m_offsetWeight2_g {0.0F};
        uint32_t m_tareCount {0U};
        bool m_tareRequested {false};
        HX711 m_scale1;
        HX711 m_scale2;
};



#endif  // _PUMP_CONTROLLER_H_