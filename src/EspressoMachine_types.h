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
    bool targetValsUpdated;
    float targetPressure_bar;
    float targetTemperature_C;
    float targetWeight_g;
};

}  // namespace EspressoMachine

#endif  // _ESPRESSO_MACHINE_TYPES_H_