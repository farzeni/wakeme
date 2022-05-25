#pragma once

#include <state.h>
#include <config.h>
#include <looper.h>


class WakemeInputs
{
    public:
        WakemeInputs(WakemeState *state);
        void setup();
        void loop(unsigned long currentMillis);
    private:
        WakemeState *_state;
        Looper* _looper;
};