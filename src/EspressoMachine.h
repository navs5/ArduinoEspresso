#ifndef _ESSPRESSO_MACHINE_H_
#define _ESSPRESSO_MACHINE_H_

#include "EspressoMachine_types.h"
#include "CloudStream.h"
#include "PumpController.h"

extern EspressoMachineNs::MachineCmdVals_S machineCmdVals;

class EspressoMachine
{
    public:
        EspressoMachine() = default;
        ~EspressoMachine() {}

        void begin();
        void runMachine100Hz();

    private:
        const String m_espressoMachineName = "espresso1";
        PumpController m_pumpController1 {machineCmdVals};
        CloudStream m_cloudStream1 {m_espressoMachineName,
                                    m_pumpController1};
};



#endif //_ESSPRESSO_MACHINE_H_