#include <Arduino.h>
#include <alarm.h>
#include <state.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <config.h>

const long delayTime = 1000;

unsigned long alarmStartTime = 0;

SoftwareSerial playerSerial(DFPLAYER_RX_PIN, DFPLAYER_TX_PIN);
DFRobotDFPlayerMini player;

WakemeAlarm::WakemeAlarm(WakemeState* state)
{
    _isRinging = false;
    _previousTick = 0;
    _state = state;
    _looper = new Looper(delayTime);
}

void WakemeAlarm::setup()
{
    log_d("WakemeAlarm::setup");

    playerSerial.begin(9600);

    while (!player.begin(playerSerial))
    {
        log_i("Player not ready, waiting....");
        delay(500);
    }

    log_d("DFPlayer Mini online.");
}

void WakemeAlarm::loop(unsigned long currentMillis)
{
    _looper->loop(currentMillis, [this]() {
        log_d("is right now ringing: %d (state: %d)", _isRinging, _state->isRinging());

        if (_state->isRinging())
        {
            if (millis() - alarmStartTime > WM_ALARM_LENGTH * 1000)
            {
                _state->setIsRinging(false);
                player.stop();
                log_d("Alarm expired");
            }
        }
        else
        {
            if(_isRinging)
            {
                player.stop();
            }
            checkAlarms();
        }
    });
}

void WakemeAlarm::checkAlarms()
{
    struct tm timeinfo = _state->getTime();

    if (_state->isRinging())
    {
        return;
    }

    log_d("WakemeAlarm::checkAlarms. %d:%d:%d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    std::vector<Alarm *> alarms = _state->getAlarms();
    for (int i = 0; i < alarms.size(); i++)
    {
        if(checkAlarm(alarms[i], timeinfo))
        {
            play(alarms[i]);
            break;
        }
    }
}

void WakemeAlarm::play(Alarm* alarm)
{
    log_d("WakemeAlarm::play");
    _isRinging = true;
    _state->setIsRinging(true);
    player.loop(1);
    alarmStartTime = millis();
}

void WakemeAlarm::snooze(Alarm* alarm)
{
    log_d("WakemeAlarm::stop");
    _isRinging = false;
    _state->setIsRinging(false);
    alarm->snoozeTime = millis();
    player.pause();
}

void WakemeAlarm::stop(Alarm* alarm)
{
    log_d("WakemeAlarm::stop");
    _isRinging = false;
    _state->setIsRinging(false);
    alarm->stopTime = millis();
    player.pause();
}


bool WakemeAlarm::checkAlarm(Alarm* alarm, tm timeinfo)
{
    String wDay = String(timeinfo.tm_wday);

    if(alarm->snoozeTime > 0 && millis() - alarm->snoozeTime < WM_SNOOZE_TIME * 1000)
    {
        return false;
    }

    if(alarm->stopTime > 0 && millis() - alarm->stopTime)
    {
        return false;
    }

    if (alarm->hour == timeinfo.tm_hour && alarm->minute == timeinfo.tm_min && alarm->days.indexOf(wDay) != -1)
    {
        return true;
    }

    return false;
}