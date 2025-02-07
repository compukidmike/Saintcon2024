#include <math.h>
#include <memory.h>
#include "esp_log.h"
#include "esp_random.h"

#include "hwtest.h"
#include "ui.h"
#include "theme.h"

// Includes for things to test
#include "accel.h"
#include "badge.h"
#include "charger.h"
#include "i2c_manager.h"
#include "ir_comm.h"
#include "load_switch.h"
#include "power_manager.h"
#include "type_c.h"

static const char *TAG = "screens/hwtest";

// Types of tests
typedef enum {
    HWTEST_NONE,  // No test selected
    HWTEST_CHIPS, // Test basic I2C communication with the various chips on the badge (Accelerometer, Type-C, Charger, I2C Mux,
                  // Load Switch)
    HWTEST_ACCEL, // Accelerometer - show tilt in a square and show directional tap/bump detection
    HWTEST_POWER, // Power management - state of battery, charging, USB Type-C, etc.
    HWTEST_TOUCH, // Touch screen - show a circle that follows the touch point
    HWTEST_SCREW, // Detect power fault from the load switch when screws are shorted
    HWTEST_IR,    // IR loopback - send a code and receive it back
    HWTEST_MINIBADGE, // Test the mini badge connectors - just scan the I2C bus for devices and show the EEPROM contents for each
                      // slot
} hwtest_t;

// Strings for the hardware test names
static const char *hwtest_labels[] = {
    "None", "Chip Comms", "Accelerometer", "Power", "Touch", "Screw Power", "IR Tx/Rx", "Mini Badges",
};
#define HWTEST_COUNT (sizeof(hwtest_labels) / sizeof(hwtest_labels[0]))

// Test result states
typedef enum {
    HWTEST_RESULT_NONE,
    HWTEST_RESULT_PASSED,
    HWTEST_RESULT_FAILED,
} hwtest_result_t;

// Track the state of each hardware test
static hwtest_result_t hwtest_results[HWTEST_COUNT] = {HWTEST_RESULT_NONE};

// Button types to handle
typedef enum {
    HWTEST_BTN_PASS,
    HWTEST_BTN_FAIL,
} hwtest_btn_t;

static lv_obj_t *scr_hwtest       = NULL;
static lv_obj_t *hwtest_container = NULL;
static lv_obj_t *hwtest_list      = NULL;
static lv_style_t container_style;
static lv_style_t pass_main;
static lv_style_t pass_light;
static lv_style_t fail_main;
static lv_style_t fail_light;
static lv_style_t button_row;

static bool styles_initialized = false;

// Chip test objects
typedef enum {
    HWTEST_CHIP_I2C,
    HWTEST_CHIP_USB_C,
    HWTEST_CHIP_CHARGER,
    HWTEST_CHIP_ACCEL,
} chips_t;
static const char *chips[] = {"I2C Mux", "USB-C Controller", "Battery Charger", "Accelerometer"};
#define CHIP_TEST_COUNT (sizeof(chips) / sizeof(chips[0]))
static chips_t chip_test_current                          = HWTEST_CHIP_I2C;
static hwtest_result_t chip_test_results[CHIP_TEST_COUNT] = {HWTEST_RESULT_NONE};

static lv_obj_t *chip_comms_overlay = NULL;
static lv_obj_t *chip_test_list     = NULL;

// Accelerometer test objects
static lv_obj_t *accel_overlay        = NULL;
static lv_obj_t *accel_dot            = NULL;
static TaskHandle_t accel_task_handle = NULL;

// Power test objects
static lv_obj_t *battery_value_label;
static lv_obj_t *usb_c_value_label;
static lv_obj_t *vbus_value_label;
static lv_obj_t *power_test_overlay   = NULL;
static TaskHandle_t power_task_handle = NULL;

// Touch test objects
static lv_obj_t *touch_overlay     = NULL;
static lv_obj_t *touch_test_circle = NULL;
static lv_obj_t *touch_fail_button = NULL;

// Screw power (load switch) test objects
static lv_obj_t *screw_power_overlay            = NULL;
static lv_obj_t *load_switch_enable_state_label = NULL;
static lv_obj_t *load_switch_flag_state_label   = NULL;
static TaskHandle_t load_switch_task_handle     = NULL;

// IR test objects
typedef struct {
    uint16_t address;
    uint16_t command;
} ir_code_base_t;
static lv_obj_t *ir_overlay            = NULL;
static lv_obj_t *ir_tx_value_label     = NULL;
static lv_obj_t *ir_rx_value_label     = NULL;
static lv_timer_t *ir_test_timer       = NULL;
static ir_code_base_t ir_test_codes[8] = {
    {0x00FF, 0x00FF}, {0x10FF, 0x807F}, {0x20FF, 0x40BF}, {0x30FF, 0x20DF},
    {0x40FF, 0xA05F}, {0x50FF, 0x609F}, {0x60FF, 0x10EF}, {0x70FF, 0x906F},
};
static ir_code_base_t ir_test_codes_received[8] = {0};
#define IR_TEST_COUNT (sizeof(ir_test_codes) / sizeof(ir_test_codes[0]))
static int ir_test_current = 0;

// Minibadge test objects
static lv_obj_t *minibadge_overlay                     = NULL;
static lv_obj_t *minibadge_list                        = NULL;
static lv_timer_t *minibadge_timer                     = NULL;
static i2c_manager_device_config_t minibadge_devices[] = {
    {.bus_index = I2C_BUS_OTHER,
     .channel   = I2C_SWITCH_CHANNEL_0,
     .config    = {.dev_addr_length = I2C_ADDR_BIT_LEN_7, .device_address = 0x50, .scl_speed_hz = 100000}},
    {.bus_index = I2C_BUS_OTHER,
     .channel   = I2C_SWITCH_CHANNEL_1,
     .config    = {.dev_addr_length = I2C_ADDR_BIT_LEN_7, .device_address = 0x50, .scl_speed_hz = 100000}},
};

// Main function prototypes
static void hwtest_initialize_styles();
static void hwtest_update_theme();
static void hwtest_render_list(lv_obj_t *parent);
static void hwtest_list_event_cb(lv_event_t *e);
static void hwtest_button_event_cb(lv_event_t *e);
static void hwtest_set_result(hwtest_t test, hwtest_result_t result);

// Quick test functions
static void global_touch_event_cb(lv_event_t *e);
static void hwtest_quick_tests();

// Chip test functions
static void hwtest_start_chip_test(lv_obj_t *parent);
static void chip_test_run_next(lv_timer_t *timer);
static void chip_test_render_results();
static void chip_test_continue_event_cb(lv_event_t *e);
static bool test_i2c_comms();
static bool test_usb_c_comms();
static bool test_charger_comms();
static bool test_accel_comms();

// Accelerometer test functions
static void hwtest_start_accel_test(lv_obj_t *parent);
static void accel_test_task(void *_arg);
static void accel_test_end();

// Power test functions
static void hwtest_start_power_test(lv_obj_t *parent);
static void power_test_btn_event_cb(lv_event_t *e);
static void power_test_task(void *_arg);
static void power_test_end(bool success);

// Touch test functions
static void hwtest_start_touch_test(lv_obj_t *parent);
static void touch_circle_event_cb(lv_event_t *e);
static void touch_test_end(bool success);

// Screw power test functions
static void hwtest_start_screw_power_test(lv_obj_t *parent);
static void load_switch_test_btn_event_cb(lv_event_t *e);
static void load_switch_test_task(void *_arg);
static void load_switch_test_end(bool success);

// IR test functions
static void hwtest_start_ir_test(lv_obj_t *parent);
static void ir_test_btn_event_cb(lv_event_t *e);
static void ir_test_run_next(lv_timer_t *timer);
static void ir_test_rx_callback(uint16_t address, uint16_t command);
static void ir_test_end(bool success);

// Minibadge test functions
static void hwtest_start_minibadge_test(lv_obj_t *parent);
static void minibadge_test_btn_event_cb(lv_event_t *e);
static void minibadge_test_scan(lv_timer_t *timer);
static void minibadge_test_end(bool success);

