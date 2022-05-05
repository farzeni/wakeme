#include <WiFi.h>
#include <network.h>
#include <ESPmDNS.h>
#include <config.h>

WakemeNetwork::WakemeNetwork() {}

void WakemeNetwork::setup()
{
    log_d("WakemeNetwork::Start connecting Wifi on %s", WM_WIFI_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WM_WIFI_SSID, WM_WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        log_d("Connecting to WiFi..");
        delay(500);
    }

    log_i("WakemeNetwork::WiFi connected. IP address: %s", WiFi.localIP().toString().c_str());

    if (MDNS.begin(WM_HOSTNAME))
    {
        log_i("MDNS responder started link: http://%s.local", WM_HOSTNAME);
        MDNS.addService("http", "tcp", 80);
    }

}

