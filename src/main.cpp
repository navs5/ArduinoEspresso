#include "EspressoMachine.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include "Credentials.h"
#include "Config.h"
#include "webServerPages.hpp"

EspressoMachine machine1;
WebServer server(80);


void setup()
{
    // Set all pins to default values
    ledcSetup(EspressoConfig::pwmChannel_pumpCtlr, EspressoConfig::pwmFreq_pumpCtlr, EspressoConfig::pwmResolution_pumpCtlr);
    ledcAttachPin(EspressoConfig::pin_pumpCtlr, EspressoConfig::pwmChannel_pumpCtlr);
    ledcWrite(EspressoConfig::pwmChannel_pumpCtlr, EspressoConfig::maxDuty_pumpCtlr);  // Default pwm to high

    delay(10);
    Serial.begin(9600);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(NodeCredentials::wifi_ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(NodeCredentials::wifi_ssid, NodeCredentials::wifi_pwd);

    size_t counter = 0U;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");

        if ( counter > 20U )
        {
            Serial.println("Could not connect...Restarting ESP");
            ESP.restart();
        }
        counter++;
    }

    /*use mdns for host name resolution*/
    if (!MDNS.begin(EspressoConfig::mdnsHostName))  //http://esp32resso.local
    {
        Serial.println("Error setting up MDNS responder!");
        // TODO: Add retry mechanism here
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    /*return index page which is stored in serverIndex */
    server.on("/", HTTP_GET, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", WebServerHtml::serverIndex);
    });
    /*handling uploading firmware file */
    server.on("/update",
        HTTP_POST,
        []() {
            server.sendHeader("Connection", "close");
            server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
            ESP.restart();
        },
        []() {
            HTTPUpload& upload = server.upload();
            if (upload.status == UPLOAD_FILE_START) {
            Serial.printf("Update: %s\n", upload.filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
                Update.printError(Serial);
            }
            } else if (upload.status == UPLOAD_FILE_WRITE) {
            /* flashing firmware to ESP*/
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                Update.printError(Serial);
            }
            } else if (upload.status == UPLOAD_FILE_END) {
            if (Update.end(true)) { //true to set the size to the current progress
                Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
            } else {
                Update.printError(Serial);
            }
            }
        });

    server.begin();
    machine1.begin();
}

void loop()
{
    server.handleClient();

    // call loop at 100hz
    static unsigned long before = micros();
    unsigned long now = micros();
    if ( now - before >= 10000U )
    {
        machine1.runMachine100Hz();
        before = now;
    }
}
