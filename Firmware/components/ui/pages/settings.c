#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_debug_helpers.h"
#include "esp_wifi.h"

#include "api.h"
#include "badge.h"
#include "components.h"
#include "display.h"
#include "loadanim.h"
#include "settings.h"
#include "theme.h"
#include "ui.h"
#include "wifi_manager.h"

static const char *TAG = "pages/settings";

LV_IMAGE_DECLARE(left_hand);
LV_IMAGE_DECLARE(right_hand);

#define MAX_SSID_COUNT  15
#define MAX_SSID_LENGTH 33

static lv_obj_t *settings_menu  = NULL;
static lv_obj_t *username_value = NULL;
static lv_obj_t *saved_networks = NULL;
static wifi_credentials_t saved_networks_list[CONFIG_EXTERNAL_WIFI_MAX_NETWORKS];
static char scanned_ssid_list[MAX_SSID_COUNT][MAX_SSID_LENGTH];
static esp_event_handler_instance_t wifi_scan_event_handler_instance = NULL;
static lv_obj_t *wrist_orientation_modal                             = NULL;

// Forward declarations
static void close_modal_event_cb(lv_event_t *e);
static void username_input_cb(lv_event_code_t event, const char *username, void *user_data);
static void wifi_networks_list_update();
static void wifi_networks_edit(lv_obj_t *parent);
static void wifi_scan_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void wifi_password_input_cb(lv_event_code_t event, const char *password, void *user_data);
static void wifi_networks_scan(lv_obj_t *parent);
static void wifi_networks_show(lv_obj_t *parent);
static void wrist_orientation_edit(lv_obj_t *parent);
static void screen_brightness_event_cb(lv_event_t *e);
static void screen_brightness_edit(lv_obj_t *parent);

// Callback for the close button on the modal overlay
static void close_modal_event_cb(lv_event_t *e) {
    lv_obj_t *modal = lv_event_get_user_data(e);
    lv_obj_delete(modal);
}

