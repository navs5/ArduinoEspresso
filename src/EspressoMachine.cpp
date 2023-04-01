#include "EspressoMachine.h"
#include <ArduinoJson.h>

#define WITHIN(A, MIN, MAX)     ((A >= MIN) && (A <= MAX))

enum GeneralCommands {
    SCALE_TARE = 1U,
    START_STOP_TIMER = 2U,
    RESET_TIMER = 4U
};

MachineCmdVals EspressoMachine::m_machineCmdVals {};

bool isDigit(char a)
{
    return WITHIN(a, '0', '9');
}

uint32_t charsToDigits(const uint8_t* const payload, unsigned int length)
{
    if (length > 9U || length < 1)
    {
        // Number too large or non-existent
        return UINT32_MAX;
    }

    uint32_t payloadVal = 0U;
    uint32_t scale = 1U;
    for(int32_t i = (length-1U); i >= 0; --i)
    {
        uint8_t charDigit = payload[i];
        if (!isDigit(charDigit))
        {
            return UINT32_MAX;
        }

        payloadVal += (scale * (charDigit - '0'));
        scale *= 10U;        
    }

    return payloadVal;
}

void processCmdPayload(const uint8_t* const payload, unsigned int length)
{
    uint32_t payloadVal = charsToDigits(payload, length);

    if (payloadVal == UINT32_MAX)
    {
        Serial.print("Invalid command payload\n");
        return;
    }

    switch (payloadVal)
    {
        case SCALE_TARE:
        {
            EspressoMachine::m_machineCmdVals.tareRequest = true;
            break;
        }

        case START_STOP_TIMER:
        {
            EspressoMachine::m_machineCmdVals.brewTimerPause = !EspressoMachine::m_machineCmdVals.brewTimerPause;
            break;
        }

        case RESET_TIMER:
        {
            EspressoMachine::m_machineCmdVals.brewTimerReset = true;
            break;
        }
    
        default:
        {
            break;
        }
    }
}

void EspressoMachine::commandsCallback(char* p_topic, uint8_t* p_message, unsigned int length)
{
    const char* p_specificTopic = strstr(p_topic, "espresso1/cmd/");

    if ( p_specificTopic != nullptr )
    {
        p_specificTopic += 14U; // progress to the point where command topic differentiates

        if ( strstr(p_specificTopic, "general") != nullptr )
        {
            processCmdPayload(p_message, length);
        }
        else if( strstr(p_specificTopic, "setTargetWeight_g") != nullptr)
        {
            uint32_t payloadVal = charsToDigits(p_message, length);

            if (payloadVal != UINT32_MAX)
            {
                Serial.printf("set target weight_g: %d\n", payloadVal);
            }
        }
        else if( strstr(p_specificTopic, "setPress_bar") != nullptr)
        {
            Serial.println("setPress_bar command");
        }
        else if( strstr(p_specificTopic, "setTemp_C") != nullptr)
        {
            Serial.println("setTemp_C command");
        }
        else
        {
            Serial.println("Unrecognized command");
        }
    }
    else
    {
        Serial.println("Command topic not correctly formatted");
    }
}

void EspressoMachine::begin()
{
    m_brewTimer.pause();
    m_machineCmdVals.brewTimerPause = true;
    m_cloudStream1.begin();
    m_pumpController1.beginController();
}

void EspressoMachine::processCommandRequests()
{
    if ( m_machineCmdVals.tareRequest )
    {
        m_pumpController1.setTareRequest();
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
        m_brewTimer.reset();
        m_machineCmdVals.brewTimerPause = true;
        m_machineCmdVals.brewTimerReset = false;
    }
}

void EspressoMachine::runMachine100Hz()
{
    // Run pump controller for regulated pressure and weight
    m_pumpController1.runController();

    // Run the brew shot length timer
    m_brewTimer.updateAndCheckTimer();
    // Serial.printf("count: %d\n", m_brewTimer.getCount());

    // Run module to publish machine data to server
    m_cloudStream1.runCloudStream();

    // Act upon any received commands
    processCommandRequests();
}
