#include <memory.h>
#include "esp_log.h"

#include "badge.h"
#include "levelup.h"
#include "theme.h"
#include "components/modal.h"

static const char *TAG = "pages/levelup";

ESP_EVENT_DEFINE_BASE(LEVELUP_EVENT);

typedef enum {
    LEVELUP_1,
    LEVELUP_2,
    LEVELUP_3,
} levelup_level_t;
static const char *levelup_names[]            = {"LevelUp 1", "LevelUp 2", "LevelUp 3"};
static const levelup_level_t levelup_values[] = {LEVELUP_1, LEVELUP_2, LEVELUP_3};

typedef enum {
    LEVELUP_PAGE_INIT,
    LEVELUP_PAGE_SHOW_LEVELUPS,
} levelup_page_state_t;

typedef struct {
    lv_obj_t *container;
    levelup_page_state_t state;
    esp_event_loop_handle_t event_loop;
} levelup_page_t;

static levelup_page_t page = {0};

// Forward declarations
static void levelup_page_cleanup(lv_event_t *event);
static void levelup_page_show_levelups();
static lv_obj_t *levelup_page_show_levelup_sending();

static void on_levelup_event(void *context, esp_event_base_t base, int32_t id, void *data);

static void levelup_send_click_event_cb(lv_event_t *e);
static void levelup_click_event_cb(lv_event_t *e);
static void close_button_event_cb(lv_event_t *e);

// Level up received - can be shown even when the page is not visible
static lv_obj_t *show_levelup_received();
void handle_levelup_code(ir_code_t *ir_code);

