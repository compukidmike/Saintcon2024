#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

// Main color scheme colors
#define RED_LIGHTEST      0xFF5762
#define RED_LIGHTER       0xFF3D4A
#define RED_LIGHT         0xFF2433
#define RED_MAIN          0xFF0A1B
#define RED_DIM           0xF00011
#define RED_DIMMER        0xD6000F
#define RED_DIMMEST       0xBD000D
#define ORANGE_LIGHTEST   0xFFA96C
#define ORANGE_LIGHTER    0xFF9A52
#define ORANGE_LIGHT      0xFF8B39
#define ORANGE_MAIN       0xFF7C1F
#define ORANGE_DIM        0xFF6D05
#define ORANGE_DIMMER     0xEB6200
#define ORANGE_DIMMEST    0xD25700
#define YELLOW_LIGHTEST   0xFFEE80
#define YELLOW_LIGHTER    0xFFEB66
#define YELLOW_LIGHT      0xFFE74D
#define YELLOW_MAIN       0xFFE433
#define YELLOW_DIM        0xFFE11A
#define YELLOW_DIMMER     0xFFDD00
#define YELLOW_DIMMEST    0xE6C700
#define GREEN_LIGHTEST    0x82DC87
#define GREEN_LIGHTER     0x6ED674
#define GREEN_LIGHT       0x5AD161
#define GREEN_MAIN        0x46CB4E
#define GREEN_DIM         0x36C13F
#define GREEN_DIMMER      0x31AD38
#define GREEN_DIMMEST     0x2B9932
#define BLUE_LIGHTEST     0x6CB8FF
#define BLUE_LIGHTER      0x52ACFF
#define BLUE_LIGHT        0x399FFF
#define BLUE_MAIN         0x1F93FF
#define BLUE_DIM          0x0587FF
#define BLUE_DIMMER       0x007AEB
#define BLUE_DIMMEST      0x006CD2
#define PURPLE_LIGHTEST   0xC245C5
#define PURPLE_LIGHTER    0xB439B7
#define PURPLE_LIGHT      0xA133A3
#define PURPLE_MAIN       0x8E2D90
#define PURPLE_DIM        0x7B277D
#define PURPLE_DIMMER     0x682169
#define PURPLE_DIMMEST    0x551B56
#define BLUEGRAY_LIGHTEST 0x8AA6B8
#define BLUEGRAY_LIGHTER  0x7B9AAE
#define BLUEGRAY_LIGHT    0x6B8EA5
#define BLUEGRAY_MAIN     0x5D8199
#define BLUEGRAY_DIM      0x537489
#define BLUEGRAY_DIMMER   0x4A6679
#define BLUEGRAY_DIMMEST  0x405969

// Monochrome colors
#define WHITE 0xFFFFFF
#define BLACK 0x000000

// Gray base color
#define GRAY_BASE 0x9E9E9E

// Gray shades
#define GRAY_SHADE_1 0x8E8E8E
#define GRAY_SHADE_2 0x7E7E7E
#define GRAY_SHADE_3 0x6E6E6E
#define GRAY_SHADE_4 0x5E5E5E
#define GRAY_SHADE_5 0x4F4F4F
#define GRAY_SHADE_6 0x3F3F3F
#define GRAY_SHADE_7 0x2F2F2F
#define GRAY_SHADE_8 0x1F1F1F
#define GRAY_SHADE_9 0x0F0F0F

// Gray tints
#define GRAY_TINT_1 0xA7A7A7
#define GRAY_TINT_2 0xB1B1B1
#define GRAY_TINT_3 0xBBBBBB
#define GRAY_TINT_4 0xC4C4C4
#define GRAY_TINT_5 0xCECECE
#define GRAY_TINT_6 0xD8D8D8
#define GRAY_TINT_7 0xE1E1E1
#define GRAY_TINT_8 0xEBEBEB
#define GRAY_TINT_9 0xF5F5F5

// Theme colors
#define STATUS_BG     ORANGE_MAIN
#define STATUS_BORDER ORANGE_DIMMER
#define STATUS_DIM    ORANGE_DIMMEST
#define STATUS_BRIGHT WHITE
#define STATUS_GOOD   GREEN_MAIN
#define CONTENT_BG    GRAY_SHADE_5

#define MENU_BUTTON_BG     BLUE_MAIN
#define MENU_BUTTON_BORDER BLUE_DIM
#define MENU_BUTTON_TEXT   WHITE

// Various fonts
LV_FONT_DECLARE(bm_mini_8);
LV_FONT_DECLARE(bm_mini_16);
LV_FONT_DECLARE(clarity_14);
LV_FONT_DECLARE(clarity_16);
LV_FONT_DECLARE(clarity_18);
LV_FONT_DECLARE(clarity_20);
LV_FONT_DECLARE(clarity_22);
LV_FONT_DECLARE(clarity_24);
LV_FONT_DECLARE(cyberphont3b_10)
LV_FONT_DECLARE(cyberphont3b_12)
LV_FONT_DECLARE(cyberphont3b_14);
LV_FONT_DECLARE(cyberphont3b_16);

// Clarity icons
#define CLARITY_ARROW_LEFT  "\xE2\x86\x90"
#define CLARITY_ARROW_UP    "\xE2\x86\x91"
#define CLARITY_ARROW_RIGHT "\xE2\x86\x92"
#define CLARITY_ARROW_DOWN  "\xE2\x86\x93"

// Icon fonts
LV_FONT_DECLARE(fa_sharp_icons_16);

// Icon font symbols
#define FA_CHIP      "\xEF\x8B\x9B" // 0xF2DB
#define FA_TOWER     "\xEF\x99\xA4" // 0xF664
#define FA_OMEGA     "\xEF\x99\xBA" // 0xF67A
#define FA_TOMBSTONE "\xEF\x9C\xA0" // 0xF720

// Images
// LV_IMG_DECLARE(theme_screens);
LV_IMG_DECLARE(left_bump);
LV_IMG_DECLARE(right_bump);

// Styles
extern lv_style_t button_style;

extern lv_style_t button_red;
extern lv_style_t button_orange;
extern lv_style_t button_yellow;
extern lv_style_t button_green;
extern lv_style_t button_blue;
extern lv_style_t button_purple;
extern lv_style_t button_bluegrey;
extern lv_style_t button_grey;

// Style initialization
void style_init();

// Theme initialization
// void theme_init();

// Get the current theme
// lv_theme_t *get_theme();

//
// Custom UI window/rectangle "screen" components built using corners and edges from each of the 4 types of screens in the
// "theme_screens" image
//

// Content container - large version
lv_obj_t *content_container_large_create(lv_obj_t *parent, lv_coord_t width, lv_coord_t height);

// Content container - small version
// lv_obj_t *content_container_small_create(lv_obj_t *parent, lv_coord_t width, lv_coord_t height);

void style_msgbox(lv_obj_t *msgbox);

void show_msgbox(const char *text);

#ifdef __cplusplus
}
#endif
