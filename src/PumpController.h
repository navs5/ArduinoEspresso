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
        PumpController() : m_weight_g(0.0F),
                           m_targetWeight_g(0.0F),
                           m_tareRequested(false) {}
        ~PumpController() {}

        void beginController();
        void runController();

        float getWeight() const
        {
            return m_weight_g;
        }

        bool shotComplete() const
        {
            return (m_weight_g >= m_targetWeight_g);
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

        float m_weight_g;
        float m_targetWeight_g;
        float m_tareRequested;
        HX711 m_scale1;
        HX711 m_scale2;
};



#endif  // _PUMP_CONTROLLER_H_