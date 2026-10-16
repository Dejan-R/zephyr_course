
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <lvgl.h>
#include "logo.h"

#define MESSAGE_TIMEOUT_MS 5000

/* Message label and its hide timer. */
static lv_obj_t *msg_label;
static lv_timer_t *msg_timer;

/* Hide the message after the timeout. */
static void hide_msg_cb(lv_timer_t *timer)
{
    lv_obj_add_flag(msg_label, LV_OBJ_FLAG_HIDDEN);
    lv_timer_pause(timer);
}

/* Show the message and restart the timeout on each click. */
static void btn_event_cb(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
        return;
    }

    lv_label_set_text(msg_label, "BUTTON CLICKED!");
    lv_obj_remove_flag(msg_label, LV_OBJ_FLAG_HIDDEN);

    lv_timer_reset(msg_timer);
    lv_timer_resume(msg_timer);
}

int main(void)
{
    const struct device *display =
        DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

    /* Zephyr initializes LVGL automatically. */
    if (!device_is_ready(display)) {
        printk("Display NOT ready\n");
        return 0;
    }

    lv_display_t *lv_display = lv_display_get_default();

    if (lv_display == NULL) {
        printk("LVGL display NOT available\n");
        return 0;
    }

    lv_obj_t *screen = lv_screen_active();

    /* White background. */
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xFFFFFF), 0);

    /* School logo. */
    lv_obj_t *img = lv_image_create(screen);
    lv_image_set_src(img, &logo);
    lv_obj_align(img, LV_ALIGN_TOP_RIGHT, -5, 5);

    /* Scrolling title. */
    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "Zephyr & ESP32 + LVGL + ILI9341");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_18, 0);
    lv_obj_set_style_anim_duration(title, 4000, 0);
    lv_label_set_long_mode(title, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);

    int32_t screen_width =
        lv_display_get_horizontal_resolution(lv_display);

    lv_obj_set_width(title,
                     screen_width > 140 ? screen_width - 140 : screen_width);

    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 5, 15);

    /* Center button. */
    lv_obj_t *btn = lv_button_create(screen);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_text = lv_label_create(btn);
    lv_label_set_text(btn_text, "CLICK");
    lv_obj_center(btn_text);

    /* Initially hidden click message. */
    msg_label = lv_label_create(screen);
    lv_label_set_text(msg_label, "BUTTON CLICKED!");
    lv_obj_align_to(msg_label, btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_add_flag(msg_label, LV_OBJ_FLAG_HIDDEN);

    /* Create one reusable timer. */
    msg_timer = lv_timer_create(hide_msg_cb, MESSAGE_TIMEOUT_MS, NULL);

    if (msg_timer == NULL) {
        printk("Failed to create LVGL timer\n");
        return 0;
    }

    lv_timer_pause(msg_timer);

    /* Render the initial screen and turn the display on. */
    lv_timer_handler();
    display_blanking_off(display);

    while (1) {
        lv_timer_handler();
        k_sleep(K_MSEC(10));
    }
}
