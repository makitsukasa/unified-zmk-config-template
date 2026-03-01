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
        0x05, 0x04, 0x17, 0x17, 0x08, 0x15, 0x1C,  // BATTERY
        0x33,                                        // :
        0x1e, 0x27, 0x27,                           // 100
        0x22                                         // %
    };

    for (int i = 0; i < sizeof(battery_codes) / sizeof(battery_codes[0]); i++) {
        // Create and post keycode press event
        struct zmk_keycode_state_changed *press_event = new_zmk_keycode_state_changed();
        press_event->keycode = battery_codes[i];
        press_event->state = true;
        press_event->implicit = false;
        press_event->timestamp = k_uptime_get();
        zmk_event_raise(press_event);

        k_msleep(10);

        // Create and post keycode release event
        struct zmk_keycode_state_changed *release_event = new_zmk_keycode_state_changed();
        release_event->keycode = battery_codes[i];
        release_event->state = false;
        release_event->implicit = false;
        release_event->timestamp = k_uptime_get();
        zmk_event_raise(release_event);

        k_msleep(10);
    }

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
