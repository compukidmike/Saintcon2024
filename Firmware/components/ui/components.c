#include "esp_log.h"

#include "components.h"
#include "theme.h"

static const char *TAG = "components";

typedef struct {
    lv_obj_t *overlay;
    lv_obj_t *textarea;
    lv_obj_t *keyboard;
    input_prompt_cb_t callback;
    void *user_data;
} input_prompt_data_t;

static void keyboard_event_cb(lv_event_t *e) {
    lv_event_code_t code        = lv_event_get_code(e);
    input_prompt_data_t *prompt = lv_event_get_user_data(e);

    if (code == LV_EVENT_CANCEL || code == LV_EVENT_READY) {
        if (code == LV_EVENT_READY && prompt->callback) {
            const char *text = lv_textarea_get_text(prompt->textarea);
            prompt->callback(code, text, prompt->user_data);
        }
        lv_obj_delete(prompt->overlay);
        lv_obj_delete(prompt->keyboard);
        free(prompt);
    }
}

void input_prompt(const char *initial_text, const char *placeholder, bool password_mode, input_prompt_cb_t callback,
                  void *user_data) {
    // Allocate memory for the prompt data
    input_prompt_data_t *prompt = malloc(sizeof(input_prompt_data_t));
    if (!prompt) {
        ESP_LOGE(TAG, "Failed to allocate memory for input prompt");
        return;
    }
    prompt->callback  = callback;
    prompt->user_data = user_data;

    // Create a modal overlay
    lv_obj_t *modal = lv_obj_create(lv_screen_active());
    lv_obj_set_size(modal, LV_HOR_RES, LV_VER_RES / 2);
    lv_obj_set_style_bg_color(modal, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(modal, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_border_width(modal, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(modal, 0, LV_PART_MAIN);
    lv_obj_align(modal, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_scrollbar_mode(modal, LV_SCROLLBAR_MODE_OFF);
    prompt->overlay = modal;

    // Create a text area
    lv_obj_t *textarea = lv_textarea_create(modal);
    lv_obj_set_size(textarea, lv_pct(100), lv_pct(100));
    lv_obj_set_style_border_width(textarea, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(textarea, lv_color_hex(BLACK), LV_PART_MAIN);
    lv_obj_set_style_radius(textarea, 3, LV_PART_MAIN);
    lv_obj_set_style_text_font(textarea, &bm_mini_16, LV_PART_MAIN);
    lv_obj_align(textarea, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_state(textarea, LV_STATE_FOCUSED);
    lv_textarea_set_text(textarea, initial_text);
    lv_textarea_set_placeholder_text(textarea, placeholder);
    if (password_mode) {
        lv_textarea_set_password_mode(textarea, true);
        lv_textarea_set_password_show_time(textarea, 1000);
    }
    prompt->textarea = textarea;

    // Create a keyboard
    lv_obj_t *keyboard = lv_keyboard_create(lv_screen_active());
    lv_obj_set_size(keyboard, LV_HOR_RES, LV_VER_RES / 2);
    lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_keyboard_set_textarea(keyboard, textarea);
    lv_obj_add_event_cb(keyboard, keyboard_event_cb, LV_EVENT_ALL, prompt);
    prompt->keyboard = keyboard;
}