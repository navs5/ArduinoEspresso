#include "EspressoMachine.h"
#include <Arduino.h>

EspressoMachine machine1;

void setup() 
{
    Serial.begin(9600);
    machine1.begin();
}

void loop() 
{
    // call loop at 1khz
    static unsigned long before = micros();
    unsigned long now = micros();
    if ( now - before >= 10000U )
    {
        machine1.runMachine100Hz();
        before = now;
    }
}
