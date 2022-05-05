#include <functional>
#include <Arduino.h>
#include <ui/clock.h>
#include <lvgl.h>

unsigned int _lastMinute = 99;
lv_obj_t *_label;
WakemeState *_state;

void _clock_tick(lv_timer_t *timer)
{
    clock_draw();
}

void clock_init(lv_obj_t *parent, WakemeState *state)
{
    _state = state;

    _label = lv_label_create(parent);
    log_d("clock_init start");

    lv_obj_align(_label, LV_ALIGN_CENTER, 0, 0);

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_color(&style, lv_palette_main(LV_PALETTE_GREY));

    lv_style_set_text_font(&style, &lv_font_montserrat_48);
    lv_obj_add_style(_label, &style, 0);

    lv_timer_create(_clock_tick, 1000, NULL);
    clock_draw();
    log_d("clock_init end");
}

void clock_draw()
{
    if(_lastMinute != _state->getMinutes())
    {
        char labelString[30];
        if(_state->isRinging()) {
            sprintf(labelString, "SVEGLIA!");
        } else {

            sprintf(labelString, "%02d:%02d", _state->getHours(), _state->getMinutes());
        }
        log_d("UIClock::draw %s", labelString);
        lv_label_set_text(_label, labelString);

        _lastMinute = _state->getMinutes();
    }
}