static void hwtest_initialize_styles() {
    if (styles_initialized) {
        return;
    }

    // Initialize the flex container base style
    lv_style_init(&container_style);
    lv_style_set_bg_color(&container_style, lv_color_hex(BLACK));
    lv_style_set_border_color(&container_style, lv_color_hex(GRAY_BASE));
    lv_style_set_text_color(&container_style, lv_color_hex(WHITE));
    lv_style_set_border_width(&container_style, 2);
    lv_style_set_radius(&container_style, 1);
    lv_style_set_size(&container_style, LV_HOR_RES, LV_VER_RES);

    // Initialize the main pass style
    lv_style_init(&pass_main);
    lv_style_set_bg_color(&pass_main, lv_color_hex(GREEN_DIMMEST));
    lv_style_set_border_color(&pass_main, lv_color_hex(GREEN_MAIN));

    // Initialize the light pass style
    lv_style_init(&pass_light);
    lv_style_set_bg_color(&pass_light, lv_color_hex(GREEN_DIM));
    lv_style_set_border_color(&pass_light, lv_color_hex(GREEN_LIGHT));

    // Initialize the main fail style
    lv_style_init(&fail_main);
    lv_style_set_bg_color(&fail_main, lv_color_hex(RED_DIMMEST));
    lv_style_set_border_color(&fail_main, lv_color_hex(RED_MAIN));

    // Initialize the light fail style
    lv_style_init(&fail_light);
    lv_style_set_bg_color(&fail_light, lv_color_hex(RED_DIM));
    lv_style_set_border_color(&fail_light, lv_color_hex(RED_LIGHT));

    // Initialize the button row style
    lv_style_init(&button_row);
    lv_style_set_size(&button_row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_style_set_bg_opa(&button_row, LV_OPA_TRANSP);
    lv_style_set_border_width(&button_row, 0);

    styles_initialized = true;
}

lv_obj_t *create_hwtest_screen() {
    static ui_screen_t screen_type = SCREEN_HWTEST;

    // Initialize the styles for the hardware test screen
    hwtest_initialize_styles();

    // Create a screen object for the hardware test screen
    scr_hwtest = lv_obj_create(NULL);
    lv_obj_set_user_data(scr_hwtest, &screen_type);
    lv_obj_set_size(scr_hwtest, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(scr_hwtest, lv_color_hex(BLACK), LV_PART_MAIN);
    lv_obj_remove_flag(scr_hwtest, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(scr_hwtest, LV_OBJ_FLAG_CLICKABLE);

    // Add the global touch event callback to the screen for the initial quick tests
    lv_obj_add_event_cb(scr_hwtest, global_touch_event_cb, LV_EVENT_ALL, NULL);

    // Create a container to hold the list of tests and the bottom pass/fail buttons
    hwtest_container = lv_obj_create(scr_hwtest);
    lv_obj_add_style(hwtest_container, &container_style, LV_PART_MAIN);
    lv_obj_add_style(hwtest_container, &pass_main, LV_PART_MAIN);
    lv_obj_remove_flag(hwtest_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(hwtest_container, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_flex_flow(hwtest_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(hwtest_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_add_flag(hwtest_container, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Create the list of hardware tests
    hwtest_render_list(hwtest_container);

    // Create the pass/fail buttons
    lv_obj_t *bottom_buttons = lv_obj_create(hwtest_container);
    lv_obj_add_style(bottom_buttons, &button_row, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(bottom_buttons, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(bottom_buttons, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(bottom_buttons, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_grow(bottom_buttons, 0);

    lv_obj_t *pass_btn = lv_button_create(bottom_buttons);
    lv_obj_add_style(pass_btn, &button_style, LV_PART_MAIN);
    lv_obj_add_style(pass_btn, &pass_light, LV_PART_MAIN);
    lv_obj_set_style_margin_all(pass_btn, 2, LV_PART_MAIN);
    lv_obj_t *pass_label = lv_label_create(pass_btn);
    lv_label_set_text(pass_label, "Pass");
    lv_obj_add_event_cb(pass_btn, hwtest_button_event_cb, LV_EVENT_CLICKED, (void *)HWTEST_BTN_PASS);

    // Run the quick tests
    hwtest_quick_tests();

    return scr_hwtest;
}

static void hwtest_update_theme() {
    bool any_failed = false;
    for (int i = 1; i < HWTEST_COUNT; i++) {
        if (hwtest_results[i] == HWTEST_RESULT_FAILED) {
            any_failed = true;
            break;
        }
    }

    lv_style_t *set_style_main  = any_failed ? &fail_main : &pass_main;
    lv_style_t *set_style_light = any_failed ? &fail_light : &pass_light;

    // Update the styles for the hardware test screen
    lv_obj_remove_style(hwtest_container, &pass_main, LV_PART_MAIN);
    lv_obj_remove_style(hwtest_container, &fail_main, LV_PART_MAIN);
    lv_obj_add_style(hwtest_container, set_style_main, LV_PART_MAIN);

    // Update the list scrollbar style
    lv_obj_remove_style(hwtest_list, &pass_main, LV_PART_SCROLLBAR);
    lv_obj_remove_style(hwtest_list, &fail_main, LV_PART_SCROLLBAR);
    lv_obj_add_style(hwtest_list, set_style_main, LV_PART_SCROLLBAR);

    // Update the list item border color for each list item
    lv_style_value_t border_color_main;
    lv_style_get_prop(set_style_main, LV_STYLE_BORDER_COLOR, &border_color_main);
    for (int i = 1; i < HWTEST_COUNT; i++) {
        lv_obj_t *item = lv_obj_get_child(hwtest_list, i - 1);
        if (item == NULL) {
            ESP_LOGW(TAG, "Failed to find hardware test item in list");
            continue;
        }
        lv_obj_set_style_border_color(item, border_color_main.color, LV_PART_MAIN);
    }

    // Update the pass/fail button styles
    lv_obj_t *bottom_buttons = lv_obj_get_child(hwtest_container, -1);
    lv_obj_t *pass_button    = lv_obj_get_child(bottom_buttons, 0);
    lv_obj_remove_style(pass_button, &pass_light, LV_PART_MAIN);
    lv_obj_remove_style(pass_button, &fail_light, LV_PART_MAIN);
    lv_obj_add_style(pass_button, set_style_light, LV_PART_MAIN);

    // Disable the pass button if any tests have failed
    lv_obj_set_state(pass_button, LV_STATE_DISABLED, any_failed);
}

static void hwtest_set_result(hwtest_t test, hwtest_result_t result) {
    if (test < 0 || test >= HWTEST_COUNT) {
        ESP_LOGW(TAG, "Invalid hardware test index");
        return;
    }
    hwtest_results[test] = result;

    // Find the item in the list and update the icon
    if (hwtest_list != NULL) {
        lv_obj_t *item = lv_obj_get_child(hwtest_list, test - 1);
        if (item == NULL) {
            return;
        }
        lv_obj_t *icon = lv_obj_get_child(item, -1);
        if (icon == NULL) {
            return;
        }
        lv_image_set_src(icon, result == HWTEST_RESULT_NONE     ? LV_SYMBOL_MINUS
                               : result == HWTEST_RESULT_PASSED ? LV_SYMBOL_OK
                                                                : LV_SYMBOL_CLOSE);
    } else {
        ESP_LOGW(TAG, "Hardware test list not created");
    }

    // Update the theme based on the test results
    hwtest_update_theme();
}

static void hwtest_render_list(lv_obj_t *parent) {
    // Delete the existing list if it exists so we can recreate it
    if (hwtest_list != NULL && lv_obj_is_valid(hwtest_list)) {
        lv_obj_delete(hwtest_list);
    }
    hwtest_list = NULL;

    // Create a list of the hardware tests that can be run
    hwtest_list = lv_list_create(parent);
    lv_obj_set_size(hwtest_list, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(hwtest_list, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(hwtest_list, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(hwtest_list, 0, LV_PART_MAIN);
    lv_obj_set_flex_grow(hwtest_list, 1);
    lv_obj_add_flag(hwtest_list, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Set the scrollbar color and style
    lv_obj_add_style(hwtest_list, &pass_main, LV_PART_SCROLLBAR);
    lv_obj_set_style_radius(hwtest_list, 0, LV_PART_SCROLLBAR);

    // Add the hardware tests to the list
    for (int i = 1; i < HWTEST_COUNT; i++) {
        lv_obj_t *btn = lv_list_add_button(hwtest_list, NULL, hwtest_labels[i]);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_EVENT_BUBBLE);

        // List item button style and alignment
        lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_color(btn, lv_color_hex(GREEN_MAIN), LV_PART_MAIN);
        lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

        // Style the button label
        lv_obj_set_style_text_font(btn, &clarity_14, LV_PART_MAIN);
        lv_obj_set_style_text_color(btn, lv_color_hex(WHITE), LV_PART_MAIN);

        // Add an icon on the right to show the test result
        hwtest_result_t result = hwtest_results[i];
        lv_obj_t *result_icon  = lv_image_create(btn);
        lv_obj_set_size(result_icon, 18, 18);
        lv_image_set_inner_align(result_icon, LV_IMAGE_ALIGN_CENTER);
        lv_obj_set_style_text_font(result_icon, &lv_font_montserrat_14, LV_PART_MAIN);
        lv_image_set_src(result_icon, result == HWTEST_RESULT_NONE     ? LV_SYMBOL_MINUS
                                      : result == HWTEST_RESULT_PASSED ? LV_SYMBOL_OK
                                                                       : LV_SYMBOL_CLOSE);

        // Add an event handler to the button
        lv_obj_add_event_cb(btn, hwtest_list_event_cb, LV_EVENT_CLICKED, (void *)i);
    }
}

static void hwtest_list_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        hwtest_t test = (hwtest_t)lv_event_get_user_data(e);
        if (test == HWTEST_NONE || test >= HWTEST_COUNT) {
            ESP_LOGW(TAG, "Invalid hardware test selected");
            return;
        }
        const char *label = hwtest_labels[test];
        ESP_LOGI(TAG, "Running hardware test: %s", label);

        // Run the selected test
        switch (test) {
            case HWTEST_CHIPS: //
                hwtest_start_chip_test(scr_hwtest);
                break;
            case HWTEST_ACCEL: //
                hwtest_start_accel_test(scr_hwtest);
                break;
            case HWTEST_POWER: //
                hwtest_start_power_test(scr_hwtest);
                break;
            case HWTEST_TOUCH: //
                hwtest_start_touch_test(scr_hwtest);
                break;
            case HWTEST_SCREW: //
                hwtest_start_screw_power_test(scr_hwtest);
                break;
            case HWTEST_IR: //
                hwtest_start_ir_test(scr_hwtest);
                break;
            case HWTEST_MINIBADGE: //
                hwtest_start_minibadge_test(scr_hwtest);
                break;
            default: break;
        }
    }
}

static void hwtest_button_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    hwtest_btn_t btn     = (hwtest_btn_t)(intptr_t)lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED) {
        switch (btn) {
            case HWTEST_BTN_PASS: //
                // Save the flag in the badge configuration
                badge_config.hw_pass = true;
                save_badge_config();

                // Go to the main menu screen
                set_screen(SCREEN_MAIN);
                break;
            default: break;
        }
    }
}

/*****************************************************
 * Quick Test Functions                              *
 *****************************************************/

static void global_touch_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_PRESSED || code == LV_EVENT_PRESSING || code == LV_EVENT_PRESS_LOST) {
        if (hwtest_results[HWTEST_TOUCH] == HWTEST_RESULT_NONE) {
            hwtest_set_result(HWTEST_TOUCH, HWTEST_RESULT_PASSED);
        }

        // Remove the touch event callback
        lv_obj_remove_event_cb(scr_hwtest, global_touch_event_cb);

        // Remove event bubble flag from the test list and it's buttons
        lv_obj_clear_flag(hwtest_list, LV_OBJ_FLAG_EVENT_BUBBLE);
        for (int i = 1; i < HWTEST_COUNT; i++) {
            lv_obj_t *item = lv_obj_get_child(hwtest_list, i - 1);
            if (item == NULL) {
                ESP_LOGW(TAG, "Failed to find hardware test item in list");
                continue;
            }
            lv_obj_clear_flag(item, LV_OBJ_FLAG_EVENT_BUBBLE);
        }
    }
}

static void hwtest_quick_tests() {
    // Run quick tests for chip communications
    bool chip_pass = test_i2c_comms() && test_usb_c_comms() && test_charger_comms() && test_accel_comms();
    hwtest_set_result(HWTEST_CHIPS, chip_pass ? HWTEST_RESULT_PASSED : HWTEST_RESULT_FAILED);

    // Quick test for accelerometer communication
    mc3419_accel_data_t accel_data = {0};
    hwtest_set_result(HWTEST_ACCEL, accel_read_data(&accel_data) == ESP_OK &&
                                            (accel_data.x.raw != 0 || accel_data.y.raw != 0 || accel_data.z.raw != 0)
                                        ? HWTEST_RESULT_PASSED
                                        : HWTEST_RESULT_FAILED);

    // Quick test for screw power state
    uint8_t mux_control = 0;
    if (i2c_switch_get_status(&mux_control) == ESP_OK) {
        bool enabled = load_switch_enabled();
        bool flag    = ((mux_control >> 4) & LOAD_SWITCH_INTERRUPT) ? true : false;
        hwtest_set_result(HWTEST_SCREW, flag ? HWTEST_RESULT_FAILED : enabled ? HWTEST_RESULT_PASSED : HWTEST_RESULT_NONE);
    }

    // Quick test for IR communication
    badge_ir_enable_rx_buffer(false);
    badge_ir_add_rx_callback(ir_test_rx_callback);
    memset(ir_test_codes_received, 0, sizeof(ir_test_codes_received));
    ir_code_base_t ir_code = ir_test_codes[7];
    ir_test_current        = 1;
    bool ir_test_passed    = false;

    for (int attempt = 0; attempt < 3; attempt++) {
        if (ir_transmit(ir_code.address, ir_code.command) == ESP_OK) {
            ESP_LOGD(TAG, "Sent IR code: 0x%04X 0x%04X", ir_code.address, ir_code.command);
            // Wait briefly for the code to be received
            vTaskDelay(pdMS_TO_TICKS(700));

            // Check if received code matches the sent code
            if (ir_test_codes_received[ir_test_current - 1].address == ir_code.address &&
                ir_test_codes_received[ir_test_current - 1].command == ir_code.command) {
                ir_test_passed = true;
                break;
            } else {
                ESP_LOGW(TAG, "Received IR code 0x%04X 0x%04X does not match sent code",
                         ir_test_codes_received[ir_test_current - 1].address,
                         ir_test_codes_received[ir_test_current - 1].command);
            }
        }
    }

    ir_test_current = 0;
    memset(ir_test_codes_received, 0, sizeof(ir_test_codes_received));
    hwtest_set_result(HWTEST_IR, ir_test_passed ? HWTEST_RESULT_PASSED : HWTEST_RESULT_FAILED);
    badge_ir_remove_rx_callback(ir_test_rx_callback);
    badge_ir_enable_rx_buffer(true);

    // Quick test for minibadge presence
    bool minibadge_pass = true;
    for (int i = 0; i < sizeof(minibadge_devices) / sizeof(minibadge_devices[0]); i++) {
        uint8_t data[1];
        if (i2c_manager_read_eeprom(&minibadge_devices[i], 0x00, data, 1) != ESP_OK) {
            minibadge_pass = false;
            break;
        }
    }
    hwtest_set_result(HWTEST_MINIBADGE, minibadge_pass ? HWTEST_RESULT_PASSED : HWTEST_RESULT_NONE);
}

/*****************************************************
 * Chip Test Functions                               *
 *****************************************************/

static void hwtest_start_chip_test(lv_obj_t *parent) {
    chip_comms_overlay = lv_obj_create(parent);
    lv_obj_set_size(chip_comms_overlay, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(chip_comms_overlay, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(chip_comms_overlay, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(chip_comms_overlay, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(chip_comms_overlay, 0, LV_PART_MAIN);
    lv_obj_align(chip_comms_overlay, LV_ALIGN_CENTER, 0, 0);
    lv_obj_remove_flag(chip_comms_overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(chip_comms_overlay, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(chip_comms_overlay, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

    // Create a list for the chip test results
    chip_test_list = lv_list_create(chip_comms_overlay);
    lv_obj_set_size(chip_test_list, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(chip_test_list, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(chip_test_list, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(chip_test_list, 0, LV_PART_MAIN);
    lv_obj_set_style_text_color(chip_test_list, lv_color_hex(GRAY_SHADE_2), LV_PART_MAIN);
    lv_obj_align(chip_test_list, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_flex_grow(chip_test_list, 1);

    // Add devices to the list
    for (int i = 0; i < CHIP_TEST_COUNT; i++) {
        lv_obj_t *item = lv_list_add_text(chip_test_list, chips[i]);
        lv_obj_set_style_bg_opa(item, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_text_font(item, &clarity_16, LV_PART_MAIN);

        // Add an icon on the right to show the test result
        lv_obj_t *result_icon = lv_image_create(item);
        lv_obj_set_size(result_icon, 18, 18);
        lv_image_set_inner_align(result_icon, LV_IMAGE_ALIGN_CENTER);
        lv_image_set_src(result_icon, LV_SYMBOL_MINUS);
        lv_obj_set_style_text_font(result_icon, &lv_font_montserrat_14, LV_PART_MAIN);
        lv_obj_set_style_text_color(result_icon, lv_color_hex(GRAY_TINT_1), LV_PART_MAIN);
        lv_obj_align(result_icon, LV_ALIGN_RIGHT_MID, 0, 0);
    }

    // Button style for the bottom pass/fail buttons
    static lv_style_t btn_style;
    lv_style_init(&btn_style);
    lv_style_set_bg_color(&btn_style, lv_color_hex(GREEN_DIM));
    lv_style_set_border_width(&btn_style, 2);
    lv_style_set_border_color(&btn_style, lv_color_hex(GREEN_MAIN));
    lv_style_set_radius(&btn_style, 1);
    lv_style_set_text_color(&btn_style, lv_color_hex(WHITE));
    lv_style_set_text_font(&btn_style, &cyberphont3b_16);

    // Create a "Continue" button
    lv_obj_t *continue_btn = lv_button_create(chip_comms_overlay);
    lv_obj_add_style(continue_btn, &btn_style, LV_PART_MAIN);
    lv_obj_t *continue_label = lv_label_create(continue_btn);
    lv_label_set_text(continue_label, "Continue");
    lv_obj_center(continue_label);
    lv_obj_add_event_cb(continue_btn, chip_test_continue_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_state(continue_btn, LV_STATE_DISABLED, true);

    // Reset the state and results for the chip tests
    chip_test_current = 0;
    for (int i = 0; i < CHIP_TEST_COUNT; i++) {
        chip_test_results[i] = HWTEST_RESULT_NONE;
    }

    // Run the chip test
    lv_timer_t *timer = lv_timer_create(chip_test_run_next, 1000, NULL);
    lv_timer_set_repeat_count(timer, CHIP_TEST_COUNT);
    lv_timer_ready(timer);
}

static void chip_test_run_next(lv_timer_t *timer) {
    if (chip_test_current >= CHIP_TEST_COUNT) {
        lv_timer_delete(timer);
        chip_test_current = 0;
        chip_test_render_results();
        return;
    }

    hwtest_result_t result = HWTEST_RESULT_NONE;
    switch (chip_test_current) {
        case HWTEST_CHIP_I2C: //
            result = test_i2c_comms() ? HWTEST_RESULT_PASSED : HWTEST_RESULT_FAILED;
            break;
        case HWTEST_CHIP_USB_C: //
            result = test_usb_c_comms() ? HWTEST_RESULT_PASSED : HWTEST_RESULT_FAILED;
            break;
        case HWTEST_CHIP_CHARGER: //
            result = test_charger_comms() ? HWTEST_RESULT_PASSED : HWTEST_RESULT_FAILED;
            break;
        case HWTEST_CHIP_ACCEL: //
            result = test_accel_comms() ? HWTEST_RESULT_PASSED : HWTEST_RESULT_FAILED;
            break;
        default: break;
    }

    // Update the test results list
    chip_test_results[chip_test_current] = result;
    chip_test_render_results();

    // Enable the "Continue" button if all tests have been run
    if (chip_test_current == CHIP_TEST_COUNT - 1) {
        lv_obj_t *continue_btn = lv_obj_get_child(chip_comms_overlay, -1);
        lv_obj_set_state(continue_btn, LV_STATE_DISABLED, false);
    }

    // Increment the current test index for the next run
    chip_test_current++;
}

static void chip_test_render_results() {
    for (int i = 0; i < CHIP_TEST_COUNT; i++) {
        lv_obj_t *item = lv_obj_get_child(chip_test_list, i);
        if (item == NULL) {
            ESP_LOGW(TAG, "Failed to find chip test item in list");
            return;
        }
        lv_obj_t *icon = lv_obj_get_child(item, -1);
        if (icon == NULL) {
            icon = lv_image_create(item);
            lv_obj_set_size(icon, 18, 18);
            lv_image_set_inner_align(icon, LV_IMAGE_ALIGN_CENTER);
        }
        lv_image_set_src(icon, chip_test_results[i] == HWTEST_RESULT_NONE     ? LV_SYMBOL_MINUS
                               : chip_test_results[i] == HWTEST_RESULT_PASSED ? LV_SYMBOL_OK
                                                                              : LV_SYMBOL_CLOSE);
        lv_obj_set_style_text_color(icon,
                                    chip_test_results[i] == HWTEST_RESULT_NONE     ? lv_color_hex(GRAY_TINT_1)
                                    : chip_test_results[i] == HWTEST_RESULT_PASSED ? lv_color_hex(GREEN_MAIN)
                                                                                   : lv_color_hex(RED_MAIN),
                                    LV_PART_MAIN);
    }
}

static void chip_test_continue_event_cb(lv_event_t *e) {
    lv_obj_delete(chip_comms_overlay);
    chip_comms_overlay = NULL;

    // Determine the overall result based on individual tests
    bool overall_result = true;
    for (int i = 0; i < CHIP_TEST_COUNT; i++) {
        if (chip_test_results[i] != HWTEST_RESULT_PASSED) {
            overall_result = false;
            break;
        }
    }
    hwtest_set_result(HWTEST_CHIPS, overall_result ? HWTEST_RESULT_PASSED : HWTEST_RESULT_FAILED);
}

static bool test_i2c_comms() {
    // Test that we can switch to all 4 I2C channels
    bool result = true;
    I2C_BUS_LOCK(I2C_BUS_OTHER, false)
    for (uint8_t chan = 0; chan < 4; chan++) {
        if (i2c_switch_select(chan) != ESP_OK) {
            result = false;
        }
    }
    I2C_BUS_UNLOCK(I2C_BUS_OTHER);

    return result;
}

static bool test_usb_c_comms() {
    // Test that we can read all 4 registers from the USB Type-C controller
    uint8_t data[4] = {0};
    esp_err_t ret   = typec_read_all(data);
    if (ret != ESP_OK) {
        return false;
    }

    return true;
}

static bool test_charger_comms() {
    // Test that we can read the charger status
    charger_system_status_t status = {0};
    esp_err_t ret                  = charger_read_status(&status);
    if (ret != ESP_OK) {
        return false;
    }

    return true;
}

static bool test_accel_comms() {
    // Test that we can read the accelerometer chip ID
    mc3419_chip_id_t id = {0};
    esp_err_t ret       = accel_get_device_id(&id);
    if (ret != ESP_OK || id.chip_id != MC3419_CHIP_ID) {
        return false;
    }

    return true;
}

/*****************************************************
 * Accelerometer Test Functions                      *
 *****************************************************/

static void hwtest_start_accel_test(lv_obj_t *parent) {
    accel_overlay = lv_obj_create(parent);
    lv_obj_set_size(accel_overlay, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(accel_overlay, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(accel_overlay, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(accel_overlay, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(accel_overlay, 0, LV_PART_MAIN);
    lv_obj_align(accel_overlay, LV_ALIGN_CENTER, 0, 0);
    lv_obj_remove_flag(accel_overlay, LV_OBJ_FLAG_SCROLLABLE);

    // Create a circle to show the tilt of the accelerometer
    accel_dot = lv_obj_create(accel_overlay);
    lv_obj_set_size(accel_dot, 30, 30);
    lv_obj_set_style_bg_color(accel_dot, lv_color_hex(GRAY_TINT_3), LV_PART_MAIN);
    lv_obj_set_style_border_width(accel_dot, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(accel_dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_align(accel_dot, LV_ALIGN_CENTER, 0, 0);

    // End the test after 7 seconds
    lv_timer_t *timer = lv_timer_create(accel_test_end, 7000, NULL);
    lv_timer_set_repeat_count(timer, 1);
    lv_timer_set_auto_delete(timer, true);

    // Start the accelerometer polling task
    xTaskCreate(accel_test_task, "accel_test_task", 4096, NULL, 5, &accel_task_handle);
}

static void accel_test_task(void *_arg) {
    // Fetch the current accelerometer range
    mc3419_range_t range = {0};
    accel_get_range(&range);
    ESP_LOGD(TAG, "Accelerometer range: %d", range.range);

    while (true) {
        // Get the accelerometer data
        mc3419_accel_data_t accel_data = {0};
        esp_err_t ret                  = accel_read_data(&accel_data);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read accelerometer data");
            continue;
        }

        // Calculate the tilt angle of the accelerometer
        float x = accel_data.x.value / mc3419_resolution_si * mc3419_range_si[range.range];
        float y = accel_data.y.value / mc3419_resolution_si * mc3419_range_si[range.range];
        float z = accel_data.z.value / mc3419_resolution_si * mc3419_range_si[range.range];

        // Calculate the tilt in degrees
        float tilt_x = atan2f(y, z) * 180.0f / M_PI;
        float tilt_y = atan2f(x, z) * 180.0f / M_PI;
        float tilt_z = atan2f(x, y) * 180.0f / M_PI;

        // Get screen coordinates for the tilt
        int pos_x = (int)(tilt_x * (LV_HOR_RES / 2) / 90.0f);
        int pos_y = (int)(tilt_y * (LV_VER_RES / 2) / 90.0f);

        // Update the position of the circle based on the tilt
        if (lvgl_lock(pdMS_TO_TICKS(100), __FILE__, __LINE__) != pdTRUE) {
            ESP_LOGW(TAG, "Failed to lock LVGL");
            continue;
        }
        lv_obj_set_pos(accel_dot, pos_x, pos_y);
        lvgl_unlock(__FILE__, __LINE__);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void accel_test_end() {
    bool success = false;

    // Make sure the dot has moved from the center for the test to pass
    lv_point_t pos;
    pos.x = lv_obj_get_x_aligned(accel_dot);
    pos.y = lv_obj_get_y_aligned(accel_dot);
    if (pos.x != 0 || pos.y != 0) {
        success = true;
    }

    if (success) {
        hwtest_set_result(HWTEST_ACCEL, HWTEST_RESULT_PASSED);
    } else {
        hwtest_set_result(HWTEST_ACCEL, HWTEST_RESULT_FAILED);
    }
    if (accel_overlay != NULL) {
        lv_obj_delete(accel_overlay);
        accel_overlay = NULL;
    }
    if (accel_task_handle != NULL) {
        vTaskDelete(accel_task_handle);
        accel_task_handle = NULL;
    }
}

/*****************************************************
 * Power Test Functions                              *
 *****************************************************/

static void hwtest_start_power_test(lv_obj_t *parent) {
    // Create an overlay for the power test
    power_test_overlay = lv_obj_create(parent);
    lv_obj_set_size(power_test_overlay, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(power_test_overlay, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(power_test_overlay, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(power_test_overlay, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(power_test_overlay, 0, LV_PART_MAIN);
    lv_obj_align(power_test_overlay, LV_ALIGN_CENTER, 0, 0);
    lv_obj_remove_flag(power_test_overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(power_test_overlay, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(power_test_overlay, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

    // Container for the labels that can grow
    lv_obj_t *label_container = lv_obj_create(power_test_overlay);
    lv_obj_set_size(label_container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(label_container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(label_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(label_container, 0, LV_PART_MAIN);
    lv_obj_set_flex_grow(label_container, 1);
    lv_obj_set_flex_flow(label_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(label_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // List for the power test labels/values
    lv_obj_t *label_list = lv_list_create(label_container);
    lv_obj_set_size(label_list, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(label_list, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(label_list, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(label_list, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(label_list, 0, LV_PART_MAIN);
    lv_obj_set_style_text_color(label_list, lv_color_hex(GRAY_SHADE_2), LV_PART_MAIN);
    lv_obj_set_flex_grow(label_list, 1);

    // Create a item to show the battery status
    lv_obj_t *battery_item = lv_list_add_button(label_list, LV_SYMBOL_BATTERY_FULL, "Battery");
    lv_obj_set_flex_align(battery_item, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_t *battery_icon = lv_obj_get_child(battery_item, 0);
    lv_obj_set_size(battery_icon, 18, 18);
    lv_obj_set_style_margin_top(battery_icon, 4, LV_PART_MAIN);
    lv_obj_set_style_text_color(battery_icon, lv_color_hex(ORANGE_DIM), LV_PART_MAIN);
    lv_obj_t *battery_label = lv_obj_get_child(battery_item, 1);
    lv_obj_set_style_margin_top(battery_label, -4, LV_PART_MAIN);
    lv_obj_set_style_text_font(battery_label, &clarity_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(battery_label, lv_color_hex(GRAY_SHADE_2), LV_PART_MAIN);
    battery_value_label = lv_label_create(battery_item);
    lv_obj_set_width(battery_value_label, lv_pct(45));
    lv_obj_align(battery_value_label, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_text_align(battery_value_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    lv_obj_set_style_pad_top(battery_value_label, 4, LV_PART_MAIN);
    lv_obj_set_style_text_font(battery_value_label, &bm_mini_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(battery_value_label, lv_color_hex(GRAY_BASE), LV_PART_MAIN);
    lv_label_set_text(battery_value_label, "---");

    // Create a item to show the USB Type-C status
    lv_obj_t *usb_c_item = lv_list_add_button(label_list, LV_SYMBOL_USB, "USB-C");
    lv_obj_set_flex_align(usb_c_item, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_t *usb_c_icon = lv_obj_get_child(usb_c_item, 0);
    lv_obj_set_size(usb_c_icon, 18, 18);
    lv_obj_set_style_margin_top(usb_c_icon, 4, LV_PART_MAIN);
    lv_obj_set_style_text_color(usb_c_icon, lv_color_hex(ORANGE_DIM), LV_PART_MAIN);
    lv_obj_t *usb_c_label = lv_obj_get_child(usb_c_item, 1);
    lv_obj_set_style_margin_top(usb_c_label, -4, LV_PART_MAIN);
    lv_obj_set_style_text_font(usb_c_label, &clarity_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(usb_c_label, lv_color_hex(GRAY_SHADE_2), LV_PART_MAIN);
    lv_label_set_long_mode(usb_c_label, LV_LABEL_LONG_WRAP);
    usb_c_value_label = lv_label_create(usb_c_item);
    lv_obj_set_width(usb_c_value_label, lv_pct(50));
    lv_obj_align(usb_c_value_label, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_text_align(usb_c_value_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    lv_obj_set_style_pad_top(usb_c_value_label, 4, LV_PART_MAIN);
    lv_obj_set_style_text_font(usb_c_value_label, &bm_mini_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(usb_c_value_label, lv_color_hex(GRAY_BASE), LV_PART_MAIN);
    lv_label_set_text(usb_c_value_label, "---");

    // Create a label to show the VBUS status
    lv_obj_t *vbus_item = lv_list_add_button(label_list, LV_SYMBOL_CHARGE, "VBUS");
    lv_obj_set_flex_align(vbus_item, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_t *vbus_icon = lv_obj_get_child(vbus_item, 0);
    lv_obj_set_size(vbus_icon, 10, 18);
    lv_obj_set_style_margin_top(vbus_icon, 4, LV_PART_MAIN);
    lv_obj_set_style_margin_hor(vbus_icon, 4, LV_PART_MAIN);
    lv_obj_set_style_text_color(vbus_icon, lv_color_hex(ORANGE_DIM), LV_PART_MAIN);
    lv_obj_t *vbus_label = lv_obj_get_child(vbus_item, 1);
    lv_obj_set_style_margin_top(vbus_label, -4, LV_PART_MAIN);
    lv_obj_set_style_text_font(vbus_label, &clarity_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(vbus_label, lv_color_hex(GRAY_SHADE_2), LV_PART_MAIN);
    vbus_value_label = lv_label_create(vbus_item);
    lv_obj_set_width(vbus_value_label, lv_pct(50));
    lv_obj_align(vbus_value_label, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_text_align(vbus_value_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    lv_obj_set_style_pad_top(vbus_value_label, 4, LV_PART_MAIN);
    lv_obj_set_style_text_font(vbus_value_label, &bm_mini_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(vbus_value_label, lv_color_hex(GRAY_BASE), LV_PART_MAIN);
    lv_label_set_text(vbus_value_label, "---");

    // Pass/Fail button container
    lv_obj_t *btn_container = lv_obj_create(power_test_overlay);
    lv_obj_set_size(btn_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(btn_container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn_container, 0, LV_PART_MAIN);
    lv_obj_set_flex_grow(btn_container, 0);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);

    // Button style for the bottom pass/fail buttons
    static lv_style_t btn_style;
    lv_style_init(&btn_style);
    lv_style_set_border_width(&btn_style, 2);
    lv_style_set_radius(&btn_style, 1);
    lv_style_set_text_color(&btn_style, lv_color_hex(WHITE));
    lv_style_set_text_font(&btn_style, &cyberphont3b_16);

    // Pass button
    lv_obj_t *pass_btn = lv_button_create(btn_container);
    lv_obj_add_style(pass_btn, &btn_style, LV_PART_MAIN);
    lv_obj_set_style_bg_color(pass_btn, lv_color_hex(GREEN_DIM), LV_PART_MAIN);
    lv_obj_set_style_border_color(pass_btn, lv_color_hex(GREEN_MAIN), LV_PART_MAIN);
    lv_obj_t *pass_label = lv_label_create(pass_btn);
    lv_label_set_text(pass_label, "Pass");
    lv_obj_add_event_cb(pass_btn, power_test_btn_event_cb, LV_EVENT_CLICKED, (void *)HWTEST_BTN_PASS);

    // Fail button
    lv_obj_t *fail_btn = lv_button_create(btn_container);
    lv_obj_add_style(fail_btn, &btn_style, LV_PART_MAIN);
    lv_obj_set_style_bg_color(fail_btn, lv_color_hex(RED_DIM), LV_PART_MAIN);
    lv_obj_set_style_border_color(fail_btn, lv_color_hex(RED_MAIN), LV_PART_MAIN);
    lv_obj_t *fail_label = lv_label_create(fail_btn);
    lv_label_set_text(fail_label, "Fail");
    lv_obj_add_event_cb(fail_btn, power_test_btn_event_cb, LV_EVENT_CLICKED, (void *)HWTEST_BTN_FAIL);

    // Start the power test task
    xTaskCreate(power_test_task, "power_test_task", 4096, NULL, 5, &power_task_handle);
}

static void power_test_btn_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    hwtest_btn_t btn     = (hwtest_btn_t)(intptr_t)lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED) {
        switch (btn) {
            case HWTEST_BTN_PASS: //
                power_test_end(true);
                break;
            case HWTEST_BTN_FAIL: //
                power_test_end(false);
                break;
            default: break;
        }
    }
}

static void power_test_task(void *_arg) {
    while (true) {
        // Get the battery status
        charger_system_status_t charger_status = {0};
        esp_err_t ret                          = charger_read_status(&charger_status);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read charger status");
            continue;
        }

        // Get the USB Type-C status
        typec_status_t type_c_status = {0};
        ret                          = typec_read_status(&type_c_status);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read USB Type-C status");
            continue;
        }

        // Get the battery status
        battery_status_t battery_status = battery_get_status();

        // Format the text for each label
        static char battery_text[64];
        snprintf(battery_text, sizeof(battery_text), "%dmV (%d%%)", battery_status.voltage, battery_status.percentage);

        static char usb_c_text[128];
        snprintf(usb_c_text, sizeof(usb_c_text), "%s\n%s\n%s", type_c_status.vbus_detected ? "VBUS Detected" : "No VBUS",
                 type_c_status.port_status == TYPEC_PORT_DEVICE   ? "Device"
                 : type_c_status.port_status == TYPEC_PORT_HOST   ? "Host"
                 : type_c_status.port_status == TYPEC_PORT_AUDIO  ? "Audio"
                 : type_c_status.port_status == TYPEC_PORT_DEBUG  ? "Debug"
                 : type_c_status.port_status == TYPEC_PORT_ACTIVE ? "Active"
                                                                  : "Standby",
                 type_c_status.plug_polarity == TYPEC_PLUG_CC1   ? "CC1"
                 : type_c_status.plug_polarity == TYPEC_PLUG_CC2 ? "CC2"
                                                                 : "Unknown");

        static char vbus_text[64];
        snprintf(vbus_text, sizeof(vbus_text), "%s",
                 charger_status.vbus_stat == VBUS_STAT_ADAPTER    ? "Adapter"
                 : charger_status.vbus_stat == VBUS_STAT_USB_HOST ? "USB Host"
                 : charger_status.vbus_stat == VBUS_STAT_USB_OTG  ? "OTG"
                                                                  : "Unknown");

        // Update the labels within LVGL lock
        if (lvgl_lock(pdMS_TO_TICKS(100), __FILE__, __LINE__) == pdTRUE) {
            lv_label_set_text(battery_value_label, battery_text);
            lv_label_set_text(usb_c_value_label, usb_c_text);
            lv_label_set_text(vbus_value_label, vbus_text);
            lvgl_unlock(__FILE__, __LINE__);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void power_test_end(bool success) {
    if (success) {
        hwtest_set_result(HWTEST_POWER, HWTEST_RESULT_PASSED);
    } else {
        hwtest_set_result(HWTEST_POWER, HWTEST_RESULT_FAILED);
    }
    if (power_test_overlay != NULL) {
        lv_obj_delete(power_test_overlay);
        power_test_overlay = NULL;
    }
    if (power_task_handle != NULL) {
        vTaskDelete(power_task_handle);
        power_task_handle = NULL;
    }
}

/*****************************************************
 * Touch Test Functions                              *
 *****************************************************/

static void touch_test_end(bool success) {
    if (success) {
        hwtest_set_result(HWTEST_TOUCH, HWTEST_RESULT_PASSED);
    } else {
        hwtest_set_result(HWTEST_TOUCH, HWTEST_RESULT_FAILED);
    }
    if (touch_overlay != NULL) {
        lv_obj_delete(touch_overlay);
        touch_overlay = NULL;
    }
}

static void touch_circle_event_cb(lv_event_t *e) {
    lv_obj_t *circle     = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_PRESSING) {
        lv_point_t p;
        lv_indev_get_point(lv_indev_get_act(), &p);
        int32_t cr = lv_obj_get_width(circle) / 2;

        // Don't allow the circle to be dragged outside the bounds of the overlay
        if (p.x - cr < 0 || p.y - cr < 0 || p.x + cr > LV_HOR_RES || p.y + cr > LV_VER_RES) {
            return;
        }

        // Get the position and radius of the fail button's circle
        lv_area_t fba;
        lv_obj_get_coords(touch_fail_button, &fba);
        int32_t fbr    = lv_area_get_width(&fba) / 2;
        lv_point_t fbc = {
            fba.x1 + lv_area_get_width(&fba) / 2, // X-coordinate of the button center
            fba.y1 + lv_area_get_height(&fba) / 2 // Y-coordinate of the button center
        };

        // Difference in X and Y coordinates
        int32_t fb_dx = p.x - fbc.x;
        int32_t fb_dy = p.y - fbc.y;

        // Distance between the two circles and the sum of their radii
        int32_t dist    = fb_dx * fb_dx + fb_dy * fb_dy;
        int32_t rad_sum = cr + fbr;

        // Check for collision with the fail button (plus a little extra padding)
        if (dist < rad_sum * rad_sum + 2) {
            return;
        }

        // Set the new position of the small circle
        lv_obj_set_pos(circle, p.x - lv_obj_get_width(circle) / 2, p.y - lv_obj_get_height(circle) / 2);
    } else if (code == LV_EVENT_PRESS_LOST || code == LV_EVENT_RELEASED) {
        // Replace the circle with a button object to allow passing the test
        touch_test_circle = lv_button_create(lv_obj_get_parent(circle));
        lv_obj_set_size(touch_test_circle, lv_obj_get_width(circle), lv_obj_get_height(circle));
        lv_obj_set_pos(touch_test_circle, lv_obj_get_x(circle), lv_obj_get_y(circle));
        lv_obj_set_style_bg_color(touch_test_circle, lv_color_hex(GREEN_MAIN), LV_PART_MAIN);
        lv_obj_set_style_border_color(touch_test_circle, lv_color_hex(GREEN_DIMMER), LV_PART_MAIN);
        lv_obj_set_style_border_width(touch_test_circle, 2, LV_PART_MAIN);
        lv_obj_set_style_radius(touch_test_circle, LV_RADIUS_CIRCLE, LV_PART_MAIN);
        lv_obj_t *btn_label = lv_label_create(touch_test_circle);
        lv_label_set_text(btn_label, LV_SYMBOL_OK);
        lv_obj_center(btn_label);
        lv_obj_set_style_text_color(btn_label, lv_color_hex(WHITE), LV_PART_MAIN);
        lv_obj_add_event_cb(touch_test_circle, touch_circle_event_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_delete(circle);
    } else if (code == LV_EVENT_CLICKED) {
        ESP_LOGD(TAG, "Touch test circle clicked");
        touch_test_end(true);
    }
}

static void corner_touch_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *corner     = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_set_style_bg_opa(corner, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_bg_color(corner, lv_color_hex(BLUE_LIGHT), LV_PART_MAIN);
        lv_obj_set_style_border_color(corner, lv_color_hex(BLUE_DIMMER), LV_PART_MAIN);
        lv_obj_t *corner_label = lv_obj_get_child(corner, 0);
        if (corner_label != NULL) {
            lv_obj_set_style_text_color(corner_label, lv_color_hex(WHITE), LV_PART_MAIN);
        }
    }
}

static void touch_fail_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        touch_test_end(false);
    }
}

// Start the touch test
static void hwtest_start_touch_test(lv_obj_t *parent) {
    // Create an overlay for the touch test
    touch_overlay = lv_obj_create(parent);
    lv_obj_set_size(touch_overlay, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_pad_all(touch_overlay, 1, LV_PART_MAIN);
    lv_obj_set_style_border_width(touch_overlay, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(touch_overlay, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(touch_overlay, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(touch_overlay, 1, LV_PART_MAIN);
    lv_obj_align(touch_overlay, LV_ALIGN_CENTER, 0, 0);
    lv_obj_remove_flag(touch_overlay, LV_OBJ_FLAG_SCROLLABLE);

    // Corner touch areas
    lv_align_t corner_aligns[] = {LV_ALIGN_TOP_LEFT, LV_ALIGN_TOP_RIGHT, LV_ALIGN_BOTTOM_LEFT, LV_ALIGN_BOTTOM_RIGHT};
    for (int i = 0; i < 4; i++) {
        lv_obj_t *corner = lv_obj_create(touch_overlay);
        lv_obj_set_size(corner, 50, 50);
        lv_obj_set_style_radius(corner, LV_RADIUS_CIRCLE, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(corner, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(corner, 2, LV_PART_MAIN);
        lv_obj_set_style_border_color(corner, lv_color_hex(GRAY_TINT_5), LV_PART_MAIN);
        lv_obj_align(corner, corner_aligns[i], 0, 0);
        lv_obj_add_flag(corner, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(corner, corner_touch_event_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t *corner_label = lv_label_create(corner);
        lv_label_set_text(corner_label, LV_SYMBOL_PLUS);
        lv_obj_set_style_text_color(corner_label, lv_color_hex(BLACK), LV_PART_MAIN);
        lv_obj_center(corner_label);
    }

    // Create a button to fail the test
    touch_fail_button = lv_button_create(touch_overlay);
    lv_obj_set_size(touch_fail_button, 50, 50);
    lv_obj_set_style_bg_color(touch_fail_button, lv_color_hex(RED_MAIN), LV_PART_MAIN);
    lv_obj_set_style_border_color(touch_fail_button, lv_color_hex(RED_DIMMER), LV_PART_MAIN);
    lv_obj_set_style_border_width(touch_fail_button, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(touch_fail_button, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_align(touch_fail_button, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t *fail_label = lv_label_create(touch_fail_button);
    lv_label_set_text(fail_label, LV_SYMBOL_CLOSE);
    lv_obj_center(fail_label);
    lv_obj_add_event_cb(touch_fail_button, touch_fail_event_cb, LV_EVENT_CLICKED, NULL);

    // Create a circle that can be dragged around the large circle
    touch_test_circle = lv_obj_create(touch_overlay);
    lv_obj_set_size(touch_test_circle, 50, 50);
    lv_obj_set_style_bg_color(touch_test_circle, lv_color_hex(GRAY_TINT_3), LV_PART_MAIN);
    lv_obj_set_style_border_width(touch_test_circle, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(touch_test_circle, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_pos(touch_test_circle, LV_HOR_RES / 2, 30);
    lv_obj_add_flag(touch_test_circle, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(touch_test_circle, touch_circle_event_cb, LV_EVENT_ALL, NULL);
}

/*****************************************************
 * Screw Power (Load Switch) Test Functions          *
 *****************************************************/

static void hwtest_start_screw_power_test(lv_obj_t *parent) {
    // Create an overlay for the screw power test
    screw_power_overlay = lv_obj_create(parent);
    lv_obj_set_size(screw_power_overlay, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(screw_power_overlay, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screw_power_overlay, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(screw_power_overlay, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(screw_power_overlay, 0, LV_PART_MAIN);
    lv_obj_align(screw_power_overlay, LV_ALIGN_CENTER, 0, 0);
    lv_obj_remove_flag(screw_power_overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(screw_power_overlay, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(screw_power_overlay, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

    // Container for the labels that can grow
    lv_obj_t *label_container = lv_obj_create(screw_power_overlay);
    lv_obj_set_size(label_container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(label_container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(label_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(label_container, 0, LV_PART_MAIN);
    lv_obj_set_flex_grow(label_container, 1);
    lv_obj_set_flex_flow(label_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(label_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

    // List for the screw power test labels/values
    lv_obj_t *label_list = lv_list_create(label_container);
    lv_obj_set_size(label_list, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(label_list, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(label_list, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(label_list, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(label_list, 0, LV_PART_MAIN);
    lv_obj_set_style_text_color(label_list, lv_color_hex(GRAY_SHADE_2), LV_PART_MAIN);
    lv_obj_set_flex_grow(label_list, 1);

    // Create an item to show the load switch enable status (on/off)
    lv_obj_t *load_switch_item = lv_list_add_button(label_list, LV_SYMBOL_POWER, "Load Switch");
    lv_obj_set_flex_align(load_switch_item, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_t *load_switch_icon = lv_obj_get_child(load_switch_item, 0);
    lv_obj_set_size(load_switch_icon, 18, 18);
    lv_obj_set_style_margin_top(load_switch_icon, 4, LV_PART_MAIN);
    lv_obj_set_style_text_color(load_switch_icon, lv_color_hex(ORANGE_DIM), LV_PART_MAIN);
    lv_obj_t *load_switch_label = lv_obj_get_child(load_switch_item, 1);
    lv_obj_set_style_margin_top(load_switch_label, -4, LV_PART_MAIN);
    lv_obj_set_style_text_font(load_switch_label, &clarity_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(load_switch_label, lv_color_hex(GRAY_SHADE_2), LV_PART_MAIN);
    load_switch_enable_state_label = lv_label_create(load_switch_item);
    lv_obj_set_width(load_switch_enable_state_label, lv_pct(35));
    lv_obj_align(load_switch_enable_state_label, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_text_align(load_switch_enable_state_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    lv_obj_set_style_pad_top(load_switch_enable_state_label, 4, LV_PART_MAIN);
    lv_obj_set_style_text_font(load_switch_enable_state_label, &bm_mini_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(load_switch_enable_state_label, lv_color_hex(GRAY_BASE), LV_PART_MAIN);
    lv_label_set_text(load_switch_enable_state_label, "---");

    // Create an item to show the load switch flag (short-circuit/interrupt) status
    lv_obj_t *load_switch_flag_item = lv_list_add_button(label_list, LV_SYMBOL_WARNING, "Flag");
    lv_obj_set_flex_align(load_switch_flag_item, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_t *load_switch_flag_icon = lv_obj_get_child(load_switch_flag_item, 0);
    lv_obj_set_size(load_switch_flag_icon, 18, 18);
    lv_obj_set_style_margin_top(load_switch_flag_icon, 4, LV_PART_MAIN);
    lv_obj_set_style_text_color(load_switch_flag_icon, lv_color_hex(ORANGE_DIM), LV_PART_MAIN);
    lv_obj_t *load_switch_flag_label = lv_obj_get_child(load_switch_flag_item, 1);
    lv_obj_set_style_margin_top(load_switch_flag_label, -4, LV_PART_MAIN);
    lv_obj_set_style_text_font(load_switch_flag_label, &clarity_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(load_switch_flag_label, lv_color_hex(GRAY_SHADE_2), LV_PART_MAIN);
    load_switch_flag_state_label = lv_label_create(load_switch_flag_item);
    lv_obj_set_width(load_switch_flag_state_label, lv_pct(35));
    lv_obj_align(load_switch_flag_state_label, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_text_align(load_switch_flag_state_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    lv_obj_set_style_pad_top(load_switch_flag_state_label, 4, LV_PART_MAIN);
    lv_obj_set_style_text_font(load_switch_flag_state_label, &bm_mini_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(load_switch_flag_state_label, lv_color_hex(GRAY_BASE), LV_PART_MAIN);
    lv_label_set_text(load_switch_flag_state_label, "---");

    // Continue button container
    lv_obj_t *btn_container = lv_obj_create(screw_power_overlay);
    lv_obj_set_size(btn_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(btn_container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn_container, 0, LV_PART_MAIN);
    lv_obj_set_flex_grow(btn_container, 0);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);

    // Continue button
    lv_obj_t *continue_btn = lv_button_create(btn_container);
    lv_obj_add_style(continue_btn, &button_style, LV_PART_MAIN);
    lv_obj_add_style(continue_btn, &pass_light, LV_PART_MAIN);
    lv_obj_t *continue_label = lv_label_create(continue_btn);
    lv_label_set_text(continue_label, "Continue");
    lv_obj_add_event_cb(continue_btn, load_switch_test_btn_event_cb, LV_EVENT_CLICKED, (void *)HWTEST_BTN_PASS);

    // Start the load switch test task
    xTaskCreate(load_switch_test_task, "screw_power_test_task", 4096, NULL, 5, &load_switch_task_handle);
}

static void load_switch_test_btn_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    hwtest_btn_t btn     = (hwtest_btn_t)(intptr_t)lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED) {
        switch (btn) {
            case HWTEST_BTN_PASS: //
                load_switch_test_end(true);
                break;
            case HWTEST_BTN_FAIL: //
                load_switch_test_end(false);
                break;
            default: break;
        }
    }
}

static void load_switch_test_task(void *_arg) {
    while (true) {
        // Check to see if the load switch interrupt is set in the I2C mux control register
        uint8_t mux_control = 0;
        esp_err_t ret       = i2c_switch_get_status(&mux_control);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read I2C mux control register");
            continue;
        }

        // Get the current load switch status
        bool enabled = load_switch_enabled();
        bool flag    = ((mux_control >> 4) & LOAD_SWITCH_INTERRUPT) ? true : false;

        // Update the load switch status labels
        if (lvgl_lock(pdMS_TO_TICKS(100), __FILE__, __LINE__) == pdTRUE) {
            if (load_switch_enable_state_label == NULL || load_switch_flag_state_label == NULL) {
                lvgl_unlock(__FILE__, __LINE__);
                continue;
            }
            lv_label_set_text(load_switch_enable_state_label, enabled ? "Enabled" : "Disabled");
            lv_label_set_text(load_switch_flag_state_label, flag ? "Short" : "Open");
            lvgl_unlock(__FILE__, __LINE__);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void load_switch_test_end(bool success) {
    if (success) {
        hwtest_set_result(HWTEST_SCREW, HWTEST_RESULT_PASSED);
    } else {
        hwtest_set_result(HWTEST_SCREW, HWTEST_RESULT_FAILED);
    }
    if (screw_power_overlay != NULL) {
        lv_obj_delete(screw_power_overlay);
        screw_power_overlay = NULL;
    }
    if (load_switch_task_handle != NULL) {
        vTaskDelete(load_switch_task_handle);
        load_switch_task_handle = NULL;
    }
}

/*****************************************************
 * IR Test Functions                                 *
 *****************************************************/

static void hwtest_start_ir_test(lv_obj_t *parent) {
    // Create an overlay for the IR test
    ir_overlay = lv_obj_create(parent);
    lv_obj_set_size(ir_overlay, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(ir_overlay, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ir_overlay, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(ir_overlay, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(ir_overlay, 0, LV_PART_MAIN);
    lv_obj_align(ir_overlay, LV_ALIGN_CENTER, 0, 0);
    lv_obj_remove_flag(ir_overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(ir_overlay, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ir_overlay, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

    // Container for the labels that can grow
    lv_obj_t *label_container = lv_obj_create(ir_overlay);
    lv_obj_set_size(label_container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(label_container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(label_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(label_container, 0, LV_PART_MAIN);
    lv_obj_set_flex_grow(label_container, 1);
    lv_obj_set_flex_flow(label_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(label_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

    // List for the IR test labels/values
    lv_obj_t *label_list = lv_list_create(label_container);
    lv_obj_set_size(label_list, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(label_list, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(label_list, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(label_list, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(label_list, 0, LV_PART_MAIN);
    lv_obj_set_style_text_color(label_list, lv_color_hex(GRAY_SHADE_2), LV_PART_MAIN);
    lv_obj_set_flex_grow(label_list, 1);

    // Create an item to show the IR Tx status
    lv_obj_t *ir_tx_item = lv_list_add_button(label_list, LV_SYMBOL_UPLOAD, "IR Tx Code");
    lv_obj_set_flex_align(ir_tx_item, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_t *ir_tx_icon = lv_obj_get_child(ir_tx_item, 0);
    lv_obj_set_size(ir_tx_icon, 18, 18);
    lv_obj_set_style_margin_top(ir_tx_icon, 4, LV_PART_MAIN);
    lv_obj_set_style_text_color(ir_tx_icon, lv_color_hex(ORANGE_DIM), LV_PART_MAIN);
    lv_obj_t *ir_tx_label = lv_obj_get_child(ir_tx_item, 1);
    lv_obj_set_style_margin_top(ir_tx_label, -4, LV_PART_MAIN);
    lv_obj_set_style_text_font(ir_tx_label, &clarity_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(ir_tx_label, lv_color_hex(GRAY_SHADE_2), LV_PART_MAIN);
    ir_tx_value_label = lv_label_create(ir_tx_item);
    lv_obj_set_width(ir_tx_value_label, lv_pct(35));
    lv_obj_align(ir_tx_value_label, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_text_align(ir_tx_value_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    lv_obj_set_style_pad_top(ir_tx_value_label, 4, LV_PART_MAIN);
    lv_obj_set_style_text_font(ir_tx_value_label, &bm_mini_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(ir_tx_value_label, lv_color_hex(GRAY_BASE), LV_PART_MAIN);
    lv_label_set_text(ir_tx_value_label, "---");

    // Create an item to show the IR Rx status
    lv_obj_t *ir_rx_item = lv_list_add_button(label_list, LV_SYMBOL_DOWNLOAD, "IR Rx Code");
    lv_obj_set_flex_align(ir_rx_item, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_t *ir_rx_icon = lv_obj_get_child(ir_rx_item, 0);
    lv_obj_set_size(ir_rx_icon, 18, 18);
    lv_obj_set_style_margin_top(ir_rx_icon, 4, LV_PART_MAIN);
    lv_obj_set_style_text_color(ir_rx_icon, lv_color_hex(ORANGE_DIM), LV_PART_MAIN);
    lv_obj_t *ir_rx_label = lv_obj_get_child(ir_rx_item, 1);
    lv_obj_set_style_margin_top(ir_rx_label, -4, LV_PART_MAIN);
    lv_obj_set_style_text_font(ir_rx_label, &clarity_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(ir_rx_label, lv_color_hex(GRAY_SHADE_2), LV_PART_MAIN);
    ir_rx_value_label = lv_label_create(ir_rx_item);
    lv_obj_set_width(ir_rx_value_label, lv_pct(35));
    lv_obj_align(ir_rx_value_label, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_text_align(ir_rx_value_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    lv_obj_set_style_pad_top(ir_rx_value_label, 4, LV_PART_MAIN);
    lv_obj_set_style_text_font(ir_rx_value_label, &bm_mini_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(ir_rx_value_label, lv_color_hex(GRAY_BASE), LV_PART_MAIN);
    lv_label_set_text(ir_rx_value_label, "---");

    // Continue button container
    lv_obj_t *btn_container = lv_obj_create(ir_overlay);
    lv_obj_set_size(btn_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(btn_container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn_container, 0, LV_PART_MAIN);
    lv_obj_set_flex_grow(btn_container, 0);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);

    // Continue button
    lv_obj_t *continue_btn = lv_button_create(btn_container);
    lv_obj_add_style(continue_btn, &button_style, LV_PART_MAIN);
    lv_obj_add_style(continue_btn, &pass_light, LV_PART_MAIN);
    lv_obj_t *continue_label = lv_label_create(continue_btn);
    lv_label_set_text(continue_label, "Continue");
    lv_obj_add_event_cb(continue_btn, ir_test_btn_event_cb, LV_EVENT_CLICKED, (void *)HWTEST_BTN_PASS);

    // Disable the IR Rx buffer during the test
    badge_ir_enable_rx_buffer(false);

    // Register the IR Rx callback
    if (badge_ir_add_rx_callback(ir_test_rx_callback) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add IR Rx callback");
    } else {
        ESP_LOGD(TAG, "IR Rx callback registered");
    }

    // Start the timer for the IR test
    ir_test_timer = lv_timer_create(ir_test_run_next, 700, NULL);
    lv_timer_set_repeat_count(ir_test_timer, IR_TEST_COUNT);
    lv_timer_ready(ir_test_timer);
}

static void ir_test_btn_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    hwtest_btn_t btn     = (hwtest_btn_t)(intptr_t)lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED) {
        switch (btn) {
            case HWTEST_BTN_PASS: //
                ir_test_end(true);
                break;
            case HWTEST_BTN_FAIL: //
                ir_test_end(false);
                break;
            default: break;
        }
    }
}

// Run the next IR test - iterates through the IR codes in the test array
static void ir_test_run_next(lv_timer_t *timer) {
    // Get the next IR code from the test array
    ir_code_base_t ir_code = ir_test_codes[ir_test_current];
    ir_test_current++;

    // Transmit the IR code
    ir_transmit(ir_code.address, ir_code.command);

    // Update the IR Tx label
    char ir_tx_text[16];
    snprintf(ir_tx_text, sizeof(ir_tx_text), "%04X %04X", ir_code.address, ir_code.command);
    lv_label_set_text(ir_tx_value_label, ir_tx_text);

    if (ir_test_current >= IR_TEST_COUNT) {
        ir_test_end(true);
    }
}

static void ir_test_rx_callback(uint16_t address, uint16_t command) {
    // Add the received IR code to the test results array
    ir_code_base_t ir_code                      = {address, command};
    ir_test_codes_received[ir_test_current - 1] = ir_code;
    if (ir_rx_value_label == NULL) {
        return;
    }

    // Update the IR Rx label
    char ir_rx_text[16];
    snprintf(ir_rx_text, sizeof(ir_rx_text), "%04X %04X", address, command);
    lv_label_set_text(ir_rx_value_label, ir_rx_text);
}

static void ir_test_end(bool success) {
    if (success) {
        hwtest_set_result(HWTEST_IR, HWTEST_RESULT_PASSED);
    } else {
        hwtest_set_result(HWTEST_IR, HWTEST_RESULT_FAILED);
    }
    ir_test_current = 0;
    if (badge_ir_remove_rx_callback(ir_test_rx_callback) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to remove IR Rx callback");
    }
    if (ir_test_timer != NULL) {
        lv_timer_delete(ir_test_timer);
        ir_test_timer = NULL;
    }
    vTaskDelay(pdMS_TO_TICKS(300));
    badge_ir_enable_rx_buffer(true);
    if (ir_overlay != NULL) {
        lv_obj_delete(ir_overlay);
        ir_overlay = NULL;
    }
}

/*****************************************************
 * Minibadge Test Functions                          *
 *****************************************************/

static void hwtest_start_minibadge_test(lv_obj_t *parent) {
    // Create an overlay for the minibadge test
    minibadge_overlay = lv_obj_create(parent);
    lv_obj_set_size(minibadge_overlay, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(minibadge_overlay, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(minibadge_overlay, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(minibadge_overlay, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(minibadge_overlay, 0, LV_PART_MAIN);
    lv_obj_align(minibadge_overlay, LV_ALIGN_CENTER, 0, 0);
    lv_obj_remove_flag(minibadge_overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(minibadge_overlay, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(minibadge_overlay, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

    // List for the minibadge test labels/values
    minibadge_list = lv_list_create(minibadge_overlay);
    lv_obj_set_size(minibadge_list, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(minibadge_list, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(minibadge_list, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(minibadge_list, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(minibadge_list, 0, LV_PART_MAIN);
    lv_obj_set_flex_grow(minibadge_list, 1);

    // Create a button container
    lv_obj_t *btn_container = lv_obj_create(minibadge_overlay);
    lv_obj_set_size(btn_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(btn_container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn_container, 0, LV_PART_MAIN);
    lv_obj_set_flex_grow(btn_container, 0);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);

    // Continue button
    lv_obj_t *continue_btn = lv_button_create(btn_container);
    lv_obj_add_style(continue_btn, &button_style, LV_PART_MAIN);
    lv_obj_add_style(continue_btn, &pass_light, LV_PART_MAIN);
    lv_obj_t *continue_label = lv_label_create(continue_btn);
    lv_label_set_text(continue_label, "Continue");
    lv_obj_add_event_cb(continue_btn, minibadge_test_btn_event_cb, LV_EVENT_CLICKED, (void *)HWTEST_BTN_PASS);

    // Start the minibadge scan timer
    minibadge_timer = lv_timer_create(minibadge_test_scan, 1000, NULL);
    lv_timer_ready(minibadge_timer);
}

static void minibadge_test_btn_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    hwtest_btn_t btn     = (hwtest_btn_t)(intptr_t)lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED) {
        switch (btn) {
            case HWTEST_BTN_PASS: //
                minibadge_test_end(true);
                break;
            case HWTEST_BTN_FAIL: //
                minibadge_test_end(false);
                break;
            default: break;
        }
    }
}

static void minibadge_test_scan(lv_timer_t *timer) {
    // Clean the list
    lv_obj_clean(minibadge_list);

    // Add the minibadge devices to the list
    for (int i = 0; i < sizeof(minibadge_devices) / sizeof(minibadge_devices[0]); i++) {
        // Read the minibadge name from the EEPROM
        char mb_label[32];
        snprintf(mb_label, sizeof(mb_label), "Slot %d", minibadge_devices[i].channel + 1);
        char mb_name[8 + 1] = {0};
        esp_err_t ret       = i2c_manager_read_eeprom(&minibadge_devices[i], 0x00, (uint8_t *)mb_name, sizeof(mb_name) - 1);
        if (ret != ESP_OK) {
            snprintf(mb_name, sizeof(mb_name), "---");
        }

        lv_obj_t *minibadge_item = lv_list_add_button(minibadge_list, LV_SYMBOL_FILE, mb_label);
        lv_obj_t *minibadge_icon = lv_obj_get_child(minibadge_item, 0);
        lv_obj_set_size(minibadge_icon, 18, 18);
        lv_obj_set_style_margin_top(minibadge_icon, 4, LV_PART_MAIN);
        lv_obj_set_style_text_color(minibadge_icon, lv_color_hex(ORANGE_DIM), LV_PART_MAIN);
        lv_obj_t *minibadge_label = lv_obj_get_child(minibadge_item, 1);
        lv_obj_set_style_margin_top(minibadge_label, -4, LV_PART_MAIN);
        lv_obj_set_style_text_font(minibadge_label, &clarity_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(minibadge_label, lv_color_hex(GRAY_SHADE_2), LV_PART_MAIN);
        lv_obj_t *minibadge_name = lv_label_create(minibadge_item);
        lv_obj_set_width(minibadge_name, lv_pct(45));
        lv_label_set_text(minibadge_name, mb_name);
        lv_obj_align(minibadge_name, LV_ALIGN_RIGHT_MID, 0, 0);
        lv_obj_set_style_text_align(minibadge_name, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
        lv_obj_set_style_pad_top(minibadge_name, 4, LV_PART_MAIN);
        lv_obj_set_style_text_font(minibadge_name, &bm_mini_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(minibadge_name, lv_color_hex(GRAY_BASE), LV_PART_MAIN);
    }
}

static void minibadge_test_end(bool success) {
    if (success) {
        hwtest_set_result(HWTEST_MINIBADGE, HWTEST_RESULT_PASSED);
    } else {
        hwtest_set_result(HWTEST_MINIBADGE, HWTEST_RESULT_FAILED);
    }
    if (minibadge_timer != NULL) {
        lv_timer_delete(minibadge_timer);
        minibadge_timer = NULL;
    }
    if (minibadge_overlay != NULL) {
        lv_obj_delete(minibadge_overlay);
        minibadge_overlay = NULL;
    }
}
