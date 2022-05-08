#pragma once

#include <secrets.h>

// Network

#define WM_WIFI_SSID WM_SECRET_WIFI_SSID
#define WM_WIFI_PASSWORD WM_SECRET_WIFI_PASSWORD
#define WM_HOSTNAME "wakeme"

// Alarm options

#define WM_ALARM_LENGTH 70
#define WM_SNOOZE_TIME 300
#define WM_STOP_TIME 300
#define WM_AWAKE_DELAY 5000

// Screen

#define WM_SCREEN_WIDTH 320
#define WM_SCREEN_HEIGHT 240

// PINs

#define DFPLAYER_TX_PIN 17
#define DFPLAYER_RX_PIN 16
#define WM_ALARM_DISPLAY_PIN 32

// Inputs

#define TOUCH_SENSOR_STOP T5
#define TOUCH_SENSOR_SNOOZE T4
#define TOUCH_SENSOR_THRESHOLD 50


