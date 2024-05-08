/* SPDX-FileCopyrightText: 2022-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "iot_button.h"
#include "esp_private/usb_phy.h"
#include "tinyusb_hid.h"

#define TAG "HID Example"

/* Button GPIO number */
#define EXAMPLE_BUTTON_NUM   4
#define EXAMPLE_BUTTON_UP    0
#define EXAMPLE_BUTTON_DOWN  11
#define EXAMPLE_BUTTON_LEFT  10
#define EXAMPLE_BUTTON_RIGHT 14

/* Mouse offset when button click */
#define EXAMPLE_MOUSE_OFFSET_X 8
#define EXAMPLE_MOUSE_OFFSET_Y 8

static int s_button_gpio[EXAMPLE_BUTTON_NUM] = {EXAMPLE_BUTTON_UP, EXAMPLE_BUTTON_DOWN, EXAMPLE_BUTTON_LEFT, EXAMPLE_BUTTON_RIGHT};
static button_handle_t s_button_handles[EXAMPLE_BUTTON_NUM] = {NULL};

static int get_button_gpio(button_handle_t btn_hdl)
{
    for (size_t i = 0; i < EXAMPLE_BUTTON_NUM; i++) {
        if (s_button_handles[i] == btn_hdl) {
            return s_button_gpio[i];
        }
    }
    return -1;
}

#ifdef CONFIG_SUBCLASS_KEYBOARD
#if CONFIG_ENABLE_FULL_KEY_KEYBOARD
static uint8_t keycode[15] = {0};

static void add_key(uint8_t key)
{
    // USAGE ID for keyboard starts from 4
    key -= 3;
    if (key >= 1 && key <= 120) {
        uint8_t byteIndex = (key - 1) / 8;
        uint8_t bitIndex = (key - 1) % 8;

        keycode[byteIndex] |= (1 << bitIndex);
    }
}

static void remove_key(uint8_t key)
{
    // USAGE ID for keyboard starts from 4
    key -= 3;
    if (key >= 1 && key <= 120) {
        uint8_t byteIndex = (key - 1) / 8;
        uint8_t bitIndex = (key - 1) % 8;

        keycode[byteIndex] &= ~(1 << bitIndex);
    }
}
#else
static uint8_t keycode[6] = {0};

static bool add_key(uint8_t key)
{
    for (size_t i = 0; i < 6; i++) {
        if (keycode[i] == 0) {
            keycode[i] = key;
            return true;
        }
    }
    return false;
}

static bool remove_key(uint8_t key)
{
    for (size_t i = 0; i < 6; i++) {
        if (keycode[i] == key) {
            keycode[i] = 0;
            return true;
        }
    }
    return false;
}
#endif
static void button_keyboard_cb(void *arg, void *arg2)
{
    button_handle_t button_hdl = (button_handle_t)arg;
    int button_gpio = get_button_gpio(button_hdl);
    uint8_t key = 0;
    switch (button_gpio) {
    case EXAMPLE_BUTTON_UP:
        key = HID_KEY_U;
        break;

    case EXAMPLE_BUTTON_DOWN:
        key = HID_KEY_D;
        break;

    case EXAMPLE_BUTTON_LEFT:
        key = HID_KEY_L;
        break;

    case EXAMPLE_BUTTON_RIGHT:
        key = HID_KEY_R;
        break;

    default:
        break;
    }

    if (iot_button_get_event(button_hdl) == BUTTON_PRESS_UP) {
        remove_key(key);
        ESP_LOGI(TAG, "Remove Keyboard %c", key - HID_KEY_A + 'a');
    } else {
        add_key(key);
        ESP_LOGI(TAG, "Add Keyboard %c", key - HID_KEY_A + 'a');
    }
    tinyusb_hid_keyboard_report(0, keycode);
}
#elif defined CONFIG_SUBCLASS_MOUSE
static void button_mouse_cb(void *arg, void *arg2)
{
    button_handle_t button_hdl = (button_handle_t)arg;
    int button_gpio = get_button_gpio(button_hdl);
    int mouse_offset_x = 0, mouse_offset_y = 0;
    switch (button_gpio) {
    case EXAMPLE_BUTTON_UP:
        mouse_offset_y = -EXAMPLE_MOUSE_OFFSET_Y;
        break;

    case EXAMPLE_BUTTON_DOWN:
        mouse_offset_y = EXAMPLE_MOUSE_OFFSET_Y;
        break;

    case EXAMPLE_BUTTON_LEFT:
        mouse_offset_x = -EXAMPLE_MOUSE_OFFSET_X;
        break;

    case EXAMPLE_BUTTON_RIGHT:
        mouse_offset_x = EXAMPLE_MOUSE_OFFSET_X;
        break;

    default:
        break;
    }
    tinyusb_hid_mouse_move_report(mouse_offset_x, mouse_offset_y, 0, 0);
    ESP_LOGI(TAG, "Mouse x=%d y=%d", mouse_offset_x, mouse_offset_y);
}
#endif

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

