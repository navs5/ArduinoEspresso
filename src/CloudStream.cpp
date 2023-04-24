#include "CloudStream.h"
#include "Credentials.h"

#define MS_TO_S(x)              ((x) / 1000.0F)
#define MS_TO_DS(x)             ((x) / 100.0F)
#define GRAMS_TO_MILLIGRAMS(x)  ((x) * 1000.0F)
#define GRAMS_TO_DECIGRAMS(x)   ((x) * 100.0F)
#define BAR_TO_DECIBAR(x)       ((x) * 100.0F)
#define C_TO_DECIBAR(x)         ((x) * 100.0F)

#define WITHIN(A, MIN, MAX)       ((A >= MIN) && (A <= MAX))
#define CHARS_TO_DIGITS_FAIL_VAL  (UINT32_MAX)

enum GeneralCommands_E {
    SCALE_TARE = 1U,
    START_STOP_TIMER = 2U,
    RESET_TIMER = 4U
};

enum MachineTarget_E
{
    WEIGHT = 0,
    PRESSURE = 1,
    TEMPERATURE
};


extern EspressoMachineNs::MachineCmdVals_S machineCmdVals;


bool isDigit(char a)
{
    return WITHIN(a, '0', '9');
}

float charsToDigits(const uint8_t* const payload, unsigned int length)
{
    if (length > 9U || length < 1)
    {
        // Number too large or non-existent
        return CHARS_TO_DIGITS_FAIL_VAL;
    }

    uint32_t payloadVal = 0U;
    uint32_t scale = 1U;
    uint32_t divisor = 1U;
    bool periodEncountered = false;
    for(int32_t i = (length-1U); i >= 0; --i)
    {
        const uint8_t charDigit = payload[i];
        const bool isPeriod = (charDigit == '.');
        if (isPeriod)
        {
            periodEncountered = true;
            continue;
        }

        if (!isDigit(charDigit))
        {
            return CHARS_TO_DIGITS_FAIL_VAL;
        }

        payloadVal += (scale * (charDigit - '0'));
        scale *= 10U;

        // Because looping backwards, keep scaling until period encountered
        if (!periodEncountered)
        {
            divisor *= 10U;
        }
    }

    return periodEncountered ? ((float)payloadVal / (float)divisor) : payloadVal;
}

void processCmdPayload(const uint8_t* const payload, unsigned int length)
{
    uint32_t payloadVal = charsToDigits(payload, length);

    if (payloadVal == CHARS_TO_DIGITS_FAIL_VAL)
    {
        Serial.print("Invalid general command payload\n");
        return;
    }

    switch (payloadVal)
    {
        case SCALE_TARE:
        {
            machineCmdVals.tareRequest = true;
            break;
        }
        case START_STOP_TIMER:
        {
            machineCmdVals.brewTimerPause = !machineCmdVals.brewTimerPause;
            break;
        }
        case RESET_TIMER:
        {
            machineCmdVals.brewTimerReset = true;
            break;
        }
        default:
        {
            break;
        }
    }
}

void updateTargets(MachineTarget_E targetType, const uint8_t* const payload, unsigned int length)
{
    float payloadVal = charsToDigits(payload, length);

    if (payloadVal == CHARS_TO_DIGITS_FAIL_VAL)
    {
        Serial.print("Invalid target command payload\n");
        return;
    }

    switch (targetType)
    {
        case MachineTarget_E::WEIGHT:
        {
            machineCmdVals.targetWeight_g = payloadVal;
            machineCmdVals.configUpdated = true;
            break;
        }
        case MachineTarget_E::PRESSURE:
        {
            machineCmdVals.targetPressure_bar = payloadVal;            
            machineCmdVals.configUpdated = true;
            break;
        }
        case MachineTarget_E::TEMPERATURE:
        {
            machineCmdVals.targetTemperature_C = payloadVal;
            machineCmdVals.configUpdated = true;
            break;
        }
        default:
        {
            break;
        }
    }
}

void commandsCallback(char* p_topic, uint8_t* p_message, unsigned int length)
{
    const char* p_specificTopic = strstr(p_topic, "espresso1/cmd/");

    if ( p_specificTopic == nullptr )
    {
        Serial.println("Command topic not correctly formatted");
        return;
    }

    p_specificTopic += 14U; // progress to the point where command topic differentiates

    if ( strstr(p_specificTopic, "general") != nullptr )
    {
        processCmdPayload(p_message, length);
    }
    else if( strstr(p_specificTopic, "setTargetWeight_g") != nullptr)
    {
        updateTargets(MachineTarget_E::WEIGHT, p_message, length);
    }
    else if( strstr(p_specificTopic, "setPress_bar") != nullptr)
    {
        updateTargets(MachineTarget_E::PRESSURE, p_message, length);
    }
    else if( strstr(p_specificTopic, "setTemp_C") != nullptr)
    {
        updateTargets(MachineTarget_E::TEMPERATURE, p_message, length);
    }
    else
    {
        Serial.println("Unrecognized command");
    }
}

