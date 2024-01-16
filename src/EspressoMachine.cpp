#include "EspressoMachine.h"

EspressoMachineNs::MachineCmdVals_S machineCmdVals {};

void EspressoMachine::begin()
{
    // Brew timer should be paused by default
    machineCmdVals.brewTimerPause = true;

    m_cloudStream1.begin();
    m_pumpController1.beginController();
}

void EspressoMachine::runMachine1kHz()
{
    // Run pump controller for regulated pressure and weight
    m_pumpController1.runController1khz();
}

void EspressoMachine::runMachine100Hz()
{
    // Run pump controller for regulated pressure and weight
    m_pumpController1.runController();

    // Run module to publish machine data to server
    m_cloudStream1.runCloudStream();
}
