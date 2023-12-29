#ifndef FILTER_H
#define FILTER_H

#include <Arduino.h>

#define LFP_GAIN_FROM_TAU(Tau, Fs)  (1.0F / (1.0F + ((Tau) * (Fs))))
#define LFP_GAIN_FROM_FC(Fc, Fs)    LFP_GAIN_FROM_TAU(1.0F / (TWO_PI * (Fc)), (Fs))

#define LPF_RUN(state, newVal, gain)  (state + (gain * (newVal - state)))


#endif // FILTER_H