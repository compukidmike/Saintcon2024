#include "loadanim.h"
#include "theme.h"
#include "splash.h"
#include "ui.h"

LV_IMG_DECLARE(sc24_logo_300);

lv_obj_t *scr_splash = NULL;

lv_obj_t *create_splash_screen() {
    static ui_screen_t screen_type = SCREEN_SPLASH;

    // Create a screen object for the splash screen
    scr_splash = lv_obj_create(NULL);
    lv_obj_set_user_data(scr_splash, &screen_type);
    lv_obj_set_size(scr_splash, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(scr_splash, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_border_width(scr_splash, 0, LV_PART_MAIN);
    lv_obj_align(scr_splash, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_scrollbar_mode(scr_splash, LV_SCROLLBAR_MODE_OFF);

    // Show the SAINTCON logo in the main content area
    lv_obj_t *logo = lv_image_create(scr_splash);
    lv_image_set_src(logo, &sc24_logo_300);
    lv_obj_align(logo, LV_ALIGN_CENTER, 0, -20);

    // Create the loading dots
    loading_dots_anim_config_t dots_config = {
        .parent            = scr_splash,
        .align_to          = logo,
        .x_offset          = 0,
        .y_offset          = 20,
        .dot_color         = lv_color_hex(0xFFFFFF),
        .dot_size          = 10,
        .fade_in_duration  = 250,
        .fade_out_duration = 250,
        .sequence_delay    = 200,
        .repeat_delay      = 500,
    };
    loading_dots_anim(&dots_config);

    return scr_splash;
}

void splash_screen_shutdown() {
    return;
}