// Helper function to create a modal overlay
static lv_obj_t *create_modal(lv_obj_t *parent, const char *title) {
    lv_obj_t *modal = lv_obj_create(parent);
    lv_obj_set_size(modal, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(modal, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(modal, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_border_width(modal, 0, LV_PART_MAIN);
    lv_obj_align(modal, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *box = lv_obj_create(modal);
    lv_obj_set_size(box, 220, 180);
    lv_obj_align(box, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_border_width(box, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(box, lv_color_hex(ORANGE_DIM), LV_PART_MAIN);
    lv_obj_set_style_radius(box, 3, LV_PART_MAIN);

    lv_obj_t *title_label = lv_label_create(box);
    lv_label_set_text(title_label, title);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 10);

    return box;
}

// Callback for the username edit button
static void username_edit_event_cb(lv_event_t *e) {
    input_prompt(badge_config.handle, "Enter new username", false, username_input_cb, NULL);
}

// Callback for the WiFi networks button
static void wifi_networks_event_cb(lv_event_t *e) {
    lv_obj_t *parent = lv_event_get_user_data(e);
    wifi_networks_edit(parent);
}

// Callback for the wrist orientation button
static void wrist_orientation_event_cb(lv_event_t *e) {
    lv_obj_t *parent = lv_event_get_user_data(e);
    wrist_orientation_edit(parent);
}

// Callback for the username input
static void username_input_cb(lv_event_code_t event, const char *username, void *user_data) {
    if (event == LV_EVENT_READY) {
        if (set_badge_handle(username) == ESP_OK) {
            ESP_LOGI(TAG, "Saved new handle: %s", username);
            lv_label_set_text(username_value, username);

            // Update with the register API
            api_result_t *result = api_register(username);
            if (result == NULL) {
                ESP_LOGE(TAG, "Failed to register new handle: %s", username);
            } else {
                if (result->status != true) {
                    ESP_LOGE(TAG, "Failed to register new handle: %s", username);
                } else {
                    badge_config.registered = true;
                    save_badge_config();
                }
                api_free_result(result, true);
            }
        } else {
            ESP_LOGE(TAG, "Failed to save new handle: %s", esp_err_to_name(ESP_FAIL));
        }
    }
}

typedef enum {
    BTN_NETWORK_DELETE,
    BTN_NETWORK_SCAN,
    BTN_NETWORK_ADD,
    BTN_NETWORK_CLOSE,
} network_btn_t;

lv_obj_t *net_scan_container = NULL;
lv_obj_t *net_show_container = NULL;

// Callback for scanning and adding a new network
static void network_modal_event_cb(lv_event_t *e) {
    network_btn_t btn_type = (network_btn_t)lv_event_get_user_data(e);
    lv_obj_t *btn          = lv_event_get_target(e);
    lv_obj_t *overlay      = lv_obj_get_parent(lv_obj_get_parent(lv_obj_get_parent(btn)));

    switch (btn_type) {
        case BTN_NETWORK_DELETE: {
            const char *ssid = (const char *)lv_obj_get_user_data(btn);
            ESP_LOGI(TAG, "Deleting network: %s", ssid);
            lv_obj_delete(lv_obj_get_parent(btn));
            esp_err_t ret = delete_wifi_network((char *)ssid);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to delete WiFi network: %s", esp_err_to_name(ret));
            }
            wifi_networks_list_update();
            break;
        }
        case BTN_NETWORK_ADD: {
            const char *ssid = (const char *)lv_obj_get_user_data(btn);
            ESP_LOGI(TAG, "Adding network: %s", ssid);
            lv_obj_delete(net_show_container);
            if (net_scan_container != NULL) {
                lv_obj_delete(net_scan_container);
            }
            char prompt_text[64];
            snprintf(prompt_text, sizeof(prompt_text), "Password for %s", ssid);
            input_prompt("", prompt_text, true, wifi_password_input_cb, (void *)ssid);
            break;
        }
        case BTN_NETWORK_SCAN: {
            wifi_networks_scan(overlay);
            break;
        }
        case BTN_NETWORK_CLOSE: {
            if (overlay != NULL) {
                lv_obj_delete(overlay);
            }
            break;
        }
        default: break;
    }
}

static void wifi_networks_list_update() {
    lv_obj_clean(saved_networks);

    uint8_t wifi_count = 0;
    get_saved_wifi_networks(saved_networks_list, &wifi_count);

    for (uint8_t i = 0; i < wifi_count; i++) {
        lv_obj_t *btn       = lv_list_add_button(saved_networks, LV_SYMBOL_WIFI, saved_networks_list[i].ssid);
        lv_obj_t *btn_label = lv_obj_get_child(btn, 1);
        lv_obj_set_style_text_font(btn_label, &bm_mini_16, LV_PART_MAIN);
        lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

        lv_obj_t *del_btn = lv_button_create(btn);
        lv_obj_t *del_img = lv_image_create(del_btn);
        lv_image_set_src(del_img, LV_SYMBOL_TRASH);
        lv_obj_add_style(del_btn, &button_style, LV_PART_MAIN);
        lv_obj_add_style(del_btn, &button_orange, LV_PART_MAIN);
        lv_obj_set_style_text_font(del_btn, &lv_font_montserrat_12, LV_PART_MAIN);
        lv_obj_align(del_btn, LV_ALIGN_RIGHT_MID, 0, 0);
        lv_obj_set_user_data(del_btn, (void *)saved_networks_list[i].ssid);
        lv_obj_add_event_cb(del_btn, network_modal_event_cb, LV_EVENT_CLICKED, BTN_NETWORK_DELETE);
    }
}

// WiFi networks management popup
static void wifi_networks_edit(lv_obj_t *parent) {
    lv_obj_t *overlay = lv_obj_create(lv_screen_active());
    lv_obj_set_size(overlay, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(overlay, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(overlay, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_border_width(overlay, 0, LV_PART_MAIN);
    lv_obj_align(overlay, LV_ALIGN_CENTER, 0, 0);
    lv_obj_remove_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *inner_container = lv_obj_create(overlay);
    lv_obj_set_size(inner_container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(inner_container, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_style_border_width(inner_container, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(inner_container, lv_color_hex(ORANGE_DIM), LV_PART_MAIN);
    lv_obj_set_style_radius(inner_container, 3, LV_PART_MAIN);
    lv_obj_align(inner_container, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_flex_flow(inner_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(inner_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

    saved_networks = lv_list_create(inner_container);
    lv_obj_set_size(saved_networks, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(saved_networks, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(saved_networks, 0, LV_PART_MAIN);
    lv_obj_set_flex_grow(saved_networks, 1);

    // Show saved networks
    wifi_networks_list_update();

    // Flex container for bottom buttons
    lv_obj_t *bottom_btns = lv_obj_create(inner_container);
    lv_obj_set_size(bottom_btns, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(bottom_btns, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bottom_btns, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(bottom_btns, 0, LV_PART_MAIN);
    lv_obj_align(bottom_btns, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(bottom_btns, LV_FLEX_FLOW_ROW);

    // Button to scan for a new network
    lv_obj_t *scan_btn = lv_button_create(bottom_btns);
    lv_obj_add_style(scan_btn, &button_style, LV_PART_MAIN);
    lv_obj_add_style(scan_btn, &button_orange, LV_PART_MAIN);
    lv_obj_t *scan_label = lv_label_create(scan_btn);
    lv_label_set_text(scan_label, "Scan");
    lv_obj_center(scan_label);
    lv_obj_add_event_cb(scan_btn, network_modal_event_cb, LV_EVENT_CLICKED, BTN_NETWORK_SCAN);

    // Button to close the overlay
    lv_obj_t *close_btn = lv_button_create(bottom_btns);
    lv_obj_add_style(close_btn, &button_style, LV_PART_MAIN);
    lv_obj_add_style(close_btn, &button_orange, LV_PART_MAIN);
    lv_obj_t *close_label = lv_label_create(close_btn);
    lv_label_set_text(close_label, "Close");
    lv_obj_center(close_label);
    lv_obj_add_event_cb(close_btn, network_modal_event_cb, LV_EVENT_CLICKED, BTN_NETWORK_CLOSE);
}

// Event handler to get WIFI_EVENT_SCAN_DONE and show the networks found
static void wifi_scan_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base != WIFI_EVENT || event_id != WIFI_EVENT_SCAN_DONE) {
        return;
    }
    ESP_LOGD(TAG, "WiFi scan done event received");
    // Unregister the event handler
    esp_err_t err = esp_event_handler_instance_unregister(WIFI_EVENT, WIFI_EVENT_SCAN_DONE, wifi_scan_event_handler_instance);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to unregister WiFi scan done event handler: %s", esp_err_to_name(err));
    }
    wifi_scan_event_handler_instance = NULL;

    // Show the networks found during the scan
    lv_obj_t *overlay = (lv_obj_t *)arg;
    lv_async_call(wifi_networks_show, overlay);
}

// Callback for the password input
static void wifi_password_input_cb(lv_event_code_t event, const char *password, void *user_data) {
    const char *ssid = (const char *)user_data;
    ESP_LOGD(TAG, "Adding network: %s, password: %s", ssid, password);
    wifi_credentials_t credentials;
    strncpy(credentials.ssid, ssid, sizeof(credentials.ssid));
    strncpy(credentials.password, password, sizeof(credentials.password));
    esp_err_t ret = save_wifi_network(&credentials);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save WiFi network: %s", esp_err_to_name(ret));
    } else {
        wifi_networks_list_update();
    }
}

static void msgbox_event_cb(lv_event_t *e) {
    lv_obj_t *msgbox = lv_obj_get_parent(lv_event_get_target(e));
    lv_msgbox_close(msgbox);
}

// Add network popup - scan for networks and let the user choose one to add
static void wifi_networks_scan(lv_obj_t *overlay) {
    net_scan_container = lv_obj_create(overlay);
    lv_obj_set_size(net_scan_container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(net_scan_container, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_style_border_width(net_scan_container, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(net_scan_container, lv_color_hex(ORANGE_DIM), LV_PART_MAIN);
    lv_obj_set_style_radius(net_scan_container, 3, LV_PART_MAIN);
    lv_obj_align(net_scan_container, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_flex_flow(net_scan_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(net_scan_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Make a label and loading animation while scanning
    lv_obj_t *scan_label = lv_label_create(net_scan_container);
    lv_label_set_text(scan_label, "Scanning...");
    lv_obj_set_style_text_font(scan_label, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_align(scan_label, LV_ALIGN_CENTER, 0, 0);

    loading_dots_anim_config_t dots_config = {
        .parent            = net_scan_container,
        .align_to          = scan_label,
        .x_offset          = 0,
        .y_offset          = 20,
        .dot_color         = lv_color_hex(0x000000),
        .dot_size          = 10,
        .fade_in_duration  = 250,
        .fade_out_duration = 250,
        .sequence_delay    = 200,
        .repeat_delay      = 500,
    };
    loading_dots_anim(&dots_config);

    // Set up an event handler to show scan results on WIFI_EVENT_SCAN_DONE
    if (wifi_scan_event_handler_instance == NULL) {
        esp_err_t err = esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_SCAN_DONE, wifi_scan_event_handler, overlay,
                                                            &wifi_scan_event_handler_instance);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to register WiFi scan done event handler: %s", esp_err_to_name(err));
            wifi_scan_event_handler_instance = NULL;
        }
    }

    // Start a non-blocking scan for networks
    wifi_scan_config_t scan_config = {
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time.active =
            {
                .min = 100,
                .max = 500,
            },
    };
    esp_err_t ret = esp_wifi_scan_start(&scan_config, false);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start WiFi scan: %s", esp_err_to_name(ret));

        // Clean up the scanning animation container
        lv_obj_delete(net_scan_container);
        net_scan_container = NULL;
        lv_refr_now(NULL);

        // Show an error message to the user
        lv_obj_t *error_msg = lv_msgbox_create(NULL);
        char error_msg_text[64];
        snprintf(error_msg_text, sizeof(error_msg_text), "Failed to scan for networks: %s", esp_err_to_name(ret));
        lv_msgbox_add_text(error_msg, error_msg_text);
        lv_obj_t *close_btn = lv_msgbox_add_footer_button(error_msg, "OK");
        style_msgbox(error_msg);
        lv_obj_add_event_cb(close_btn, msgbox_event_cb, LV_EVENT_CLICKED, NULL);
    }
}

// Timer callback to get the scan results and show them
static void wifi_networks_show(lv_obj_t *overlay) {
    uint16_t ap_count = 0;
    esp_err_t ret     = esp_wifi_scan_get_ap_num(&ap_count);
    if (ret != ESP_OK || ap_count == 0) {
        ESP_LOGE(TAG, "Failed to get WiFi AP count or no APs found: err=%s, ap_count=%d", esp_err_to_name(ret), ap_count);

        // Clean up the scanning animation container
        lv_obj_delete(net_scan_container);
        net_scan_container = NULL;
        lv_refr_now(NULL);

        // Show an error message to the user
        lv_obj_t *error_msg = lv_msgbox_create(NULL);
        lv_msgbox_add_text(error_msg, "Couldn't find any networks");
        lv_obj_t *close_btn = lv_msgbox_add_footer_button(error_msg, "OK");
        style_msgbox(error_msg);
        lv_obj_add_event_cb(close_btn, msgbox_event_cb, LV_EVENT_CLICKED, NULL);
        return;
    }

    // Fetch the list of networks and show them
    wifi_ap_record_t *ap_list = malloc(sizeof(wifi_ap_record_t) * ap_count);
    if (ap_list == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for AP scan list");
        return;
    }
    ret = esp_wifi_scan_get_ap_records(&ap_count, ap_list);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get WiFi AP records: %s", esp_err_to_name(ret));
        free(ap_list);
        return;
    }

    // Sort the AP list by signal strength (RSSI)
    for (uint16_t i = 0; i < ap_count - 1; i++) {
        for (uint16_t j = i + 1; j < ap_count; j++) {
            if (ap_list[i].rssi < ap_list[j].rssi) {
                wifi_ap_record_t temp = ap_list[i];
                ap_list[i]            = ap_list[j];
                ap_list[j]            = temp;
            }
        }
    }

    // Make a deduplicated list of networks to show
    uint16_t ssid_count = 0;
    memset(scanned_ssid_list, 0, sizeof(scanned_ssid_list));
    for (uint16_t i = 0; i < ap_count && ssid_count < MAX_SSID_COUNT; i++) {
        bool found = false;
        for (uint16_t j = 0; j < ssid_count; j++) {
            if (strcmp((const char *)ap_list[i].ssid, scanned_ssid_list[j]) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            strncpy(scanned_ssid_list[ssid_count], (const char *)ap_list[i].ssid, MAX_SSID_LENGTH);
            ssid_count++;
        }
    }
    free(ap_list);

    // Clean up the scanning animation container
    lv_obj_delete(net_scan_container);
    net_scan_container = NULL;

    // Show the networks found during the scan
    net_show_container = lv_obj_create(overlay);
    lv_obj_set_size(net_show_container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(net_show_container, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_style_border_width(net_show_container, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(net_show_container, lv_color_hex(ORANGE_DIM), LV_PART_MAIN);
    lv_obj_set_style_radius(net_show_container, 3, LV_PART_MAIN);
    lv_obj_align(net_show_container, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_flex_flow(net_show_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(net_show_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

    lv_obj_t *close_button = lv_button_create(overlay);
    lv_obj_set_size(close_button, 25, 25);
    lv_obj_align_to(close_button, overlay, LV_ALIGN_OUT_TOP_RIGHT, -5, 30);
    lv_obj_set_style_bg_color(close_button, lv_color_hex(ORANGE_MAIN), LV_PART_MAIN);
    lv_obj_set_style_radius(close_button, 0, LV_PART_MAIN);
    lv_obj_t *close_label = lv_label_create(close_button);
    lv_label_set_text(close_label, LV_SYMBOL_CLOSE);
    lv_obj_center(close_label);
    lv_obj_add_event_cb(close_button, close_modal_event_cb, LV_EVENT_CLICKED, overlay);

    lv_obj_t *network_list = lv_list_create(net_show_container);
    lv_obj_set_size(network_list, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(network_list, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(network_list, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(network_list, 1, LV_PART_MAIN);
    lv_obj_set_flex_grow(network_list, 1);

    // Modal button style to apply to delete and other buttons
    static lv_style_t btn_style;
    lv_style_init(&btn_style);
    lv_style_set_bg_color(&btn_style, lv_color_hex(ORANGE_MAIN));
    lv_style_set_border_width(&btn_style, 2);
    lv_style_set_border_color(&btn_style, lv_color_hex(ORANGE_DIM));
    lv_style_set_radius(&btn_style, 3);
    lv_style_set_text_color(&btn_style, lv_color_hex(WHITE));
    lv_style_set_text_font(&btn_style, &cyberphont3b_16);

    for (uint16_t i = 0; i < ssid_count; i++) {
        lv_obj_t *btn       = lv_list_add_button(network_list, LV_SYMBOL_WIFI, scanned_ssid_list[i]);
        lv_obj_t *btn_label = lv_obj_get_child(btn, 1);
        lv_obj_set_style_text_font(btn_label, &bm_mini_16, LV_PART_MAIN);
        lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
        lv_obj_set_user_data(btn, (void *)scanned_ssid_list[i]);
        lv_obj_add_event_cb(btn, network_modal_event_cb, LV_EVENT_CLICKED, BTN_NETWORK_ADD);
    }
}

// Callback for setting wrist orientation
static void set_wrist_orientation_event_cb(lv_event_t *e) {
    badge_wrist_t wrist = (badge_wrist_t)lv_event_get_user_data(e);
    ESP_LOGD(TAG, "Setting wrist orientation: %d", wrist);
    set_badge_wrist(wrist);
    lv_obj_delete(wrist_orientation_modal);
    wrist_orientation_modal = NULL;
}

// Wrist orientation popup
static void wrist_orientation_edit(lv_obj_t *parent) {
    wrist_orientation_modal = lv_obj_create(lv_screen_active());
    lv_obj_set_size(wrist_orientation_modal, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(wrist_orientation_modal, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(wrist_orientation_modal, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_border_width(wrist_orientation_modal, 0, LV_PART_MAIN);
    lv_obj_align(wrist_orientation_modal, LV_ALIGN_CENTER, 0, 0);
    lv_obj_remove_flag(wrist_orientation_modal, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *inner_container = lv_obj_create(wrist_orientation_modal);
    lv_obj_set_size(inner_container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(inner_container, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_style_border_width(inner_container, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(inner_container, lv_color_hex(ORANGE_DIM), LV_PART_MAIN);
    lv_obj_set_style_radius(inner_container, 3, LV_PART_MAIN);
    lv_obj_align(inner_container, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_flex_flow(inner_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(inner_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *close_button = lv_button_create(wrist_orientation_modal);
    lv_obj_set_size(close_button, 25, 25);
    lv_obj_align_to(close_button, inner_container, LV_ALIGN_OUT_TOP_RIGHT, 5, 20);
    lv_obj_set_style_bg_color(close_button, lv_color_hex(ORANGE_MAIN), LV_PART_MAIN);
    lv_obj_set_style_radius(close_button, 0, LV_PART_MAIN);
    lv_obj_t *close_label = lv_label_create(close_button);
    lv_label_set_text(close_label, LV_SYMBOL_CLOSE);
    lv_obj_center(close_label);
    lv_obj_add_event_cb(close_button, close_modal_event_cb, LV_EVENT_CLICKED, wrist_orientation_modal);

    lv_obj_t *wrist_orientation_label = lv_label_create(inner_container);
    lv_label_set_text(wrist_orientation_label, "Wrist Orientation");
    lv_obj_set_style_text_font(wrist_orientation_label, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(wrist_orientation_label, lv_color_hex(BLACK), LV_PART_MAIN);
    lv_obj_set_style_margin_bottom(wrist_orientation_label, 10, LV_PART_MAIN);

    lv_obj_t *button_row = lv_obj_create(inner_container);
    lv_obj_set_size(button_row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_border_width(button_row, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(button_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(button_row, 4, LV_PART_MAIN);
    lv_obj_set_flex_flow(button_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(button_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *left_container = lv_obj_create(button_row);
    lv_obj_set_size(left_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_border_width(left_container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(left_container, 4, LV_PART_MAIN);
    lv_obj_set_flex_flow(left_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(left_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *left_btn = lv_button_create(left_container);
    lv_obj_add_style(left_btn, &button_style, LV_PART_MAIN);
    lv_obj_set_style_bg_color(left_btn, lv_color_hex(GRAY_TINT_5), LV_PART_MAIN);
    lv_obj_set_style_border_color(left_btn, lv_color_hex(GRAY_BASE), LV_PART_MAIN);
    lv_obj_t *left_img = lv_image_create(left_btn);
    lv_image_set_src(left_img, &left_hand);
    lv_obj_add_event_cb(left_btn, set_wrist_orientation_event_cb, LV_EVENT_CLICKED, (void *)BADGE_WRIST_LEFT);

    lv_obj_t *left_label = lv_label_create(left_container);
    lv_label_set_text(left_label, "Left");
    lv_obj_set_style_text_font(left_label, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(left_label, lv_color_hex(BLACK), LV_PART_MAIN);

    lv_obj_t *right_container = lv_obj_create(button_row);
    lv_obj_set_size(right_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_border_width(right_container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(right_container, 4, LV_PART_MAIN);
    lv_obj_set_flex_flow(right_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(right_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *right_btn = lv_button_create(right_container);
    lv_obj_add_style(right_btn, &button_style, LV_PART_MAIN);
    lv_obj_set_style_bg_color(right_btn, lv_color_hex(GRAY_TINT_5), LV_PART_MAIN);
    lv_obj_set_style_border_color(right_btn, lv_color_hex(GRAY_BASE), LV_PART_MAIN);
    lv_obj_t *right_img = lv_image_create(right_btn);
    lv_image_set_src(right_img, &right_hand);
    lv_obj_add_event_cb(right_btn, set_wrist_orientation_event_cb, LV_EVENT_CLICKED, (void *)BADGE_WRIST_RIGHT);

    lv_obj_t *right_label = lv_label_create(right_container);
    lv_label_set_text(right_label, "Right");
    lv_obj_set_style_text_font(right_label, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(right_label, lv_color_hex(BLACK), LV_PART_MAIN);
}

// Callback for the screen brightness slider
static void screen_brightness_event_cb(lv_event_t *e) {
    lv_obj_t *slider     = lv_event_get_target(e);
    int brightness_value = lv_slider_get_value(slider);

    set_backlight(brightness_value);
    badge_config.brightness = brightness_value;
    save_badge_config();

    ESP_LOGD(TAG, "Screen brightness changed to %d", brightness_value);
}

// Screen brightness popup
static void screen_brightness_edit(lv_obj_t *parent) {
    lv_obj_t *brightness_modal = lv_obj_create(lv_screen_active());
    lv_obj_set_size(brightness_modal, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(brightness_modal, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(brightness_modal, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_border_width(brightness_modal, 0, LV_PART_MAIN);
    lv_obj_align(brightness_modal, LV_ALIGN_CENTER, 0, 0);
    lv_obj_remove_flag(brightness_modal, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *inner_container = lv_obj_create(brightness_modal);
    lv_obj_set_size(inner_container, lv_pct(90), lv_pct(50));
    lv_obj_set_style_bg_color(inner_container, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_style_border_width(inner_container, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(inner_container, lv_color_hex(ORANGE_DIM), LV_PART_MAIN);
    lv_obj_set_style_radius(inner_container, 3, LV_PART_MAIN);
    lv_obj_align(inner_container, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_flex_flow(inner_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(inner_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

    lv_obj_t *close_button = lv_button_create(brightness_modal);
    lv_obj_set_size(close_button, 25, 25);
    lv_obj_align_to(close_button, inner_container, LV_ALIGN_OUT_TOP_RIGHT, 5, 20);
    lv_obj_set_style_bg_color(close_button, lv_color_hex(ORANGE_MAIN), LV_PART_MAIN);
    lv_obj_set_style_radius(close_button, 0, LV_PART_MAIN);
    lv_obj_t *close_label = lv_label_create(close_button);
    lv_label_set_text(close_label, LV_SYMBOL_CLOSE);
    lv_obj_center(close_label);
    lv_obj_add_event_cb(close_button, close_modal_event_cb, LV_EVENT_CLICKED, brightness_modal);

    lv_obj_t *slider_label = lv_label_create(inner_container);
    lv_label_set_text(slider_label, "Brightness");
    lv_obj_set_style_text_font(slider_label, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(slider_label, lv_color_hex(BLACK), LV_PART_MAIN);
    lv_obj_set_style_margin_bottom(slider_label, 10, LV_PART_MAIN);
    lv_obj_align(slider_label, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t *slider = lv_slider_create(inner_container);
    lv_obj_set_width(slider, lv_pct(100));
    lv_slider_set_range(slider, 0, 255);
    lv_slider_set_value(slider, badge_config.brightness, LV_ANIM_OFF);
    lv_obj_align(slider, LV_ALIGN_CENTER, 0, 10);

    // Style the slider to have square edges
    lv_obj_set_style_radius(slider, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(slider, 0, LV_PART_INDICATOR);
    lv_obj_set_style_radius(slider, 0, LV_PART_KNOB);
    lv_obj_set_style_bg_color(slider, lv_color_hex(GRAY_TINT_5), LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, lv_color_hex(ORANGE_MAIN), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, lv_color_hex(ORANGE_DIM), LV_PART_KNOB);

    lv_obj_add_event_cb(slider, screen_brightness_event_cb, LV_EVENT_VALUE_CHANGED, slider);
}

void settings_page_create(lv_obj_t *parent) {
    lv_obj_clean(parent);

    settings_menu = lv_list_create(parent);
    lv_obj_set_size(settings_menu, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_ver(settings_menu, 10, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(settings_menu, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(settings_menu, 0, LV_PART_MAIN);
    lv_obj_set_flex_align(settings_menu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

    // Username setting
    lv_obj_t *username_btn = lv_obj_class_create_obj(&lv_list_button_class, settings_menu);
    lv_obj_class_init_obj(username_btn);
    lv_obj_set_flex_flow(username_btn, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_opa(username_btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(username_btn, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(username_btn, lv_color_hex(ORANGE_LIGHT), LV_PART_MAIN);

    lv_obj_t *username_icon = lv_image_create(username_btn);
    lv_image_set_src(username_icon, LV_SYMBOL_IMAGE);
    lv_obj_align(username_icon, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_text_color(username_icon, lv_color_hex(ORANGE_LIGHTER), LV_PART_MAIN);
    lv_obj_set_style_pad_top(username_icon, 2, LV_PART_MAIN);
    lv_obj_set_size(username_icon, 18, 18);
    lv_image_set_inner_align(username_icon, LV_IMAGE_ALIGN_CENTER);

    lv_obj_t *username_label = lv_label_create(username_btn);
    lv_obj_align(username_label, LV_ALIGN_LEFT_MID, 40, 0);
    lv_label_set_text(username_label, "Username:");
    lv_obj_set_style_text_font(username_label, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(username_label, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_style_pad_ver(username_label, 5, LV_PART_MAIN);

    char username_str[64];
    snprintf(username_str, sizeof(username_str), strlen(badge_config.handle) > 0 ? "%s" : "<not set>", badge_config.handle);
    username_value = lv_label_create(username_btn);
    lv_label_set_text(username_value, username_str);
    lv_obj_set_style_text_font(username_value, &bm_mini_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(
        username_value, strlen(badge_config.handle) > 0 ? lv_color_hex(GRAY_TINT_6) : lv_color_hex(GRAY_TINT_2), LV_PART_MAIN);
    lv_obj_set_style_pad_ver(username_value, 5, LV_PART_MAIN);
    lv_label_set_long_mode(username_value, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_flex_grow(username_value, 1);
    lv_obj_add_event_cb(username_btn, username_edit_event_cb, LV_EVENT_CLICKED, parent);

    lv_obj_t *list_btn;
    lv_obj_t *list_btn_img;
    lv_obj_t *list_btn_lbl;

#ifdef CONFIG_ALLOW_EXTERNAL_WIFI_NETWORKS
    if (true /* badge_config.badge_team || badge_config.custom_wifi */) { // Post-con: enable for everyone
        // WiFi networks management
        list_btn = lv_list_add_button(settings_menu, LV_SYMBOL_WIFI, "WiFi Networks");
        lv_obj_set_style_bg_opa(list_btn, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(list_btn, 2, LV_PART_MAIN);
        lv_obj_set_style_border_color(list_btn, lv_color_hex(ORANGE_LIGHT), LV_PART_MAIN);
        list_btn_img = lv_obj_get_child_by_type(list_btn, 0, &lv_image_class);
        list_btn_lbl = lv_obj_get_child_by_type(list_btn, -1, &lv_label_class);
        lv_obj_set_style_text_color(list_btn_img, lv_color_hex(ORANGE_LIGHTER), LV_PART_MAIN);
        lv_obj_set_style_pad_top(list_btn_img, 2, LV_PART_MAIN);
        lv_obj_set_size(list_btn_img, 18, 18);
        lv_image_set_inner_align(list_btn_img, LV_IMAGE_ALIGN_CENTER);
        lv_obj_set_style_text_font(list_btn_lbl, &cyberphont3b_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(list_btn_lbl, lv_color_hex(WHITE), LV_PART_MAIN);
        lv_obj_set_style_pad_ver(list_btn_lbl, 5, LV_PART_MAIN);
        lv_obj_add_event_cb(list_btn, wifi_networks_event_cb, LV_EVENT_CLICKED, parent);
    }
#endif

    // Wrist orientation toggle
    list_btn = lv_list_add_button(settings_menu, LV_SYMBOL_REFRESH, "Wrist Orientation");
    lv_obj_set_style_bg_opa(list_btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(list_btn, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(list_btn, lv_color_hex(ORANGE_LIGHT), LV_PART_MAIN);
    list_btn_img = lv_obj_get_child_by_type(list_btn, 0, &lv_image_class);
    list_btn_lbl = lv_obj_get_child_by_type(list_btn, -1, &lv_label_class);
    lv_obj_set_style_text_color(list_btn_img, lv_color_hex(ORANGE_LIGHTER), LV_PART_MAIN);
    lv_obj_set_size(list_btn_img, 18, 18);
    lv_image_set_inner_align(list_btn_img, LV_IMAGE_ALIGN_CENTER);
    lv_obj_set_style_text_font(list_btn_lbl, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(list_btn_lbl, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_style_pad_ver(list_btn_lbl, 5, LV_PART_MAIN);
    lv_obj_add_event_cb(list_btn, wrist_orientation_event_cb, LV_EVENT_CLICKED, parent);

    // Screen brightness setting
    list_btn = lv_list_add_button(settings_menu, LV_SYMBOL_SETTINGS, "Screen Brightness");
    lv_obj_set_style_bg_opa(list_btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(list_btn, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(list_btn, lv_color_hex(ORANGE_LIGHT), LV_PART_MAIN);
    list_btn_img = lv_obj_get_child_by_type(list_btn, 0, &lv_image_class);
    list_btn_lbl = lv_obj_get_child_by_type(list_btn, -1, &lv_label_class);
    lv_obj_set_style_text_color(list_btn_img, lv_color_hex(ORANGE_LIGHTER), LV_PART_MAIN);
    lv_obj_set_size(list_btn_img, 18, 18);
    lv_image_set_inner_align(list_btn_img, LV_IMAGE_ALIGN_CENTER);
    lv_obj_set_style_text_font(list_btn_lbl, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(list_btn_lbl, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_style_pad_ver(list_btn_lbl, 5, LV_PART_MAIN);
    lv_obj_add_event_cb(list_btn, screen_brightness_edit, LV_EVENT_CLICKED, parent);
}
