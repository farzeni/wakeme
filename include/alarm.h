#pragma once

#include <state.h>
#include <looper.h>

class WakemeAlarm
{
    public:
        WakemeAlarm(WakemeState* state);
        void setup();
        void checkAlarms();
        void play(Alarm* alarm);
        void snooze(Alarm* alarm);
        void stop(Alarm* alarm);
        void loop(unsigned long currentMillis);
    private:
        bool checkAlarm(Alarm* alarm, tm timeinfo);
        bool _isRinging;
        Looper* _looper;
        WakemeState* _state;
};
