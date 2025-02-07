#include "../loadanim.h"
#include "modal.h"
#include "theme.h"

lv_obj_t *modal_create_base(lv_obj_t *screen, bool closeable) {
    if (screen == NULL) {
        screen = lv_screen_active();
    }

    // Screen overlay
    lv_obj_t *overlay = lv_obj_create(screen);
    lv_obj_set_size(overlay, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(overlay, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_bg_color(overlay, lv_color_hex(GRAY_SHADE_6), LV_PART_MAIN);
    lv_obj_set_style_border_width(overlay, 0, LV_PART_MAIN);
    lv_obj_align(overlay, LV_ALIGN_CENTER, 0, 0);
    lv_obj_remove_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(overlay, LV_SCROLLBAR_MODE_OFF);

    // Inner container
    lv_obj_t *container = lv_obj_create(overlay);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_style_border_width(container, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(container, lv_color_hex(ORANGE_DIM), LV_PART_MAIN);
    lv_obj_set_style_radius(container, 3, LV_PART_MAIN);
    lv_obj_align(container, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_add_event_cb(container, base_modal_exit_cb, LV_EVENT_DELETE, overlay);

    // Add a close button
    if (closeable) {
        lv_obj_t *close_btn = lv_button_create(overlay);
        lv_obj_set_size(close_btn, 25, 25);
        lv_obj_align_to(close_btn, container, LV_ALIGN_OUT_TOP_RIGHT, 5, 20);
        lv_obj_set_style_bg_color(close_btn, lv_color_hex(RED_MAIN), LV_PART_MAIN);
        lv_obj_set_style_radius(close_btn, 0, LV_PART_MAIN);
        lv_obj_add_event_cb(close_btn, base_modal_exit_cb, LV_EVENT_CLICKED, overlay);

        lv_obj_t *close_label = lv_label_create(close_btn);
        lv_label_set_text(close_label, LV_SYMBOL_CLOSE);
        lv_obj_center(close_label);
    }

    return container;
}

void base_modal_exit_cb(lv_event_t *event) {
    lv_obj_t *overlay = lv_event_get_user_data(event);
    if (lv_obj_is_valid(overlay)) {
        lv_obj_delete_async(overlay);
    }
}

lv_obj_t *loading_modal(const char *label_text) {
    lv_obj_t *modal     = modal_create_base(NULL, false);
    lv_obj_t *container = lv_obj_create(modal);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_radius(container, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(container, 0, LV_PART_MAIN);
    lv_obj_set_flex_grow(container, 1);
    lv_obj_t *label = lv_label_create(container);
    lv_label_set_text(label, label_text);
    lv_obj_set_style_text_font(label, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(BLACK), LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_line_space(label, 10, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -25);

    loading_dots_anim_config_t config = {
        .parent            = container,
        .align_to          = label,
        .x_offset          = 0,
        .y_offset          = 20,
        .dot_color         = lv_color_hex(BLACK),
        .dot_size          = 10,
        .fade_in_duration  = 250,
        .fade_out_duration = 250,
        .sequence_delay    = 200,
        .repeat_delay      = 500,
    };
    loading_dots_anim(&config);

    return modal;
}