#include <Arduino.h>
#include <state.h>
#include <inputs.h>
#include <config.h>

unsigned int delayTime = 10;

unsigned int lastContactTime = 0;
unsigned int contactDelayTime = WM_AWAKE_DELAY;

WakemeInputs::WakemeInputs(WakemeState *state)
{
    _state = state;
    _previousTick = 0;
}

void WakemeInputs::setup()
{
}

void WakemeInputs::loop(unsigned long currentMillis)
{
    if (currentMillis - _previousTick > delayTime)
    {
        int stop_sensor_value = touchRead(TOUCH_SENSOR_STOP);
        if (stop_sensor_value < TOUCH_SENSOR_THRESHOLD)
        {
            if (!_state->isDisplayOn())
            {
                _state->setIsDisplayOn(true);
            }

            if (_state->isRinging())
            {
                _state->setIsRinging(false);
            }

            lastContactTime = currentMillis;
        }

        if (stop_sensor_value > TOUCH_SENSOR_THRESHOLD && currentMillis - lastContactTime > contactDelayTime)
        {
            _state->setIsDisplayOn(false);
        }
    }
}
