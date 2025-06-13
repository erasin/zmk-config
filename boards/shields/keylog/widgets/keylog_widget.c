#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>
#include <string.h>
#include <stdio.h>
#include <zmk/display/widgets/widget.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/hid.h>
#include <zmk/hid_usage.h>
#include "keylog_widget.h"

#define MAX_LOG_LEN 10

static char keylog[MAX_LOG_LEN][8];
static int log_index = 0;

static const char *mod_names[8] = {
    "LCTL", "LSFT", "LALT", "LGUI", "RCTL", "RSFT", "RALT", "RGUI"
};

static void draw_keylog(const struct device *dev, struct widget *widget, void *data) {
    char line[64];
    struct display_buffer_descriptor desc = {
        .width = 128,
        .height = 32,
        .pitch = 128,
        .buf_size = 128*4
    };
    memset(line, 0, sizeof(line));
    snprintf(line, sizeof(line), "Key: %s", keylog[(log_index - 1 + MAX_LOG_LEN) % MAX_LOG_LEN]);
    display_write(dev, 0, 0, &desc, line);

    // 显示修饰键
    struct zmk_hid_keyboard_report *report = zmk_hid_get_keyboard_report();
    char mod_str[32] = {0};
    for (int i = 0; i < 8; i++) {
        if (report && (report->modifiers & (1 << i))) {
            strcat(mod_str, mod_names[i]);
            strcat(mod_str, " ");
        }
    }
    if (strlen(mod_str) == 0) strcpy(mod_str, "Mods: None");
    else {
        char tmp[40];
        snprintf(tmp, sizeof(tmp), "Mods: %s", mod_str);
        strcpy(mod_str, tmp);
    }
    display_write(dev, 0, 16, &desc, mod_str);
}

static int on_keycode_state_changed(const struct zmk_keycode_state_changed *ev) {
    if (ev->state) {
        snprintf(keylog[log_index], sizeof(keylog[log_index]), "0x%X", ev->keycode);
        log_index = (log_index + 1) % MAX_LOG_LEN;
        widget_refresh();
    }
    return 0;
}

struct widget keylog_widget = WIDGET_INITIALIZER(draw_keylog, NULL);

void keylog_widget_init(void) {
    widget_init(&keylog_widget);
}

ZMK_LISTENER(keylog_widget, on_keycode_state_changed);
ZMK_SUBSCRIPTION(keylog_widget, zmk_keycode_state_changed);
