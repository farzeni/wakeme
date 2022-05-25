#pragma once

#include <functional>

typedef std::function<void()> LooperCallback;

class Looper
{
    public:
        Looper(unsigned long delay);
        void loop(unsigned long currentMillis, LooperCallback callback);
    private:
        unsigned long _previousTick;
        unsigned long _delay;
};