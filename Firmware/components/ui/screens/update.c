#include "esp_log.h"

#include "theme.h"
#include "display.h"
#include "ui.h"
#include "update.h"

static const char *TAG = "screens/update";

lv_obj_t *scr_update      = NULL;
lv_obj_t *update_progress = NULL;
lv_obj_t *progress_label  = NULL;
lv_obj_t *update_message  = NULL;

void bytes_to_string(size_t bytes, char *buffer, size_t buffer_size) {
    if (buffer == NULL) {
        ESP_LOGE(TAG, "Buffer is NULL in bytes_to_string");
        return;
    }
    if (bytes < 1024) {
        snprintf(buffer, buffer_size, "%d B", bytes);
    } else if (bytes < 1024 * 1024) {
        snprintf(buffer, buffer_size, "%.1f KB", (float)bytes / 1024);
    } else {
        snprintf(buffer, buffer_size, "%.1f MB", (float)bytes / (1024 * 1024));
    }
}

lv_obj_t *create_update_screen() {
    static ui_screen_t screen_type = SCREEN_UPDATE;

    // Create a screen object for the update screen
    scr_update = lv_obj_create(NULL);
    lv_obj_set_user_data(scr_update, &screen_type);
    lv_obj_set_size(scr_update, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(scr_update, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_border_width(scr_update, 0, LV_PART_MAIN);
    lv_obj_align(scr_update, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_scrollbar_mode(scr_update, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(scr_update, LV_OBJ_FLAG_SCROLLABLE);

    // Create a label for the update screen
    lv_obj_t *title = lv_label_create(scr_update);
    lv_label_set_text(title, "Firmware Update");
    lv_obj_set_style_text_font(title, &clarity_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // Styles for the progress bar
    static lv_style_t progress_style_bg;
    lv_style_init(&progress_style_bg);
    lv_style_set_border_color(&progress_style_bg, lv_color_hex(GREEN_DIM));
    lv_style_set_border_width(&progress_style_bg, 2);
    lv_style_set_pad_all(&progress_style_bg, 4);
    lv_style_set_radius(&progress_style_bg, 0);

    static lv_style_t progress_style_indicator;
    lv_style_init(&progress_style_indicator);
    lv_style_set_bg_color(&progress_style_indicator, lv_color_hex(GREEN_LIGHT));
    lv_style_set_bg_opa(&progress_style_indicator, LV_OPA_COVER);
    lv_style_set_radius(&progress_style_indicator, 0);

    // Create the progress bar
    update_progress = lv_bar_create(scr_update);
    lv_obj_remove_style_all(update_progress);
    lv_obj_add_style(update_progress, &progress_style_bg, LV_PART_MAIN);
    lv_obj_add_style(update_progress, &progress_style_indicator, LV_PART_INDICATOR);
    lv_obj_set_size(update_progress, LV_HOR_RES - 40, 20);
    lv_obj_align(update_progress, LV_ALIGN_CENTER, 0, 0);
    lv_bar_set_range(update_progress, 0, 100);
    lv_bar_set_value(update_progress, 0, LV_ANIM_ON);

    // Label below to show bytes and percentage
    progress_label = lv_label_create(scr_update);
    lv_label_set_text(progress_label, "0% (0/0)");
    lv_obj_set_style_text_font(progress_label, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(progress_label, lv_color_hex(GRAY_BASE), LV_PART_MAIN);
    lv_obj_align_to(progress_label, update_progress, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    // Create a message label
    update_message = lv_label_create(scr_update);
    lv_label_set_text(update_message, "Downloading...");
    lv_obj_set_style_text_font(update_message, &cyberphont3b_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(update_message, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_align(update_message, LV_ALIGN_BOTTOM_MID, 0, -10);

    return scr_update;
}

void update_ota_state(ota_state_t state) {
    if (lvgl_lock(pdMS_TO_TICKS(portMAX_DELAY), __FILE__, __LINE__)) {
        double percent = ((double)state.progress.bytes_received * 100) / state.progress.bytes_total;
        // Update the progress bar
        lv_bar_set_value(update_progress, percent <= 0 ? 0 : (int)percent, LV_ANIM_ON);

        // Update the progress label
        char received[16];
        char total[16];
        bytes_to_string(state.progress.bytes_received, received, sizeof(received));
        bytes_to_string(state.progress.bytes_total, total, sizeof(total));
        lv_label_set_text_fmt(progress_label, "%.2f%% (%s/%s)", percent, received, total);

        // Update the message label
        lv_label_set_text(update_message, state.message);

        lvgl_unlock(__FILE__, __LINE__);
    } else {
        ESP_LOGW(TAG, "Failed to lock LVGL for update progress");
    }
}