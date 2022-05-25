#include <looper.h>

Looper::Looper(unsigned long delay)
{
    _previousTick = 0;
    _delay = delay;
}

void Looper::loop(unsigned long currentMillis, LooperCallback callback)
{
    if (currentMillis - _previousTick > _delay)
    {
        _previousTick = currentMillis;
        callback();
    }
}