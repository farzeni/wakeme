#include <Arduino.h>
#include <state.h>
#include <ArduinoJson.h>
#include <Preferences.h>

const long delayTime = 1000;

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

WakemeState::WakemeState()
{
    _hours = 0;
    _minutes = 0;
    _seconds = 0;
    _isRinging = false;
    _previousTick = 0;
    _isDisplayOn = false;
}

WakemeState::~WakemeState()
{
    for (int i = 0; i < _alarms.size(); i++)
    {
        delete _alarms[i];
    }
}

void WakemeState::setup()
{
    randomSeed(analogRead(19));
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    _preferences.begin("wakeme", false);
    loadSettings();
}

void WakemeState::loop(unsigned long currentMillis)
{
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

tm WakemeState::getTime()
{
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    return timeinfo;
}

void WakemeState::setTime(int hours, int minutes, int seconds)
{
    _hours = hours;
    _minutes = minutes;
    _seconds = seconds;
}

bool WakemeState::isRinging()
{
    return _isRinging;
}
void WakemeState::setIsRinging(bool isRinging)
{
    _isRinging = isRinging;
}

bool WakemeState::isDisplayOn()
{
    return _isDisplayOn;
}
void WakemeState::setIsDisplayOn(bool isDisplayOn)
{
    _isDisplayOn = isDisplayOn;
}

int WakemeState::getHours()
{
    return _hours;
}
int WakemeState::getMinutes()
{
    return _minutes;
}
int WakemeState::getSeconds()
{
    return _seconds;
}

std::vector<Alarm *> WakemeState::getAlarms()
{
    return _alarms;
}

Alarm* WakemeState::getAlarmByID(String id) 
{
    for (int i = 0; i < _alarms.size(); i++)
    {
        if (_alarms[i]->id == id)
        {
            return _alarms[i];
        }
    }
    return NULL;
}

bool WakemeState::addAlarm(Alarm *alarm)
{
    _alarms.push_back(alarm);
    log_d("Alarm added %d", _alarms.size());
    std::sort(_alarms.begin(), _alarms.end());
    storeSettings();
    return false;
}

bool WakemeState::setAlarm(String id, Alarm *data)
{
    Alarm* alarm = getAlarmByID(id);
    if(alarm == NULL)
    {
        return false;
    }

    alarm->days = data->days;
    alarm->hour = data->hour;
    alarm->minute = data->minute;
    alarm->sound = data->sound;
    alarm->enabled = data->enabled;

    std::sort(_alarms.begin(), _alarms.end());
    storeSettings();

    return true;
}

bool WakemeState::deleteAlarm(String idx)
{
    int pos = -1;
    for (int i = 0; i < _alarms.size(); i++)
    {
        if (_alarms[i]->id == idx)
        {
            pos = i;
            break;
        }
    }

    if(pos == -1)
    {
        return false;
    }

    _alarms.erase(_alarms.begin() + pos);
    storeSettings();

    return true;
}

bool WakemeState::deleteAllAlarms()
{
    _alarms.clear();
    _preferences.clear();

    return true;
}

void WakemeState::storeSettings()
{
    log_d("storeSettings");
    DynamicJsonDocument serializedAlarms = alarmsToJSON(_alarms);
    char data[4096];
    serializeJson(serializedAlarms, data, 4096);
    log_d("%s", data);
    _preferences.putString("alarms", data);
}

void WakemeState::loadSettings()
{
    const int capacity = 4096; // TOFIX: hardcoded
    DynamicJsonDocument doc(capacity);
    String data = _preferences.getString("alarms");
    log_d("loadSettings %s", data.c_str());
    deserializeJson(doc, data);
    _alarms = alarmsFromJSON(doc.as<JsonObject>());
    log_d("Alarms loaded %d", _alarms.size());
}

bool isValidAlarmData(JsonObject data)
{
    bool hasKeys = data.containsKey("days") &&
                   data.containsKey("hour") &&
                   data.containsKey("minute") &&
                   data.containsKey("sound") &&
                   data.containsKey("enabled");

    if (!hasKeys)
    {
        log_e("Error parsing JSON: key error");
        return false;
    }

    if (!data["days"].is<String>() ||
        !data["hour"].is<int>() ||
        !data["minute"].is<int>() ||
        !data["enabled"].is<bool>())
    {
        log_e("Error parsing JSON: type error");
        return false;
    }

    if (sizeof(data["days"]) != 16)
    {

        log_w("Error parsing JSON: days size error: expected 16, got %d", sizeof(data["days"]));
        return false;
    }

    return true;
}

Alarm *alarmFromJSON(JsonObject data)
{
    Alarm *alarm = new Alarm();
    if (data.containsKey("id"))
    {
        alarm->id = data["id"].as<String>();
    }
    else
    {
        alarm->id = generateAlarmID();
    }
    alarm->days = data["days"].as<String>();
    log_d("createAlarmFromData:: days: %s", alarm->days.c_str());
    alarm->hour = data["hour"];
    alarm->minute = data["minute"];
    alarm->enabled = data["enabled"];
    alarm->sound = 0;
    return alarm;
}

std::vector<Alarm *> alarmsFromJSON(JsonObject data)
{
    std::vector<Alarm *> alarms;
    JsonArray alarmsArray = data["alarms"].as<JsonArray>();
    for (int i = 0; i < alarmsArray.size(); i++)
    {
        JsonObject alarmData = alarmsArray[i].as<JsonObject>();
        log_d("%d", alarmData.size());
        if (isValidAlarmData(alarmData))
        {
            alarms.push_back(alarmFromJSON(alarmData));
        }
    }

    return alarms;
}

DynamicJsonDocument alarmsToJSON(std::vector<Alarm *> alarms)
{
    const int capacity = 4096; // TOFIX: hardcoded
    DynamicJsonDocument doc(capacity);
    JsonArray objs = doc.createNestedArray("alarms");

    for (int i = 0; i < alarms.size(); i++)
    {
        log_d("Get alarms:: create object response %d", i);
        const Alarm alarm = *alarms[i];

        JsonObject a = objs.createNestedObject();
        log_d("Get alarms:: create object response %s", alarm.days);
        a["id"] = alarm.id;
        a["days"] = alarm.days;
        a["hour"] = alarm.hour;
        a["minute"] = alarm.minute;
        a["sound"] = alarm.sound;
        a["enabled"] = alarm.enabled;
    }

    return doc;
}

String generateAlarmID()
{
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    String id;

    for (int i = 0; i < 16; ++i)
    {
        id += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return id;
}