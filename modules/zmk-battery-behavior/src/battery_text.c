#define DT_DRV_COMPAT zmk_behavior_battery

#include <zephyr/kernel.h>
#include <zmk/behavior.h>
#include <zmk/behavior_queue.h>
#include <zmk/keymap.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/event_manager.h>
#include <drivers/behavior.h>

static int behavior_battery_pressed(struct zmk_behavior_binding *binding,
                                    struct zmk_behavior_binding_event event) {

    // HID Usage ID for keyboard
    // 'B'=0x05, 'A'=0x04, 'T'=0x17, 'E'=0x08, 'R'=0x15, 'Y'=0x1C
    // ':'=0x33, '1'=0x1E, '0'=0x27, '%'=0x22
    const uint8_t battery_codes[] = {
        0x05, 0x04, 0x17, 0x17, 0x08, 0x15, 0x1C, // BATTERY
        0x33,                                     // :
        0x1e, 0x27, 0x27,                         // 100
        0x22                                      // %
    };

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    for (int i = 0; i < sizeof(battery_codes) / sizeof(battery_codes[0]); i++) {
        struct zmk_keycode_state_changed_event press_event = {
            .data =
                {
                    .usage_page = HID_USAGE_KEY,
                    .keycode = battery_codes[i],
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

        struct zmk_keycode_state_changed_event release_event = {
            .data =
                {
                    .usage_page = HID_USAGE_KEY,
                    .keycode = battery_codes[i],
                    .implicit_modifiers = 0,
                    .explicit_modifiers = 0,
                    .state = false,
                    .timestamp = k_uptime_get(),
                },
            .header =
                {
                    .event = &zmk_event_zmk_keycode_state_changed,
                    .last_listener_index = 0,
                },
        };
        ZMK_EVENT_RAISE(release_event);

        k_msleep(10);
    }
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
