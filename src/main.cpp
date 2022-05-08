#include <Arduino.h>
#include <alarm.h>
#include <network.h>
#include <config.h>
#include <state.h>
#include <ui.h>
#include <api.h>
#include <inputs.h>

unsigned long currentMillis = 0; 

WakemeNetwork network;
WakemeState state;
WakemeAPI api(&state);
WakemeAlarm alarmClock(&state);
WakemeUI ui(&state);
WakemeInputs inputs(&state);

void setup()    
{
    log_d("Wakeme::Main Setup");
    Serial.begin(9600);

    network.setup();
    state.setup();
    inputs.setup();
    ui.setup();
    alarmClock.setup();
    api.begin();
}

void loop(void)
{
    currentMillis = millis();
    state.loop(currentMillis);
    inputs.loop(currentMillis);
    ui.loop(currentMillis);
    alarmClock.loop(currentMillis);
}
