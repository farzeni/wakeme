#include <Arduino.h>
#include <state.h>
#include <ArduinoJson.h>

const long delayTime = 1000;

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

WakemeState::WakemeState() {
    _hours = 0;
    _minutes = 0;
    _seconds = 0;
    _isRinging = false;
    _previousTick = 0;
    _isDisplayOn = false;
}

WakemeState::~WakemeState() {
    for (int i = 0; i < _alarms.size(); i++) {
        delete _alarms[i];
    }
}

void WakemeState::setup() {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void WakemeState::loop(unsigned long currentMillis) {
    if (currentMillis - _previousTick > delayTime)
    {
        struct tm timeinfo;

        _previousTick = currentMillis;

        if (!getLocalTime(&timeinfo))
        {
            log_e("Failed to obtain time");
            return;
        }

        _hours = timeinfo.tm_hour;
        _minutes = timeinfo.tm_min;
        _seconds = timeinfo.tm_sec;
    }
}

tm WakemeState::getTime() {
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    return timeinfo;
}

void WakemeState::setTime(int hours, int minutes, int seconds) {
    _hours = hours;
    _minutes = minutes;
    _seconds = seconds;
}

bool WakemeState::isRinging() {
    return _isRinging;
}
void WakemeState::setIsRinging(bool isRinging) {
    _isRinging = isRinging;
}

bool WakemeState::isDisplayOn() {
    return _isDisplayOn;
}
void WakemeState::setIsDisplayOn(bool isDisplayOn) {
    _isDisplayOn = isDisplayOn;
}

int WakemeState::getHours() {
    return _hours;
}
int WakemeState::getMinutes() {
    return _minutes;
}
int WakemeState::getSeconds() {
    return _seconds;
}

std::vector<Alarm *> WakemeState::getAlarms()
{
    return _alarms;
}

bool WakemeState::setAlarm(int idx, Alarm* alarm)
{
    _alarms[idx] = alarm;
    std::sort(_alarms.begin(), _alarms.end());
    return true;
}

bool WakemeState::deleteAlarm(int idx)
{
    _alarms.erase(_alarms.begin() + idx);
    return true;
}

bool WakemeState::addAlarm(Alarm* alarm)
{
    _alarms.push_back(alarm);
    log_d("Alarm added %d", _alarms.size());
    std::sort(_alarms.begin(), _alarms.end());
    return false;
}


bool isValidAlarmData(JsonObject data) {
    bool hasKeys = data.containsKey("days") &&
           data.containsKey("hour") &&
           data.containsKey("minute") &&
           data.containsKey("enabled");

    if(!hasKeys) {
        log_e("Error parsing JSON: key error");
        return false;
    }

    if(!data["days"].is<String>() ||
       !data["hour"].is<int>() ||
       !data["minute"].is<int>() ||
       !data["enabled"].is<bool>()) {
        log_e("Error parsing JSON: type error");
        return false;
    }

    if(sizeof(data["days"]) != 16) {

        log_w("Error parsing JSON: days size error: expected 16, got %d", sizeof(data["days"]));
        return false;
    }

    return true;
}

Alarm* alarmFromJSON(JsonObject data) {
    Alarm *alarm = new Alarm();
    alarm->days = data["days"].as<String>();
    log_d("createAlarmFromData:: days: %s", alarm->days.c_str());
    alarm->hour = data["hour"];
    alarm->minute = data["minute"];
    alarm->enabled = data["enabled"];
    alarm->sound = 0;
    return alarm;
}