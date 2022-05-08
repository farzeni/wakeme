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
    _server->on("/ready", HTTP_GET, [this](AsyncWebServerRequest *request)
                { handleReady(request); });

    _server->on("/alarms", HTTP_GET, [this](AsyncWebServerRequest *request)
                { handleGetAlarms(request); });

    _server->on("^\\/alarms\\/(.*)$", HTTP_DELETE, [this](AsyncWebServerRequest *request)
                { handleDeleteAlarm(request); });

    _server->on("/alarms_all", HTTP_DELETE, [this](AsyncWebServerRequest *request)
                { handleDeleteAllAlarms(request); });

    _server->addHandler(new AsyncCallbackJsonWebHandler(
        "/alarms",
        [this](AsyncWebServerRequest *request, JsonVariant &json)
        {
            if (request->method() == HTTP_POST)
            {
                handleCreateAlarm(request, json);
            }
        }));

    _server->addHandler(new AsyncCallbackJsonWebHandler(
        "^\\/alarms\\/(.*)$",
        [this](AsyncWebServerRequest *request, JsonVariant &json)
        {
            if (request->method() == HTTP_PUT)
            {
                handleUpdateAlarm(request, json);
            }
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

    std::vector<Alarm *> alarms = _state->getAlarms();
    ;
    //_storage->getAlarms();
    log_d("Get alarms %d", *_state);
    DynamicJsonDocument doc = alarmsToJSON(alarms);
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
        Alarm *alarm = alarmFromJSON(data);
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
        Alarm *alarm = alarmFromJSON(data);
        _state->setAlarm(alarmId, alarm);
        request->send_P(200, "text/plain", "{\"status\":\"OK\"}");
    }
    else
    {
        request->send_P(400, F("text/html"), "{\"status\":\"Error in data\"}");
    }
}

void WakemeAPI::handleDeleteAlarm(AsyncWebServerRequest *request)
{
    String alarmId = request->pathArg(0);
    log_d("Delete alarm: %s", alarmId);

    bool success = _state->deleteAlarm(alarmId);
    if (success)
    {
        request->send_P(200, "text/plain", "{\"status\":\"OK\"}");
    }
    else
    {
        request->send_P(400, "text/plain", "{\"status\":\"KO\"}");
    }
}

void WakemeAPI::handleDeleteAllAlarms(AsyncWebServerRequest *request)
{
    _state->deleteAllAlarms();
    request->send_P(200, "text/plain", "{\"status\":\"OK\"}");
}