void levelup_page_create(lv_obj_t *parent) {
    // Clear the content area
    lv_obj_clean(parent);

    page.container = lv_obj_create(parent);
    lv_obj_set_size(page.container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(page.container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(page.container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(page.container, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(page.container, 0, LV_PART_MAIN);
    lv_obj_remove_flag(page.container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(page.container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(page.container, levelup_page_cleanup, LV_EVENT_DELETE, NULL);

    // Create the event loop
    esp_event_loop_args_t loop_args = {
        .queue_size      = 10,
        .task_name       = "levelup_async_events",
        .task_stack_size = 4096,
        .task_priority   = 5,
    };
    esp_event_loop_create(&loop_args, &page.event_loop);
    esp_event_handler_register_with(page.event_loop, LEVELUP_EVENT, ESP_EVENT_ANY_ID, on_levelup_event, NULL);

    if (badge_config.can_level) {
        // Create a container for the community info
        lv_obj_t *community_container = lv_obj_create(page.container);
        lv_obj_set_size(community_container, lv_pct(100), lv_pct(100));
        lv_obj_set_style_bg_opa(community_container, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(community_container, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(community_container, 0, LV_PART_MAIN);
        lv_obj_set_style_align(community_container, LV_ALIGN_TOP_MID, LV_PART_MAIN);
        lv_obj_set_flex_flow(community_container, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(community_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_flex_grow(community_container, 1);

        // Create a grid to show the community name and the number of levels the user has in their community
        lv_obj_t *grid = lv_obj_create(community_container);
        lv_obj_set_size(grid, lv_pct(100), lv_pct(100));
        lv_obj_set_style_pad_all(grid, 0, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(grid, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(grid, 0, LV_PART_MAIN);
        lv_obj_set_style_align(grid, LV_ALIGN_TOP_MID, LV_PART_MAIN);
        lv_obj_set_layout(grid, LV_LAYOUT_GRID);
        lv_obj_set_flex_grow(grid, 1);

        static lv_coord_t column_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
        static lv_coord_t row_dsc[]    = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
        lv_obj_set_grid_dsc_array(grid, column_dsc, row_dsc);

        // Community label
        lv_obj_t *community_label = lv_label_create(grid);
        lv_label_set_text(community_label, "Community");
        lv_obj_set_style_text_font(community_label, &clarity_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(community_label, lv_color_hex(WHITE), LV_PART_MAIN);
        lv_obj_set_grid_cell(community_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

        // Community name label
        lv_obj_t *community_name_label = lv_label_create(grid);
        lv_obj_set_style_pad_top(community_name_label, 8, LV_PART_MAIN);
        lv_label_set_text(community_name_label, badge_config.community);
        lv_obj_set_style_text_font(community_name_label, &bm_mini_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(community_name_label, lv_color_hex(GRAY_TINT_8), LV_PART_MAIN);
        lv_obj_set_grid_cell(community_name_label, LV_GRID_ALIGN_END, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

        // Community levels label
        lv_obj_t *community_levels_label = lv_label_create(grid);
        lv_label_set_text(community_levels_label, "Levels");
        lv_obj_set_style_text_font(community_levels_label, &clarity_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(community_levels_label, lv_color_hex(WHITE), LV_PART_MAIN);
        lv_obj_set_grid_cell(community_levels_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

        // Community levels value label
        lv_obj_t *community_levels_value_label = lv_label_create(grid);
        lv_obj_set_style_pad_top(community_levels_value_label, 8, LV_PART_MAIN);
        lv_label_set_text_fmt(community_levels_value_label, "%d", badge_config.community_levels);
        lv_obj_set_style_text_font(community_levels_value_label, &bm_mini_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(community_levels_value_label, lv_color_hex(GRAY_TINT_8), LV_PART_MAIN);
        lv_obj_set_grid_cell(community_levels_value_label, LV_GRID_ALIGN_END, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

        // Flex container for the buttons
        lv_obj_t *btn_container = lv_obj_create(community_container);
        lv_obj_set_size(btn_container, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(btn_container, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(btn_container, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(btn_container, 0, LV_PART_MAIN);
        lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_flex_grow(btn_container, 0);

        // Add a "Send Level Up" button
        lv_obj_t *send_levelup_btn = lv_btn_create(btn_container);
        lv_obj_add_style(send_levelup_btn, &button_style, LV_PART_MAIN);
        lv_obj_add_style(send_levelup_btn, &button_green, LV_PART_MAIN);
        lv_obj_t *send_levelup_label = lv_label_create(send_levelup_btn);
        lv_label_set_text(send_levelup_label, "Send Level Up");
        lv_obj_center(send_levelup_label);
        if (badge_config.community_levels > 0) {
            lv_obj_add_event_cb(send_levelup_btn, levelup_send_click_event_cb, LV_EVENT_CLICKED, NULL);
        } else {
            lv_obj_set_state(send_levelup_btn, LV_STATE_DISABLED, true);
        }
    } else {
        // Not a community staff member so can only receive level ups
        lv_obj_t *description_label = lv_label_create(page.container);
        lv_label_set_text(description_label, "Visit a community to receive level ups");
        lv_obj_set_size(description_label, lv_pct(100), LV_SIZE_CONTENT);
        lv_label_set_long_mode(description_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_line_space(description_label, 10, LV_PART_MAIN);
        lv_obj_set_style_text_font(description_label, &cyberphont3b_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(description_label, lv_color_hex(WHITE), LV_PART_MAIN);
        lv_obj_set_style_text_align(description_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_align(description_label, LV_ALIGN_CENTER, 0, 0);
    }
}

static void levelup_page_cleanup(lv_event_t *event) {
    if (page.container != NULL) {
        esp_event_loop_delete(page.event_loop);
        memset(&page, 0, sizeof(levelup_page_t));
    }
}

static void on_levelup_event(void *context, esp_event_base_t base, int32_t id, void *data) {
    switch (id) {
        case LEVELUP_EVENT_SEND_LEVELUP: //
            lv_obj_t *modal = NULL;
            // lv_async_call(levelup_page_show_levelup_sending, NULL);
            if (lvgl_lock(pdMS_TO_TICKS(1000), __FILE__, __LINE__)) {
                modal = levelup_page_show_levelup_sending();
                lvgl_unlock(__FILE__, __LINE__);
            }

            levelup_level_t *level = (levelup_level_t *)data;

            // Request the code from the API
            ir_code_t ir_code    = {0};
            api_result_t *result = api_request_level_up(*level);
            if (result != NULL) {
                if (result->status == true) {
                    ir_code.code = ((api_ir_code_only_t *)result->data)->code;
                }
                api_free_result(result, true);
            }

            if (ir_code.code > 0) {
                ir_disable_rx();
                // Repeat the code a few times to ensure it gets through
                for (int i = 0; i < 3; i++) {
                    ir_transmit(ir_code.address, ir_code.command);
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
                ir_enable_rx();
            }

            if (lvgl_lock(pdMS_TO_TICKS(1000), __FILE__, __LINE__)) {
                lv_obj_delete_async(modal);
                lvgl_unlock(__FILE__, __LINE__);
            }
            break;
    }
}

static void levelup_send_click_event_cb(lv_event_t *e) {
    levelup_page_show_levelups();
}

static void levelup_click_event_cb(lv_event_t *e) {
    levelup_level_t *level = (levelup_level_t *)lv_event_get_user_data(e);
    ESP_LOGD(TAG, "Sending level up: %s", levelup_names[*level]);
    esp_event_post_to(page.event_loop, LEVELUP_EVENT, LEVELUP_EVENT_SEND_LEVELUP, level, sizeof(levelup_level_t *), 0);
}

static void close_button_event_cb(lv_event_t *e) {
}

static void levelup_page_show_levelups() {
    // Create a modal container
    lv_obj_t *modal = modal_create_base(NULL, true);

    // Add a description label
    lv_obj_t *description_label = lv_label_create(modal);
    lv_label_set_text(description_label, "Select a level to send");
    lv_obj_set_style_text_font(description_label, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(description_label, lv_color_hex(BLACK), LV_PART_MAIN);
    lv_obj_set_flex_grow(description_label, 0);

    // Create a flex container for the buttons
    lv_obj_t *btn_container = lv_obj_create(modal);
    lv_obj_set_size(btn_container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(btn_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn_container, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(btn_container, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_flex_grow(btn_container, 1);

    // Create the 3 level up buttons
    lv_style_t *level_colors[] = {&button_red, &button_blue, &button_green};
    for (int i = 0; i < badge_config.community_levels; i++) {
        lv_obj_t *level_btn = lv_btn_create(btn_container);
        lv_obj_set_size(level_btn, lv_pct(80), 30);
        lv_obj_set_style_margin_all(level_btn, 5, LV_PART_MAIN);
        lv_obj_add_style(level_btn, &button_style, LV_PART_MAIN);
        lv_obj_add_style(level_btn, level_colors[i], LV_PART_MAIN);
        lv_obj_t *level_label = lv_label_create(level_btn);
        lv_label_set_text(level_label, levelup_names[i]);
        lv_obj_center(level_label);
        lv_obj_add_event_cb(level_btn, levelup_click_event_cb, LV_EVENT_CLICKED, &levelup_values[i]);
    }
}

// Clean up the bump animation when the modal is deleted
static void bump_cleanup(lv_event_t *event) {
    lv_obj_t *icon = lv_event_get_user_data(event);
    if (icon != NULL && lv_obj_is_valid(icon)) {
        lv_obj_delete(icon);
    }
}

static lv_obj_t *levelup_page_show_levelup_sending() {
    lv_obj_t *modal        = loading_modal("Sending level up...");
    lv_obj_t *modal_screen = lv_obj_get_screen(modal);
    lv_obj_t *icon         = lv_img_create(modal_screen);
    lv_img_set_src(icon, badge_config.wrist == BADGE_WRIST_LEFT ? &left_bump : &right_bump);
    lv_obj_set_size(icon, left_bump.header.w, left_bump.header.h);
    lv_obj_set_align(icon, badge_config.wrist == BADGE_WRIST_LEFT ? LV_ALIGN_RIGHT_MID : LV_ALIGN_LEFT_MID);
    lv_obj_set_pos(icon, 0, 0);

    // Animate the icon to move left and right
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, icon);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_values(&a, 0, -(left_bump.header.w / 2));
    lv_anim_set_duration(&a, 700);
    lv_anim_set_playback_duration(&a, 700);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);

    // Make sure the icon is deleted when the modal is deleted
    lv_obj_add_event_cb(modal, bump_cleanup, LV_EVENT_DELETE, icon);

    return modal;
}

static lv_obj_t *show_levelup_received() {
    lv_obj_t *modal = modal_create_base(NULL, true);

    // Label container for expansion/centering
    lv_obj_t *label_container = lv_obj_create(modal);
    lv_obj_set_size(label_container, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(label_container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(label_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(label_container, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(label_container, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(label_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(label_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_flex_grow(label_container, 1);

    lv_obj_t *label = lv_label_create(label_container);
    lv_label_set_text(label, "Level up received!");
    lv_obj_set_style_text_font(label, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(BLACK), LV_PART_MAIN);

    // Create a flex container for the OK button
    lv_obj_t *btn_container = lv_obj_create(modal);
    lv_obj_set_size(btn_container, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(btn_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn_container, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(btn_container, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_flex_grow(btn_container, 0);

    lv_obj_t *ok_btn = lv_btn_create(btn_container);
    lv_obj_add_style(ok_btn, &button_style, LV_PART_MAIN);
    lv_obj_add_style(ok_btn, &button_orange, LV_PART_MAIN);
    lv_obj_t *ok_label = lv_label_create(ok_btn);
    lv_label_set_text(ok_label, "OK");
    lv_obj_center(ok_label);
    lv_obj_add_event_cb(ok_btn, base_modal_exit_cb, LV_EVENT_CLICKED, modal);

    return modal;
}

// Handler for IR level up codes - called by `route_high_priority_code(...)` in `ir.c`
void handle_levelup_code(ir_code_t *ir_code) {
    lv_async_call(show_levelup_received, NULL);
}
