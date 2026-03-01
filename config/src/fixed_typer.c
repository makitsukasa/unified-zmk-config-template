#include <zephyr/kernel.h>
#include <zmk/behaviors.h>
#include <zmk/hid.h>
#include <zmk/keys.h>

#define DT_DRV_COMPAT zmk_behavior_fixed_typer

// 1キーずつ押して離す補助関数
static void type_key(uint32_t usage_id) {
    zmk_hid_keyboard_press(usage_id);
    zmk_hid_keyboard_release(usage_id);
}

static int behavior_fixed_typer_init(const struct device *dev) { return 0; }

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    // 文字列 "HELLO" を順に入力
    type_key(ZMK_HID_USAGE_ID_KEY_KEYBOARD_H);
    type_key(ZMK_HID_USAGE_ID_KEY_KEYBOARD_E);
    type_key(ZMK_HID_USAGE_ID_KEY_KEYBOARD_L);
    type_key(ZMK_HID_USAGE_ID_KEY_KEYBOARD_L);
    type_key(ZMK_HID_USAGE_ID_KEY_KEYBOARD_O);
    return 0;
}

static const struct behavior_driver_api behavior_fixed_typer_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
};

DEVICE_DT_INST_DEFINE(0, behavior_fixed_typer_init, NULL,
                      NULL, NULL, POST_KERNEL,
                      CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                      &behavior_fixed_typer_driver_api);
