#include <Arduino.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include <config.h>
#include <ui.h>
#include <ui/clock.h>

const long delayTime = 5;

static const uint16_t screenWidth = WM_SCREEN_WIDTH;
static const uint16_t screenHeight = WM_SCREEN_HEIGHT;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

bool isAwake = false;

TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); 

void my_log_cb(const char *buf)
{
  log_d("%s", buf);
}

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

struct UIState
{
  uint8_t hours = 0, minutes = 0, seconds = 0;
};

WakemeUI::WakemeUI(WakemeState *state)
{
  _previousTick = 0;
  _state = state;
}

void WakemeUI::setup()
{
  log_d("WakemeUI::setup");
  
  pinMode(WM_ALARM_DISPLAY_PIN, OUTPUT);

  initialize();
  draw();

#if LV_USE_LOG
  lv_log_register_print_cb(my_log_cb);
#endif /*LV_USE_LOG*/

  log_d("WakemeUI::setup TFT");
}

void WakemeUI::loop(unsigned long currentMillis)
{
  if (currentMillis - _previousTick > delayTime)
  {
    _previousTick = currentMillis;
    lv_timer_handler();
    lv_tick_inc(delayTime);

    if (isAwake != _state->isDisplayOn())
    {
      isAwake = _state->isDisplayOn();
      if (isAwake)
      {
        digitalWrite(WM_ALARM_DISPLAY_PIN, HIGH);
      }
      else
      {
        digitalWrite(WM_ALARM_DISPLAY_PIN, LOW);
      }
    }
  }
}


void WakemeUI::initialize()
{
  log_d("WakemeUI::init");
  lv_init();

  tft.begin();        /* TFT init */
  tft.setRotation(3); /* Landscape orientation, flipped */

  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /*Style main screen*/
  static lv_style_t style;
  lv_style_init(&style);
  lv_style_set_bg_color(&style, lv_color_black());
  lv_obj_add_style(lv_scr_act(), &style, 0);
}

void WakemeUI::draw()
{
  log_d("WakemeUI::Main Draw");
  clock_init(lv_scr_act(), _state);
}
