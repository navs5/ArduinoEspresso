#include "CloudStream.h"
#include "Credentials.h"


void CloudStream::reconnect() {
  // Loop until we're reconnected
    while (!m_client.connected()) 
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (m_client.connect(m_nodeName.c_str(), NodeCredentials::mqttServer_username, NodeCredentials::mqttServer_pwd)) 
        {
            Serial.println("connected");
            // Subscribe
            m_client.subscribe("espresso1/cmd/general");
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

void CloudStream::begin()
{
    // We start by connecting to a WiFi network
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(NodeCredentials::wifi_ssid);

    WiFi.begin(NodeCredentials::wifi_ssid, NodeCredentials::wifi_pwd);

    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    m_client.setServer(NodeCredentials::mqttServer_ip, 1883);
    m_client.setCallback(m_callback);
}

void CloudStream::runCloudStream(JsonDocument& sensorJsonDoc, JsonDocument& infoJsonDoc)
{
    if (!m_client.connected()) 
    {
        reconnect();
    }
    m_client.loop();
    
    if (m_sensorValsTimer.updateTimer())
    {
        size_t bufferSize_bytes = sensorJsonDoc.memoryUsage();
        char buffer[bufferSize_bytes];
        size_t n = serializeJson(sensorJsonDoc, buffer, bufferSize_bytes);
        m_client.publish("espresso1/sensorVals", buffer, n);    
        Serial.println(buffer);    

        m_sensorValsTimer.reset();
    }

    if (m_infoTimer.updateTimer())
    {
        size_t bufferSize_bytes = infoJsonDoc.memoryUsage();
        char buffer[bufferSize_bytes];
        size_t n = serializeJson(infoJsonDoc, buffer, bufferSize_bytes);
        m_client.publish("espresso1/info", buffer, n);    
        Serial.println(buffer);    
        
        m_sensorValsTimer.reset();
    }
}
