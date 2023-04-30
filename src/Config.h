#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>

namespace EspressoConfig
{
    constexpr uint8_t pin_pumpCtlr = 25U;
    constexpr uint8_t pwmChannel_pumpCtlr = 0U;
    constexpr double pwmFreq_pumpCtlr = 1000U;
    constexpr uint8_t pwmResolution_pumpCtlr = 8U;

    constexpr uint16_t maxDuty_pumpCtlr = ((1U << pwmResolution_pumpCtlr) - 1U);
    constexpr uint16_t prefusionDuty_pumpCtlr = (0.3F * maxDuty_pumpCtlr);
    constexpr uint16_t offDuty_pumpCtlr = (0U);

    constexpr char mdnsHostName[] = "espresso";

} // namespace EspressoConfig


#endif // _CONFIG_H_