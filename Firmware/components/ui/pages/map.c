#include <stdio.h>
#include "esp_log.h"
#include "lvgl.h"
#include "lvgl_private.h"

#include "map.h"
#include "theme.h"

static const char *TAG = "pages/map";

// Conference center levels
typedef enum {
    MAP_LEVEL_1,
    MAP_LEVEL_2,
    MAP_LEVEL_3,
} map_level_t;

// Map files to load for each level
static const char *map_files[] = {
    "/spiffs/LVL1.bin",
    "/spiffs/LVL2.bin",
    "/spiffs/LVL3.bin",
};

static void level_click_event_cb(lv_event_t *e);
static void close_button_event_cb(lv_event_t *e);
static void show_map(map_level_t level);

static void level_click_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    map_level_t level    = (map_level_t)lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED) {
        show_map(level);
    }
}

static void close_button_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *overlay = lv_obj_get_parent(lv_event_get_target(e));
        lv_obj_del(overlay);
    }
}

static void show_map(map_level_t level) {
    // Create an overlay for the map
    lv_obj_t *overlay = lv_obj_create(lv_screen_active());
    lv_obj_set_size(overlay, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_pad_all(overlay, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(overlay, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(overlay, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(overlay, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(overlay, 0, LV_PART_MAIN);
    lv_obj_remove_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(overlay, LV_SCROLLBAR_MODE_OFF);

    // Path (including FS driver letter) to the map image
    char path[32] = {0};
    snprintf(path, sizeof(path), "S:%s", map_files[level]);

    // Create the map image object
    lv_obj_t *map_img = lv_image_create(overlay);
    lv_image_set_src(map_img, path);
    lv_image_set_antialias(map_img, false);
    lv_obj_align(map_img, LV_ALIGN_CENTER, 0, 0);

    // Add a close button
    lv_obj_t *close_button = lv_button_create(overlay);
    lv_obj_set_size(close_button, 30, 30);
    lv_obj_align(close_button, LV_ALIGN_TOP_RIGHT, -5, 5);
    lv_obj_set_style_bg_color(close_button, lv_color_hex(ORANGE_MAIN), LV_PART_MAIN);
    lv_obj_set_style_radius(close_button, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(close_button, close_button_event_cb, LV_EVENT_CLICKED, overlay);
    lv_obj_t *close_label = lv_label_create(close_button);
    lv_label_set_text(close_label, LV_SYMBOL_CLOSE);
    lv_obj_center(close_label);
}

void map_page_create(lv_obj_t *parent) {
    // Clear the content area
    lv_obj_clean(parent);

    // Create a flex container for the buttons
    lv_obj_t *btn_container = lv_obj_create(parent);
    lv_obj_set_size(btn_container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(btn_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn_container, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(btn_container, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Button style, text, and data
    const char *level_texts[]  = {"Level 3", "Ballroom Level", "Ground Level"};
    map_level_t levels[]       = {MAP_LEVEL_3, MAP_LEVEL_2, MAP_LEVEL_1};
    lv_style_t *level_colors[] = {&button_red, &button_blue, &button_green};

    for (int i = 0; i < 3; i++) {
        lv_obj_t *level_btn = lv_btn_create(btn_container);
        lv_obj_set_size(level_btn, lv_pct(80), 30);
        lv_obj_set_style_margin_all(level_btn, 5, LV_PART_MAIN);
        lv_obj_add_style(level_btn, &button_style, LV_PART_MAIN);
        lv_obj_add_style(level_btn, level_colors[i], LV_PART_MAIN);
        lv_obj_t *level_label = lv_label_create(level_btn);
        lv_label_set_text(level_label, level_texts[i]);
        lv_obj_center(level_label);
        lv_obj_add_event_cb(level_btn, level_click_event_cb, LV_EVENT_CLICKED, (void *)levels[i]);
    }
}
