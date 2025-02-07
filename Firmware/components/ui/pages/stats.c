#include "badge.h"
#include "stats.h"
#include "theme.h"

void stats_page_create(lv_obj_t *parent) {
    // Clear the content area
    lv_obj_clean(parent);

    // Container to hold the grid of stats
    lv_obj_t *container = lv_obj_create(parent);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(container, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(container, 0, LV_PART_MAIN);
    lv_obj_remove_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Create a grid layout
    lv_obj_t *grid = lv_obj_create(container);
    lv_obj_set_size(grid, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(grid, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(grid, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(grid, 0, LV_PART_MAIN);
    lv_obj_set_layout(grid, LV_LAYOUT_GRID);

    // Create a grid layout
    static lv_coord_t column_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[]    = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(grid, column_dsc, row_dsc);

    // Stats: Player ID, XP, Level, Coins
    lv_obj_t *id_label = lv_label_create(grid);
    lv_label_set_text(id_label, "Player ID");
    lv_obj_set_style_text_font(id_label, &clarity_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(id_label, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_grid_cell(id_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *id_value = lv_label_create(grid);
    lv_label_set_text_fmt(id_value, "%d", badge_config.id);
    lv_obj_set_style_text_font(id_value, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(id_value, lv_color_hex(GRAY_TINT_5), LV_PART_MAIN);
    lv_obj_set_grid_cell(id_value, LV_GRID_ALIGN_END, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *xp_label = lv_label_create(grid);
    lv_label_set_text(xp_label, "XP");
    lv_obj_set_style_text_font(xp_label, &clarity_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(xp_label, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_grid_cell(xp_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    lv_obj_t *xp_value = lv_label_create(grid);
    lv_label_set_text_fmt(xp_value, "%d", badge_config.xp);
    lv_obj_set_style_text_font(xp_value, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(xp_value, lv_color_hex(GRAY_TINT_5), LV_PART_MAIN);
    lv_obj_set_grid_cell(xp_value, LV_GRID_ALIGN_END, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    lv_obj_t *level_label = lv_label_create(grid);
    lv_label_set_text(level_label, "Level");
    lv_obj_set_style_text_font(level_label, &clarity_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(level_label, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_grid_cell(level_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);

    lv_obj_t *level_value = lv_label_create(grid);
    lv_label_set_text_fmt(level_value, "%d", badge_config.level);
    lv_obj_set_style_text_font(level_value, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(level_value, lv_color_hex(GRAY_TINT_5), LV_PART_MAIN);
    lv_obj_set_grid_cell(level_value, LV_GRID_ALIGN_END, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);

    lv_obj_t *coins_label = lv_label_create(grid);
    lv_label_set_text(coins_label, "Coin Balance");
    lv_obj_set_style_text_font(coins_label, &clarity_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(coins_label, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_grid_cell(coins_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 3, 1);

    lv_obj_t *coins_value = lv_label_create(grid);
    lv_label_set_text_fmt(coins_value, "$%d", badge_config.coins);
    lv_obj_set_style_text_font(coins_value, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(coins_value, lv_color_hex(GRAY_TINT_5), LV_PART_MAIN);
    lv_obj_set_grid_cell(coins_value, LV_GRID_ALIGN_END, 1, 1, LV_GRID_ALIGN_CENTER, 3, 1);
}
