#include "PumpController.h"


#define calibration1_factor 728.951
#define calibration2_factor -674.231

#define LOADCELL1_DOUT_PIN  4
#define LOADCELL1_SCK_PIN  2
#define LOADCELL2_DOUT_PIN  23
#define LOADCELL2_SCK_PIN  5


void PumpController::beginController()
{
    m_scale1.begin(LOADCELL1_DOUT_PIN, LOADCELL1_SCK_PIN, 128U);
    m_scale2.begin(LOADCELL2_DOUT_PIN, LOADCELL2_SCK_PIN, 128U);
    m_scale1.set_scale(calibration1_factor);
    m_scale2.set_scale(calibration2_factor);
    m_scale1.tare(10);	//Reset the scale to 0
    m_scale2.tare(10);	//Reset the scale to 0

    // long zero_factor = scale1.read_average(); //Get a baseline reading
    Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
    Serial.println(m_scale1.get_offset());
    Serial.println(m_scale2.get_offset());
}

void PumpController::runController()
{
    readInputs();
    processController();
    writeOutputs();
}

void PumpController::readInputs()
{
    if ( m_tareRequested )
    {
        m_scale1.tare(10);	//Reset the scale to 0
        m_scale2.tare(10);	//Reset the scale to 0
        m_tareRequested = false;
        Serial.println("Taring");
    }

    m_weight_g = m_scale1.get_units(2) + m_scale2.get_units(2);
}

void PumpController::processController()
{

}

void PumpController::writeOutputs()
{

}
