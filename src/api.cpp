#include <Arduino.h>
#include <api.h>
#include <state.h>
#include "ArduinoJson.h"
#include <AsyncJson.h>
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"

WakemeAPI::WakemeAPI(WakemeState *state)
{
    _state = state;
    _server = new AsyncWebServer(API_PORT);

    // Route for root / web page
    _server->on("/ready", HTTP_GET, [&](AsyncWebServerRequest *request)
                { handleReady(request); });

    _server->on("/alarms", HTTP_GET, [&](AsyncWebServerRequest *request)
                { handleGetAlarms(request); });

    _server->addHandler(new AsyncCallbackJsonWebHandler(
        "/alarms",
        [this](AsyncWebServerRequest *request, JsonVariant &json)
        {
            handleCreateAlarm(request, json);
        }));

    log_d("HTTP server started");
}

WakemeAPI::~WakemeAPI()
{
    delete _server;
}

void WakemeAPI::begin()
{
    _server->begin();
}

void WakemeAPI::handleReady(AsyncWebServerRequest *request)
{
    request->send_P(200, "application/json", "{\"status\":\"OK\"}");
}

void WakemeAPI::handleGetAlarms(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");

    std::vector<Alarm *> alarms = _state->getAlarms();;
    //_storage->getAlarms();
    log_d("Get alarms %d", *_state);

    const int capacity = JSON_OBJECT_SIZE(6) + JSON_ARRAY_SIZE(20);
    StaticJsonDocument<capacity> doc;
    JsonArray objs = doc.createNestedArray("alarms");
    
    log_d("Get alarms:: create array response %d", alarms.size());

    for (int i = 0; i < alarms.size(); i++)
    {
        log_d("Get alarms:: create object response %d", i);
        const Alarm alarm = *alarms[i];
        JsonObject a = objs.createNestedObject();
        log_d("Get alarms:: create object response %s", alarm.days);
        a["days"] = alarm.days;
        a["hour"] = alarm.hour;
        a["minute"] = alarm.minute;
        a["sound"] = alarm.sound;
        a["enabled"] = alarm.enabled;
    }

    String data;
    log_d("serializeJson()");
    serializeJson(doc, *response);

    request->send(response);
}

void WakemeAPI::handleCreateAlarm(AsyncWebServerRequest *request, JsonVariant &json)
{
    log_d("Create alarm %d", *_state);
    if (not json.is<JsonObject>())
    {
        request->send(400, "text/plain", "Not an object");
        return;
    }

    auto &&data = json.as<JsonObject>();

    if (isValidAlarmData(data))
    {
        log_d("is valid data");
        Alarm* alarm = alarmFromJSON(data);
        log_d("alrm created");
        bool error = _state->addAlarm(alarm);
        log_d("alrm added %d", error);
        if (error)
        {
            request->send(400, "text/plain", "{\"status\":\"KO\"}");
        }
        else
        {
            request->send(200, "text/plain", "{\"status\":\"OK\"}");
        }
    }
    else
    {
        request->send(400, F("text/html"), "{\"status\":\"Error in data\"}");
    }
}

void WakemeAPI::handleUpdateAlarm(AsyncWebServerRequest *request, JsonVariant &json)
{
    String alarmId = request->pathArg(0);

    log_d("Set alarm: %s", alarmId);
    if (not json.is<JsonObject>())
    {
        request->send(400, "text/plain", "Not an object");
        return;
    }

    auto &&data = json.as<JsonObject>();

    if (isValidAlarmData(data))
    {
        Alarm* alarm = alarmFromJSON(data);
        _state->setAlarm(alarmId.toInt(), alarm);
        request->send_P(200, "text/plain", "{\"status\":\"OK\"}");
    }
    else
    {
        request->send_P(400, F("text/html"), "{\"status\":\"Error in data\"}");
    }
}

void WakemeAPI::handleDeleteAlarm(AsyncWebServerRequest *request)
{
    // String alarmId = _server.pathArg(0);
    // log_d("Delete alarm: %s", alarmId);
    // _state->deleteAlarm(alarmId.toInt());
}
