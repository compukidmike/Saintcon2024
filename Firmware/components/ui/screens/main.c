#include "content.h"
#include "main.h"
#include "theme.h"
#include "statusbar.h"
#include "ui.h"

lv_obj_t *scr_main  = NULL;
lv_obj_t *scr_inner = NULL;

lv_obj_t *create_main_screen() {
    static ui_screen_t screen_type = SCREEN_MAIN;

    // Create a screen object for the main screen
    scr_main = lv_obj_create(NULL);
    lv_obj_set_user_data(scr_main, &screen_type);
    lv_obj_set_size(scr_main, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(scr_main, lv_color_hex(0x000000), LV_PART_MAIN);

    // Create the inner screen container
    scr_inner = lv_obj_create(scr_main);
    lv_obj_set_size(scr_inner, LV_HOR_RES, LV_VER_RES);
    lv_obj_align(scr_inner, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_scrollbar_mode(scr_inner, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(scr_inner, lv_color_hex(CONTENT_BG), LV_PART_MAIN);
    lv_obj_set_style_border_width(scr_inner, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(scr_inner, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(scr_inner, 7, LV_PART_MAIN);

    // Create the status bar
    create_status_bar(scr_inner);

    // Create the content area
    create_content_area(scr_inner);

    return scr_main;
}