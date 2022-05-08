#pragma once

#include <Arduino.h>
#include "ArduinoJson.h"
#include <Preferences.h>

using namespace std;

struct Alarm
{
    String id;
    String days;
    int hour;
    int minute;
    int sound;
    bool enabled;

    unsigned long int snoozeTime;
    unsigned long int stopTime;

    bool operator<(const Alarm &b) const
    {
        if (hour != b.hour)
        {
            return hour < b.hour;
        }

        if (minute != b.minute)
        {
            return minute < b.minute;
        }

        return false;
    }
};

class WakemeState
{
public:
    WakemeState();
    ~WakemeState();
    void setup();
    void loop(unsigned long currentMillis);
    tm getTime();
    bool isRinging();
    void setIsRinging(bool isRinging);
    bool isDisplayOn();
    void setIsDisplayOn(bool isDisplayOn);
    void setTime(int hours, int minutes, int seconds);
    int getHours();
    int getMinutes();
    int getSeconds();
    void storeSettings();
    void loadSettings();
    std::vector<Alarm *> getAlarms();
    Alarm* getAlarmByID(String id);
    bool addAlarm(Alarm *alarm);
    bool setAlarm(String id, Alarm *alarm);
    bool deleteAlarm(String id);
    bool deleteAllAlarms();
    void handleRoot();

private:
    unsigned int _hours;
    unsigned int _minutes;
    unsigned int _seconds;
    bool _isRinging;
    bool _isDisplayOn;
    Preferences _preferences;

    unsigned long _previousTick;
    std::vector<Alarm *> _alarms;
};

bool isValidAlarmData(JsonObject data);

Alarm *alarmFromJSON(JsonObject data);
std::vector<Alarm *> alarmsFromJSON(JsonObject data);
DynamicJsonDocument alarmsToJSON(std::vector<Alarm *> alarms);
String generateAlarmID();