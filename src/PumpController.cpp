#include "PumpController.h"
#include <ArduinoJson.h>


#define calibration1_factor (728.951F)
#define calibration2_factor (-674.231F)

#define LOADCELL1_DOUT_PIN   4
#define LOADCELL1_SCK_PIN    2
#define LOADCELL2_DOUT_PIN   23
#define LOADCELL2_SCK_PIN    5

#define PREFUSION_END_WEIGHT_G 3U  // Weight at which prefusion will turn off


float calculateRateChange(float startValue, float endValue, float timeLength)
{
    return ((endValue - startValue) / std::abs(timeLength));
}

float saturate(float value, float minLimit, float maxLimit)
{
    return (std::min(std::max(value, minLimit), maxLimit));
}

float rateLimit(float currentValue, float endValue, float rateChange, float dt, float minLimit, float maxLimit)
{
    float unsaturatedVal = (currentValue + (rateChange * dt));

    // Saturate so don't go beyond end value
    unsaturatedVal = (rateChange >= 0.0F) ? std::min(unsaturatedVal, endValue) : std::max(unsaturatedVal, endValue);

    return saturate(unsaturatedVal, minLimit, maxLimit);
}

void PumpController::beginController()
{
    m_scale1.begin(LOADCELL1_DOUT_PIN, LOADCELL1_SCK_PIN, 128U);
    m_scale2.begin(LOADCELL2_DOUT_PIN, LOADCELL2_SCK_PIN, 128U);
    m_scale1.set_scale(calibration1_factor);
    m_scale2.set_scale(calibration2_factor);
    m_scale1.tare(10U);	//Reset the scale to 0
    m_scale2.tare(10U);	//Reset the scale to 0

    m_brewTimer.pause();
    m_pumpState = PumpCtlrState::INIT;

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
    processAlerts();
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
    if ( validRead1 && validRead2 )
    {
        m_weight_g = m_weight1_g + m_weight2_g;
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

        // Clear target reached time
        StaticJsonDocument<50> alertJsonDoc;
        alertJsonDoc["tt_ms"] = 0U;
        size_t bufferSize_bytes = alertJsonDoc.memoryUsage();
        char serializedData[bufferSize_bytes];
        serializeJson(alertJsonDoc, serializedData, bufferSize_bytes);
        m_targetWeightAlertSet = false;
        AlertPayload clearAlert = {.alertName="targetWeightReached", .alertPayload=serializedData};
        addAlert(clearAlert);
    }
}

void PumpController::processController()
{
    // Run the brew shot length timer
    m_brewTimer.updateAndCheckTimer();

    switch (m_pumpState)
    {
        case PumpCtlrState::INIT:
        {
            // Allow time for tank to fill up
            m_pumpPwm = EspressoConfig::maxDuty_pumpCtlr;
            if (m_initFullPowerTimer.updateAndCheckTimer())
            {
                if (m_machineCmdVals.prefusionEnable)
                {
                    m_pumpRateChange = calculateRateChange(EspressoConfig::maxDuty_pumpCtlr, EspressoConfig::prefusionDuty_pumpCtlr, PUMP_BREW_TO_PREFUSION_TIME_MS);
                    m_pumpState = PumpCtlrState::PREFUSION;
                }
                else
                {
                    // No rate change required. Going from full power to full power
                    m_pumpRateChange = 0.0F;
                    m_pumpState = PumpCtlrState::BREW;
                }
                m_initFullPowerTimer.reset();
            }

            break;
        }

        case PumpCtlrState::PREFUSION:
        {
            m_pumpPwm = rateLimit(m_pumpPwm, EspressoConfig::prefusionDuty_pumpCtlr,  m_pumpRateChange, PUMP_MODULE_PERIOD_MS, EspressoConfig::offDuty_pumpCtlr, EspressoConfig::maxDuty_pumpCtlr);

            // Exit: If prefusion manually turned off
            if ( !m_machineCmdVals.prefusionEnable )
            {
                m_pumpRateChange = calculateRateChange(EspressoConfig::prefusionDuty_pumpCtlr, EspressoConfig::maxDuty_pumpCtlr, PUMP_PREFUSION_TO_BREW_TIME_MS);
                m_pumpState = PumpCtlrState::BREW;
            }

            // Exit: If weight target reached
            if (m_weight_g > PREFUSION_END_WEIGHT_G)
            {
                m_prefusionEndQualCount++;
                if (m_prefusionEndQualCount >= PREFUSION_END_QUAL_COUNT)
                {
                    m_prefusionEndQualCount = 0U;
                    m_pumpRateChange = calculateRateChange(EspressoConfig::prefusionDuty_pumpCtlr, EspressoConfig::maxDuty_pumpCtlr, PUMP_PREFUSION_TO_BREW_TIME_MS);
                    m_pumpState = PumpCtlrState::BREW;
                }
            }
            else
            {
                m_prefusionEndQualCount = 0U;
            }

            break;
        }

        case PumpCtlrState::BREW:
        {
            m_pumpPwm = rateLimit(m_pumpPwm, EspressoConfig::maxDuty_pumpCtlr, m_pumpRateChange, PUMP_MODULE_PERIOD_MS, EspressoConfig::offDuty_pumpCtlr, EspressoConfig::maxDuty_pumpCtlr);

            // Exit: Transition back to prefusion on rising edge
            if (!m_prevPrefusionCmd && m_machineCmdVals.prefusionEnable)
            {
                m_pumpRateChange = calculateRateChange(EspressoConfig::maxDuty_pumpCtlr, EspressoConfig::prefusionDuty_pumpCtlr, PUMP_BREW_TO_PREFUSION_TIME_MS);
                m_pumpState = PumpCtlrState::PREFUSION;
            }

            break;
        }

        default:
        {
            break;
        }
    }

    m_prevPrefusionCmd = m_machineCmdVals.prefusionEnable;
}

void PumpController::writeOutputs()
{
    ledcWrite(EspressoConfig::pwmChannel_pumpCtlr, EspressoConfig::maxDuty_pumpCtlr);  // Default pwm to high
}

void PumpController::processAlerts()
{
    const bool weightReached = (m_weight_g >= m_machineCmdVals.targetWeight_g);

    if ( !m_targetWeightAlertSet && m_brewTimer.isRunning() && weightReached )
    {
        if (m_targetWeightQualTimer.updateAndCheckTimer())  // Add some qualification to prevent hair trigger
        {
            m_targetWeightQualTimer.reset();
            StaticJsonDocument<50> alertJsonDoc;
            alertJsonDoc["tt_ms"] = m_brewTimer.getCount();
            size_t bufferSize_bytes = alertJsonDoc.memoryUsage();  // TODO: Look into why byte size set to 16
            char serializedData[bufferSize_bytes];
            serializeJson(alertJsonDoc, serializedData, bufferSize_bytes);


            AlertPayload myAlert {.alertName="targetWeightReached", .alertPayload=serializedData};
            printf("Adding alert payload: ");
            printf(serializedData);
            printf("\n\n");
            addAlert(myAlert);
            m_targetWeightAlertSet = true;
        }
    }
    else
    {
        m_targetWeightQualTimer.reset();
    }
}
