#include "PumpController.h"


#define calibration1_factor 728.951
#define calibration2_factor -674.231

#define LOADCELL1_DOUT_PIN   4
#define LOADCELL1_SCK_PIN    2
#define LOADCELL2_DOUT_PIN   23
#define LOADCELL2_SCK_PIN    5


void PumpController::beginController()
{
    m_scale1.begin(LOADCELL1_DOUT_PIN, LOADCELL1_SCK_PIN, 128U);
    m_scale2.begin(LOADCELL2_DOUT_PIN, LOADCELL2_SCK_PIN, 128U);
    m_scale1.set_scale(calibration1_factor);
    m_scale2.set_scale(calibration2_factor);
    m_scale1.tare(10U);	//Reset the scale to 0
    m_scale2.tare(10U);	//Reset the scale to 0

    m_brewTimer.pause();

    // long zero_factor = scale1.read_average(); //Get a baseline reading
    Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
    Serial.println(m_scale1.get_offset());
    Serial.println(m_scale2.get_offset());
}

void PumpController::runController()
{
    readInputs();
    processCommandRequests();
    processController();
    writeOutputs();
}

void PumpController::readInputs()
{
    // Read from the scales
    float weight1_g {0.0F};
    float weight2_g {0.0F};
    bool validRead1 = m_scale1.get_units_nonblocking(weight1_g);
    bool validRead2 = m_scale2.get_units_nonblocking(weight2_g);

    if ( validRead1 )
    {
        m_weight1_g = weight1_g;
    }
    if ( validRead2 )
    {
        m_weight2_g = weight2_g;
    }

    // if ( m_tareRequested )
    // {
    //     if ( validReads )
    //     {
    //         m_offsetWeight1_g += weight1_g;
    //         m_offsetWeight2_g += weight2_g;

    //         m_tareCount++;
    //     }
        
    //     if ( m_tareCount >= m_kMaxTareCount )
    //     {
    //         m_offsetWeight1_g /= m_tareCount;
    //         m_offsetWeight2_g /= m_tareCount;            
    //         m_scale1.set_offset(m_offsetWeight1_g);
    //         m_scale2.set_offset(m_offsetWeight2_g);

    //         m_tareRequested = false;
    //     }
    //     Serial.println("Taring");
    // }
    // else
    // {
    //     m_offsetWeight1_g = 0.0F;
    //     m_offsetWeight2_g = 0.0F;
    //     m_tareCount = 0U;
    // }
}

void PumpController::processCommandRequests()
{
    if ( m_machineCmdVals.tareRequest )
    {
        m_scale1.tare(10U);
        m_scale2.tare(10U);
        m_machineCmdVals.tareRequest = false;
    }

    if ( m_machineCmdVals.brewTimerPause )
    {
        m_brewTimer.pause();
    }
    else
    {
        m_brewTimer.resume();
    }

    if ( m_machineCmdVals.brewTimerReset )
    {
        m_brewTimer.pause();
        m_brewTimer.reset();
        m_machineCmdVals.brewTimerPause = true;
        m_machineCmdVals.brewTimerReset = false;
    }
}

void PumpController::processController()
{
    // Run the brew shot length timer
    m_brewTimer.updateAndCheckTimer();
    // Serial.printf("count: %d\n", m_brewTimer.getCount());

}

void PumpController::writeOutputs()
{

}