void CloudStream::begin()
{
    m_client.setServer(NodeCredentials::mqttServer_ip, 1883);
    m_client.setCallback(static_cast<std::function<void(char*, uint8_t*, unsigned int)>>(commandsCallback));
    machineCmdVals.configUpdated = true;
}

void CloudStream::runCloudStream()
{
    m_msgsSentCurrentRun = 0U;

    if (!m_client.connected()) 
    {
        reconnect();
    }
    m_client.loop();

    // Immediately post config values if recently changed
    if (machineCmdVals.configUpdated)
    {
        machineCmdVals.configUpdated = false;
        m_infoTimer.expire();
    }

    postAlerts();
    postPeriodicMessages();
}

void CloudStream::reconnect() {
  // Loop until we're reconnected
  // TODO: Make this non-blocking and check for wifi connection
    while (!m_client.connected()) 
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (m_client.connect(m_nodeName.c_str(), NodeCredentials::mqttServer_username, NodeCredentials::mqttServer_pwd)) 
        {
            Serial.println("connected");
            // Subscribe
            m_client.subscribe("espresso1/cmd/general");  // TODO: Grab topic names from config
            m_client.subscribe("espresso1/cmd/setTargetWeight_g"); 
            m_client.subscribe("espresso1/cmd/setPress_bar"); 
            m_client.subscribe("espresso1/cmd/setTemp_C"); 
        } else 
        {
            Serial.print("failed, rc=");
            Serial.print(m_client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void CloudStream::postAlerts()
{
    std::vector<Controller*> alertControllers {&m_pumpController};

    for (Controller* currController : alertControllers)
    {
        while ((m_msgsSentCurrentRun < m_maxMsgsPerRun) && currController->alertPresent())
        {
            AlertPayload alertToSend {};
            currController->getAlert(alertToSend);
            
            // TODO: grab topic name from AlertPayload
            m_client.publish("espresso1/alert/targetWeightReached", alertToSend.alertPayload.c_str());       

            m_msgsSentCurrentRun++;
        }
    }    
}

void CloudStream::postPeriodicMessages()
{
    if (m_sensorValsTimer.updateAndCheckTimer() && (m_msgsSentCurrentRun < m_maxMsgsPerRun))
    {
        StaticJsonDocument<150> sensorJsonDoc;
        packageSensorData(sensorJsonDoc);

        size_t bufferSize_bytes = sensorJsonDoc.memoryUsage();
        char buffer[bufferSize_bytes];
        serializeJson(sensorJsonDoc, buffer, bufferSize_bytes);
        m_client.publish("espresso1/sensorVals", buffer);    
        m_msgsSentCurrentRun++;
        // Serial.println(buffer);    

        m_sensorValsTimer.reset();
    }

    if (m_infoTimer.updateAndCheckTimer() && (m_msgsSentCurrentRun < m_maxMsgsPerRun))
    {
        StaticJsonDocument<150> infoJsonDoc;
        packageInfoData(infoJsonDoc);

        size_t bufferSize_bytes = infoJsonDoc.memoryUsage();
        char buffer[bufferSize_bytes];
        serializeJson(infoJsonDoc, buffer, bufferSize_bytes);
        m_client.publish("espresso1/config", buffer);
        m_msgsSentCurrentRun++;    
        // Serial.println(buffer);    
        
        m_infoTimer.reset();
    }
}

void CloudStream::packageSensorData(JsonDocument& jsonDoc)
{
    jsonDoc["w"] = lroundf(GRAMS_TO_MILLIGRAMS(m_pumpController.getWeight()));
    jsonDoc["p"] = lroundf(456.5F);
    jsonDoc["tm"] = "323";
    jsonDoc["ti"] = lroundf(MS_TO_DS(m_pumpController.brewTimerGetCount()));
}

void CloudStream::packageInfoData(JsonDocument& jsonDoc)
{
    jsonDoc["tw"] = lroundf(GRAMS_TO_DECIGRAMS(machineCmdVals.targetWeight_g));
    jsonDoc["tp"] = lround((BAR_TO_DECIBAR(machineCmdVals.targetPressure_bar)));
    jsonDoc["tt"] = lround((C_TO_DECIBAR(machineCmdVals.targetTemperature_C)));
}
