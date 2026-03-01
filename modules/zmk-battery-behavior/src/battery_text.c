#define DT_DRV_COMPAT zmk_behavior_battery

#include <zephyr/kernel.h>
#include <zmk/behavior.h>
#include <zmk/behavior_queue.h>
#include <zmk/keymap.h>
#include <zmk/battery.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/event_manager.h>
#include <drivers/behavior.h>

static uint8_t soc_central;
static uint8_t soc_peripheral;
static bool soc_central_valid;
static bool soc_peripheral_valid;

static int battery_text_listener(const zmk_event_t *eh) {
    struct zmk_battery_state_changed *central = as_zmk_battery_state_changed(eh);
    if (central) {
        soc_central = central->state_of_charge;
        soc_central_valid = true;
        return 0;
    }

    struct zmk_peripheral_battery_state_changed *periph =
        as_zmk_peripheral_battery_state_changed(eh);
    if (periph) {
        /* 単一ペリフェラル構成を想定し、これを「L側」とみなす */
        soc_peripheral = periph->state_of_charge;
        soc_peripheral_valid = true;
        return 0;
    }

    return 0;
}

ZMK_LISTENER(battery_text, battery_text_listener);
ZMK_SUBSCRIPTION(battery_text, zmk_battery_state_changed);
ZMK_SUBSCRIPTION(battery_text, zmk_peripheral_battery_state_changed);

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
static void battery_text_send_usage(uint8_t usage) {
    struct zmk_keycode_state_changed_event press_event = {
        .data =
            {
                .usage_page = HID_USAGE_KEY,
                .keycode = usage,
                .implicit_modifiers = 0,
                .explicit_modifiers = 0,
                .state = true,
                .timestamp = k_uptime_get(),
            },
        .header =
            {
                .event = &zmk_event_zmk_keycode_state_changed,
                .last_listener_index = 0,
            },
    };
    ZMK_EVENT_RAISE(press_event);
    k_msleep(10);

    struct zmk_keycode_state_changed_event release_event = press_event;
    release_event.data.state = false;
    release_event.data.timestamp = k_uptime_get();
    ZMK_EVENT_RAISE(release_event);
    k_msleep(10);
}

static void battery_text_send_digit(uint8_t d) {
    uint8_t usage = (d == 0) ? 0x27 : (uint8_t)(0x1E + (d - 1)); /* '0' or '1'〜'9' */
    battery_text_send_usage(usage);
}

static void battery_text_send_decimal(uint8_t value) {
    if (value > 100) {
        value = 100;
    }
    if (value == 100) {
        battery_text_send_digit(1);
        battery_text_send_digit(0);
        battery_text_send_digit(0);
        return;
    }

    uint8_t tens = value / 10;
    uint8_t ones = value % 10;

    if (tens) {
        battery_text_send_digit(tens);
    }
    battery_text_send_digit(ones);
}

static void battery_text_send_lr(uint8_t left_soc, uint8_t right_soc) {
    const uint8_t USAGE_L = 0x0F;
    const uint8_t USAGE_R = 0x15;
    const uint8_t USAGE_SPACE = 0x2C;

    /* 左（ペリフェラル想定） */
    battery_text_send_usage(USAGE_L);
    battery_text_send_decimal(left_soc);

    /* 区切りスペース */
    battery_text_send_usage(USAGE_SPACE);

    /* 右（セントラル） */
    battery_text_send_usage(USAGE_R);
    battery_text_send_decimal(right_soc);
}
#endif

static int behavior_battery_pressed(struct zmk_behavior_binding *binding,
                                    struct zmk_behavior_binding_event event) {
    uint8_t right_soc = soc_central_valid ? soc_central : zmk_battery_state_of_charge();
    uint8_t left_soc = soc_peripheral_valid ? soc_peripheral : right_soc;

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    battery_text_send_lr(left_soc, right_soc);
#endif

    return ZMK_BEHAVIOR_OPAQUE;
}

static int behavior_battery_released(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_battery_driver_api = {
    .binding_pressed = behavior_battery_pressed,
    .binding_released = behavior_battery_released,
};

#define BATTERY_INST(n) \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL, \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_battery_driver_api);

DT_INST_FOREACH_STATUS_OKAY(BATTERY_INST)
