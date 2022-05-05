#pragma once

#include "state.h"
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"

#define API_PORT 80

class WakemeAPI
{
    public:
        WakemeAPI(WakemeState *state);
        ~WakemeAPI();
        void begin();
    private:
        void handleReady(AsyncWebServerRequest *request);
        void handleGetAlarms(AsyncWebServerRequest *request);
        void handleUpdateAlarm(AsyncWebServerRequest *request, JsonVariant& json);
        void handleCreateAlarm(AsyncWebServerRequest *request, JsonVariant& json);
        void handleDeleteAlarm(AsyncWebServerRequest *request);
        void handleIndex(AsyncWebServerRequest *request);
        AsyncWebServer *_server;
        WakemeState *_state;
};

