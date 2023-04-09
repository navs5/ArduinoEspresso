#ifndef _ESPRESSO_MACHINE_TYPES_H_
#define _ESPRESSO_MACHINE_TYPES_H_

#include <stdint.h>

namespace EspressoMachineNs 
{

struct MachineCmdVals_S
{
    bool tareRequest;
    bool brewTimerPause;
    bool brewTimerReset;
    uint32_t targetPressure_bar;
    uint32_t targetTemperature_C;
    uint32_t targetWeight_g;
};

}  // namespace EspressoMachine

#endif  // _ESPRESSO_MACHINE_TYPES_H_