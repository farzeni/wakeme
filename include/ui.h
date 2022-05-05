#pragma once

#include <state.h>

class WakemeUI
{
    public:
        WakemeUI(WakemeState *state);
        void setup();
        void loop(unsigned long currentMillis);
    private:
        void initialize();
        void draw();
        unsigned long _previousTick;
        WakemeState *_state;
};