static void tusb_device_task(void *arg)
{
    while (1) {
        tud_task();
    }
}

// Invoked when device is mounted
void tud_mount_cb(void)
{
    ESP_LOGI(TAG, "USB Mount");
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    ESP_LOGI(TAG, "USB Un-Mount");
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    ESP_LOGI(TAG, "USB Suspend");
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    ESP_LOGI(TAG, "USB Resume");
}

//--------------------------------------------------------------------+
// USB PHY config
//--------------------------------------------------------------------+
static void usb_phy_init(void)
{
    usb_phy_handle_t phy_hdl;
    // Configure USB PHY
    usb_phy_config_t phy_conf = {
        .controller = USB_PHY_CTRL_OTG,
        .otg_mode = USB_OTG_MODE_DEVICE,
    };
    phy_conf.target = USB_PHY_TARGET_INT;
    usb_new_phy(&phy_conf, &phy_hdl);
}

void app_main(void)
{
    // switch esp usb phy to usb-otg
    usb_phy_init();
    tud_init(BOARD_TUD_RHPORT);
    xTaskCreate(tusb_device_task, "TinyUSB", 4096, NULL, 5, NULL);
    tinyusb_hid_init();

    /* buttons init, buttons used to simulate keyboard or mouse events */
    button_config_t cfg = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
            .active_level = 0,
        },
    };

    for (size_t i = 0; i < EXAMPLE_BUTTON_NUM; i++) {
        cfg.gpio_button_config.gpio_num = s_button_gpio[i];
        s_button_handles[i] = iot_button_create(&cfg);
        if (s_button_handles[i] == NULL) {
            ESP_LOGE(TAG, "Button io = %d created failed", s_button_gpio[i]);
            assert(0);
        } else {
            ESP_LOGI(TAG, "Button io = %d created", s_button_gpio[i]);
        }
    }

#ifdef CONFIG_SUBCLASS_KEYBOARD
    button_cb_t button_cb = button_keyboard_cb;
    ESP_LOGI(TAG, "HID Keyboard demo: press button to simulate keyboard");
#elif defined CONFIG_SUBCLASS_MOUSE
    button_cb_t button_cb = button_mouse_cb;
    ESP_LOGI(TAG, "HID Mouse demo: press button to simulate mouse");
#endif
    ESP_LOGI(TAG, "Wait Mount through USB interface");

    /* register button callback, send HID report when click button */
    for (size_t i = 0; i < EXAMPLE_BUTTON_NUM; i++) {
#ifdef CONFIG_SUBCLASS_KEYBOARD
        iot_button_register_cb(s_button_handles[i], BUTTON_PRESS_DOWN, button_cb, NULL);
        iot_button_register_cb(s_button_handles[i], BUTTON_PRESS_UP, button_cb, NULL);
#elif defined CONFIG_SUBCLASS_MOUSE
        iot_button_register_cb(s_button_handles[i], BUTTON_SINGLE_CLICK, button_cb, NULL);
#endif
    }

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
