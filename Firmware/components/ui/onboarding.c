#include <string.h>
#include "esp_log.h"

#include "api.h"
#include "badge.h"
#include "onboarding.h"
#include "theme.h"

static const char *TAG = "onboarding";

static bool onboarding_showing        = false;
static lv_obj_t *onboarding_container = NULL;
static lv_obj_t *username_ta          = NULL;
static lv_obj_t *kb                   = NULL;
static lv_obj_t *entry_overlay        = NULL;
static lv_obj_t *entry_ta             = NULL;
static lv_obj_t *submit_btn           = NULL;

// Event handler prototypes
static void submit_handle_event_cb(lv_event_t *e);
static void ta_event_cb(lv_event_t *e);
static void kb_event_cb(lv_event_t *e);

void onboarding_show() {
    if (onboarding_showing) {
        return;
    }
    onboarding_showing = true;

    // Create an overlay to show the onboarding screen
    onboarding_container = lv_obj_create(lv_screen_active());
    lv_obj_set_size(onboarding_container, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(onboarding_container, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(onboarding_container, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_border_width(onboarding_container, 0, LV_PART_MAIN);
    lv_obj_align(onboarding_container, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_scrollbar_mode(onboarding_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_move_foreground(onboarding_container);

    // Inner container for the label, text box, and buttons
    lv_obj_t *onboarding_inner = lv_obj_create(onboarding_container);
    lv_obj_set_size(onboarding_inner, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(onboarding_inner, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_border_width(onboarding_inner, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(onboarding_inner, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_radius(onboarding_inner, 3, LV_PART_MAIN);
    lv_obj_align(onboarding_inner, LV_ALIGN_CENTER, 0, 0);

    // Label for the onboarding screen
    lv_obj_t *onboarding_label = lv_label_create(onboarding_inner);
    lv_label_set_text(onboarding_label, "Welcome to SAINTCON!");
    lv_obj_set_style_text_font(onboarding_label, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(onboarding_label, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_align(onboarding_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(onboarding_label, LV_ALIGN_TOP_MID, 0, 10);

    // Text area for the username
    username_ta = lv_textarea_create(onboarding_inner);
    lv_textarea_set_text(username_ta, badge_config.handle);
    lv_textarea_set_placeholder_text(username_ta, "Enter your handle");
    lv_obj_set_style_text_font(username_ta, &bm_mini_16, LV_PART_MAIN);
    lv_obj_set_style_radius(username_ta, 3, LV_PART_MAIN);
    lv_obj_set_size(username_ta, lv_pct(90), 30);
    lv_obj_align(username_ta, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_scrollbar_mode(username_ta, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(username_ta, ta_event_cb, LV_EVENT_CLICKED, NULL);

    // Button to save the username
    submit_btn = lv_button_create(onboarding_inner);
    lv_obj_align(submit_btn, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_t *label = lv_label_create(submit_btn);
    lv_label_set_text(label, "Register");
    lv_obj_set_style_text_font(label, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_bg_color(submit_btn, lv_color_hex(STATUS_BG), LV_PART_MAIN);
    lv_obj_set_style_border_width(submit_btn, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(submit_btn, lv_color_hex(STATUS_BORDER), LV_PART_MAIN);
    lv_obj_set_style_radius(submit_btn, 3, LV_PART_MAIN);
    lv_obj_add_state(submit_btn, LV_STATE_DISABLED);

    // Event handler for the save button
    lv_obj_add_event_cb(submit_btn, submit_handle_event_cb, LV_EVENT_CLICKED, username_ta);
}

bool onboarding_is_showing() {
    return onboarding_showing;
}

void onboarding_hide() {
    if (onboarding_container != NULL) {
        lv_obj_delete(onboarding_container);
        onboarding_container = NULL;
        onboarding_showing   = false;
    }
}

static void kb_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        if (entry_ta != NULL) {
            // Synchronize temporary textarea content with the original textarea
            lv_textarea_set_text(username_ta, lv_textarea_get_text(entry_ta));
            lv_textarea_set_cursor_pos(username_ta, lv_textarea_get_cursor_pos(entry_ta));

            // Delete the temporary textarea and keyboard
            lv_obj_delete(entry_ta);
            entry_ta = NULL;
            lv_obj_delete(entry_overlay);
            entry_overlay = NULL;
        }
        lv_obj_delete(kb);
        kb = NULL;

        // Check if the submit button should be enabled
        if (strlen(lv_textarea_get_text(username_ta)) == 0) {
            lv_obj_add_state(submit_btn, LV_STATE_DISABLED);
        } else {
            lv_obj_clear_state(submit_btn, LV_STATE_DISABLED);
        }
    }
}

static void ta_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta         = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED) {
        if (kb == NULL) {
            // Save current cursor position and unfocus the text area
            uint32_t cursor_pos = lv_textarea_get_cursor_pos(ta);
            lv_obj_remove_state(ta, LV_STATE_FOCUSED);

            // Create a temporary textarea inside an overlay
            entry_overlay = lv_obj_create(lv_screen_active());
            lv_obj_set_size(entry_overlay, LV_HOR_RES, LV_VER_RES / 2);
            lv_obj_set_style_bg_color(entry_overlay, lv_color_hex(0x333333), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(entry_overlay, LV_OPA_80, LV_PART_MAIN);
            lv_obj_set_style_border_width(entry_overlay, 0, LV_PART_MAIN);
            lv_obj_set_style_radius(entry_overlay, 0, LV_PART_MAIN);
            lv_obj_align(entry_overlay, LV_ALIGN_TOP_MID, 0, 0);
            lv_obj_set_scrollbar_mode(entry_overlay, LV_SCROLLBAR_MODE_OFF);

            entry_ta = lv_textarea_create(entry_overlay);
            lv_obj_set_size(entry_ta, lv_pct(100), lv_pct(100));
            lv_obj_set_style_border_width(entry_ta, 2, LV_PART_MAIN);
            lv_obj_set_style_border_color(entry_ta, lv_color_hex(0x000000), LV_PART_MAIN);
            lv_obj_set_style_radius(entry_ta, 3, LV_PART_MAIN);
            lv_obj_set_style_text_font(entry_ta, &bm_mini_16, LV_PART_MAIN);
            lv_obj_align(entry_ta, LV_ALIGN_CENTER, 0, 0);
            lv_obj_add_state(entry_ta, LV_STATE_FOCUSED);
            lv_textarea_set_text(entry_ta, lv_textarea_get_text(ta));
            lv_textarea_set_placeholder_text(entry_ta, lv_textarea_get_placeholder_text(ta));
            lv_textarea_set_cursor_pos(entry_ta, cursor_pos);

            // Create the keyboard
            kb = lv_keyboard_create(lv_screen_active());
            lv_obj_set_size(kb, LV_HOR_RES, LV_VER_RES / 2);
            lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
            lv_keyboard_set_textarea(kb, entry_ta);
            lv_obj_add_event_cb(kb, kb_event_cb, LV_EVENT_ALL, ta);
        }
    }
}

// Callback for the save button on the onboarding screen
static void submit_handle_event_cb(lv_event_t *e) {
    const lv_obj_t *ta = lv_event_get_user_data(e);

    // Trim the handle string
    const char *handle = lv_textarea_get_text(ta);
    const char *start  = handle;
    while (*start && *start == ' ') {
        start++;
    }
    char *end = handle + strlen(handle) - 1;
    while (end > handle && *end == ' ') {
        end--;
    }
    *(end + 1) = '\0';
    if (start != handle) {
        memmove(handle, start, end - start + 2);
    }

    // Check if the handle is empty
    if (strlen(handle) == 0) {
        lv_textarea_set_text(ta, "");
        return;
    }

    // Save the entered username
    strcpy(badge_config.handle, handle);

    // Try to register the username with the API
    api_result_t *result = api_register(handle);
    if (result == NULL) {
        show_msgbox("Registration failed... we'll save your username and try again later");
    } else {
        if (result->status != true) {
            char msg[256] = {0};
            snprintf(msg, sizeof(msg), "Registration failed... we'll save your username and try again later.\nMessage: %s",
                     result->detail);
            show_msgbox(msg);
        } else {
            badge_config.registered = true;
            show_msgbox("Registration successful!");
        }
        api_free_result(result, true);
    }

    // Save the badge configuration to flash
    save_badge_config();

    // Close the onboarding screen
    lv_obj_delete(onboarding_container);
    onboarding_container = NULL;
    onboarding_showing   = false;
}
