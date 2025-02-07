/*******************************************************************************
 * Size: 14 px
 * Bpp: 1
 * Opts: --bpp 1 --size 14 --no-compress --no-prefilter --font
 * /Users/dwarkentin/Library/CloudStorage/GoogleDrive-ev0rtex@gmail.com/.shortcut-targets-by-id/1M-9uKjsqE0vKNDFWxBemLGBB4Gobi9fB/Saintcon
 *2024 Badge/Assets/cyberphont 3.0/Cyberphont 3.0 B.ttf -r 0x20-0x7C --format lvgl -o
 * /Users/dwarkentin/src/projects/Saintcon2024Dev/badge-firmware/components/ui/fonts/cyberphont3b_14.c --force-fast-kern-format
 ******************************************************************************/

#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif

#ifndef CYBERPHONT3B_14
    #define CYBERPHONT3B_14 1
#endif

#if CYBERPHONT3B_14

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xf3, 0xc0,

    /* U+0022 "\"" */
    0xde, 0xf7, 0xb0,

    /* U+0023 "#" */
    0x36, 0x1b, 0x3f, 0xff, 0xf3, 0x67, 0xff, 0xfe, 0x6c, 0x36, 0x0,

    /* U+0024 "$" */
    0x31, 0xbf, 0xfc, 0x7f, 0xe6, 0x30,

    /* U+0025 "%" */
    0xc1, 0xe0, 0xc1, 0x80, 0xc0, 0x81, 0x80, 0xc1, 0x83, 0xc1, 0x80,

    /* U+0027 "'" */
    0xff,

    /* U+0028 "(" */
    0x33, 0xcc, 0xcc, 0xc3, 0x30,

    /* U+0029 ")" */
    0xcc, 0x33, 0x33, 0x3c, 0xc0,

    /* U+002B "+" */
    0x37, 0xfe, 0x63, 0x0,

    /* U+002C "," */
    0x33, 0xcc,

    /* U+002D "-" */
    0xff, 0xc0,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x1, 0x80, 0xc1, 0x80, 0xc0, 0x81, 0x80, 0xc1, 0x80, 0xc0, 0x0,

    /* U+0030 "0" */
    0x3f, 0x9f, 0xf1, 0xf8, 0xfc, 0x9f, 0x8f, 0xc7, 0xfc, 0xfe, 0x0,

    /* U+0031 "1" */
    0xff, 0xff, 0xc0,

    /* U+0032 "2" */
    0xfe, 0x7f, 0x0, 0x60, 0x3f, 0xfe, 0x3, 0x1, 0xff, 0xff, 0x80,

    /* U+0033 "3" */
    0xfe, 0x7f, 0x0, 0x60, 0x3f, 0xf8, 0xc, 0x7, 0xff, 0xff, 0x80,

    /* U+0034 "4" */
    0xc0, 0x60, 0x30, 0x18, 0xc, 0x67, 0xff, 0xfe, 0xc, 0x6, 0x0,

    /* U+0035 "5" */
    0x3f, 0x9f, 0xf0, 0x18, 0xf, 0xf8, 0xc, 0x7, 0xfc, 0xfe, 0x0,

    /* U+0036 "6" */
    0x3f, 0x9f, 0xf0, 0x18, 0xf, 0xfe, 0xf, 0x7, 0xfc, 0xfe, 0x0,

    /* U+0037 "7" */
    0xfe, 0x7f, 0x0, 0x60, 0x30, 0x18, 0xc, 0x6, 0x3, 0x1, 0x80,

    /* U+0038 "8" */
    0x3f, 0x9f, 0xf0, 0x78, 0x3f, 0xfe, 0xf, 0x7, 0xfc, 0xfe, 0x0,

    /* U+0039 "9" */
    0x3f, 0x9f, 0xf0, 0x78, 0x3f, 0xf8, 0xc, 0x7, 0xfc, 0xfe, 0x0,

    /* U+003A ":" */
    0xf3, 0xc0,

    /* U+003B ";" */
    0x33, 0x3, 0x3c, 0xc0,

    /* U+003C "<" */
    0x3c, 0xc3, 0x30,

    /* U+003E ">" */
    0xc3, 0x3c, 0xc0,

    /* U+003F "?" */
    0xff, 0xfc, 0x18, 0x3f, 0x9f, 0x0, 0x60, 0xc0,

    /* U+0041 "A" */
    0x3f, 0x9f, 0xf0, 0x78, 0x3f, 0xfe, 0xf, 0x7, 0x83, 0xc1, 0x80,

    /* U+0042 "B" */
    0x3f, 0x9f, 0xf0, 0x78, 0x3f, 0xe6, 0xf, 0x7, 0xff, 0xff, 0x80,

    /* U+0043 "C" */
    0x3f, 0x9f, 0xf0, 0x18, 0xc, 0x6, 0x3, 0x1, 0xff, 0xff, 0x80,

    /* U+0044 "D" */
    0xfe, 0x7f, 0x30, 0x78, 0x3c, 0x1e, 0xf, 0x7, 0xff, 0xff, 0x80,

    /* U+0045 "E" */
    0x3f, 0x9f, 0xf0, 0x18, 0xc, 0xe6, 0x3, 0x1, 0xff, 0xff, 0x80,

    /* U+0046 "F" */
    0x3f, 0x9f, 0xf0, 0x18, 0xc, 0xe6, 0x3, 0x1, 0x80, 0xc0, 0x0,

    /* U+0047 "G" */
    0x3f, 0x9f, 0xf0, 0x18, 0xc, 0xfe, 0xf, 0x7, 0xfc, 0xfe, 0x0,

    /* U+0048 "H" */
    0xc1, 0xe0, 0xf0, 0x78, 0x3f, 0xfe, 0xf, 0x7, 0x83, 0xc1, 0x80,

    /* U+0049 "I" */
    0xff, 0xff, 0xc0,

    /* U+004A "J" */
    0xff, 0xcc, 0x63, 0x18, 0xde, 0xf0,

    /* U+004B "K" */
    0xc7, 0x8f, 0x26, 0x4f, 0x19, 0x32, 0x63, 0xc6,

    /* U+004C "L" */
    0xc1, 0x83, 0x6, 0xc, 0x18, 0x30, 0x1f, 0x3e,

    /* U+004D "M" */
    0x3f, 0x9f, 0xf2, 0x79, 0x3c, 0x9e, 0x4f, 0x27, 0x93, 0xc9, 0x80,

    /* U+004E "N" */
    0x3f, 0x9f, 0xf0, 0x78, 0x3c, 0x1e, 0xf, 0x7, 0x83, 0xc1, 0x80,

    /* U+004F "O" */
    0x3f, 0x9f, 0xf0, 0x78, 0x3c, 0x1e, 0xf, 0x7, 0xfc, 0xfe, 0x0,

    /* U+0050 "P" */
    0x3f, 0x9f, 0xf0, 0x78, 0x3f, 0xfe, 0x3, 0x1, 0x80, 0xc0, 0x0,

    /* U+0051 "Q" */
    0x3e, 0x1f, 0x31, 0x98, 0xcc, 0x66, 0x33, 0x19, 0xff, 0xff, 0x80,

    /* U+0052 "R" */
    0x3f, 0x9f, 0xf0, 0x78, 0x3f, 0xe6, 0xf, 0x7, 0x83, 0xc1, 0x80,

    /* U+0053 "S" */
    0x3f, 0x9f, 0xf0, 0x18, 0xf, 0xf8, 0xc, 0x7, 0xff, 0xff, 0x80,

    /* U+0054 "T" */
    0xff, 0xff, 0xc2, 0x1, 0x0, 0x80, 0x40, 0x20, 0x10, 0x8, 0x0,

    /* U+0055 "U" */
    0xc1, 0xe0, 0xf0, 0x78, 0x3c, 0x1e, 0xf, 0x6, 0x7f, 0x3f, 0x80,

    /* U+0056 "V" */
    0xc1, 0xe0, 0xf0, 0x78, 0x3c, 0x1f, 0xbf, 0xde, 0x7c, 0x3e, 0x0,

    /* U+0057 "W" */
    0xc1, 0xe0, 0xf2, 0x79, 0x3c, 0x9e, 0x4f, 0x27, 0xfc, 0xfe, 0x0,

    /* U+0058 "X" */
    0xc1, 0xe0, 0xf0, 0x78, 0x33, 0xe6, 0xf, 0x7, 0x83, 0xc1, 0x80,

    /* U+0059 "Y" */
    0xc1, 0xe0, 0xf0, 0x78, 0x33, 0xe0, 0x40, 0x20, 0x10, 0x8, 0x0,

    /* U+005A "Z" */
    0xfe, 0x7f, 0x0, 0x60, 0x3f, 0xfe, 0x3, 0x0, 0x7f, 0x3f, 0x80,

    /* U+005B "[" */
    0xff, 0xcc, 0xcc, 0xcf, 0xf0,

    /* U+005D "]" */
    0xff, 0x33, 0x33, 0x3f, 0xf0,

    /* U+005E "^" */
    0x31, 0xb3, 0x90,

    /* U+005F "_" */
    0xff, 0xc0,

    /* U+0061 "a" */
    0x3f, 0x9f, 0xf0, 0x78, 0x3f, 0xfe, 0xf, 0x7, 0x83, 0xc1, 0x80,

    /* U+0062 "b" */
    0x3f, 0x9f, 0xf0, 0x78, 0x3f, 0xe6, 0xf, 0x7, 0xff, 0xff, 0x80,

    /* U+0063 "c" */
    0x3f, 0x9f, 0xf0, 0x18, 0xc, 0x6, 0x3, 0x1, 0xff, 0xff, 0x80,

    /* U+0064 "d" */
    0xfe, 0x7f, 0x30, 0x78, 0x3c, 0x1e, 0xf, 0x7, 0xff, 0xff, 0x80,

    /* U+0065 "e" */
    0x3f, 0x9f, 0xf0, 0x18, 0xc, 0xe6, 0x3, 0x1, 0xff, 0xff, 0x80,

    /* U+0066 "f" */
    0x3f, 0x9f, 0xf0, 0x18, 0xc, 0xe6, 0x3, 0x1, 0x80, 0xc0, 0x0,

    /* U+0067 "g" */
    0x3f, 0x9f, 0xf0, 0x18, 0xc, 0xfe, 0xf, 0x7, 0xfc, 0xfe, 0x0,

    /* U+0068 "h" */
    0xc1, 0xe0, 0xf0, 0x78, 0x3f, 0xfe, 0xf, 0x7, 0x83, 0xc1, 0x80,

    /* U+0069 "i" */
    0xff, 0xff, 0xc0,

    /* U+006A "j" */
    0xff, 0xcc, 0x63, 0x18, 0xde, 0xf0,

    /* U+006B "k" */
    0xc7, 0x8f, 0x26, 0x4f, 0x19, 0x32, 0x63, 0xc6,

    /* U+006C "l" */
    0xc1, 0x83, 0x6, 0xc, 0x18, 0x30, 0x1f, 0x3e,

    /* U+006D "m" */
    0x3f, 0x9f, 0xf2, 0x79, 0x3c, 0x9e, 0x4f, 0x27, 0x93, 0xc9, 0x80,

    /* U+006E "n" */
    0x3f, 0x9f, 0xf0, 0x78, 0x3c, 0x1e, 0xf, 0x7, 0x83, 0xc1, 0x80,

    /* U+006F "o" */
    0x3f, 0x9f, 0xf0, 0x78, 0x3c, 0x1e, 0xf, 0x7, 0xfc, 0xfe, 0x0,

    /* U+0070 "p" */
    0x3f, 0x9f, 0xf0, 0x78, 0x3f, 0xfe, 0x3, 0x1, 0x80, 0xc0, 0x0,

    /* U+0071 "q" */
    0x3e, 0x1f, 0x31, 0x98, 0xcc, 0x66, 0x33, 0x19, 0xff, 0xff, 0x80,

    /* U+0072 "r" */
    0x3f, 0x9f, 0xf0, 0x78, 0x3f, 0xe6, 0xf, 0x7, 0x83, 0xc1, 0x80,

    /* U+0073 "s" */
    0x3f, 0x9f, 0xf0, 0x18, 0xf, 0xf8, 0xc, 0x7, 0xff, 0xff, 0x80,

    /* U+0074 "t" */
    0xff, 0xff, 0xc2, 0x1, 0x0, 0x80, 0x40, 0x20, 0x10, 0x8, 0x0,

    /* U+0075 "u" */
    0xc1, 0xe0, 0xf0, 0x78, 0x3c, 0x1e, 0xf, 0x6, 0x7f, 0x3f, 0x80,

    /* U+0076 "v" */
    0xc1, 0xe0, 0xf0, 0x78, 0x3c, 0x1f, 0xbf, 0xde, 0x7c, 0x3e, 0x0,

    /* U+0077 "w" */
    0xc1, 0xe0, 0xf2, 0x79, 0x3c, 0x9e, 0x4f, 0x27, 0xfc, 0xfe, 0x0,

    /* U+0078 "x" */
    0xc1, 0xe0, 0xf0, 0x78, 0x33, 0xe6, 0xf, 0x7, 0x83, 0xc1, 0x80,

    /* U+0079 "y" */
    0xc1, 0xe0, 0xf0, 0x78, 0x33, 0xe0, 0x40, 0x20, 0x10, 0x8, 0x0,

    /* U+007A "z" */
    0xfe, 0x7f, 0x0, 0x60, 0x3f, 0xfe, 0x3, 0x0, 0x7f, 0x3f, 0x80,

    /* U+007C "|" */
    0xff, 0xff, 0xc0};

/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 84, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 56, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 4, .adv_w = 112, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 7, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 18, .adv_w = 140, .box_w = 5, .box_h = 9, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 24, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 35, .adv_w = 56, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 36, .adv_w = 84, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 41, .adv_w = 84, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 46, .adv_w = 112, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 50, .adv_w = 84, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 52, .adv_w = 112, .box_w = 5, .box_h = 2, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 54, .adv_w = 56, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 55, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 66, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 77, .adv_w = 56, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 80, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 91, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 102, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 113, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 124, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 135, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 146, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 157, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 168, .adv_w = 56, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 170, .adv_w = 84, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 174, .adv_w = 84, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 177, .adv_w = 84, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 180, .adv_w = 140, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 188, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 199, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 210, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 221, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 232, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 243, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 254, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 265, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 276, .adv_w = 56, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 279, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 285, .adv_w = 140, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 293, .adv_w = 140, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 301, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 312, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 323, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 334, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 345, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 356, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 367, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 378, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 389, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 400, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 411, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 422, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 433, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 444, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 455, .adv_w = 84, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 460, .adv_w = 84, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 465, .adv_w = 112, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 468, .adv_w = 112, .box_w = 5, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 470, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 481, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 492, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 503, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 514, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 525, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 536, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 547, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 558, .adv_w = 56, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 561, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 567, .adv_w = 140, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 575, .adv_w = 140, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 583, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 594, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 605, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 616, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 627, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 638, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 649, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 660, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 671, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 682, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 693, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 704, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 715, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 726, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 737, .adv_w = 56, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0}};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint8_t glyph_id_ofs_list_0[] = {
    0,  1,  2,  3,  4,  5,  0,  6,  7,  8,  0,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 0, 27,
    28, 0,  29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 0, 56,
    57, 58, 0,  59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 0, 85};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] = {{.range_start       = 32,
                                                .range_length      = 93,
                                                .glyph_id_start    = 1,
                                                .unicode_list      = NULL,
                                                .glyph_id_ofs_list = glyph_id_ofs_list_0,
                                                .list_length       = 93,
                                                .type              = LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL}};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

    #if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static lv_font_fmt_txt_glyph_cache_t cache;
    #endif

    #if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
    #else
static lv_font_fmt_txt_dsc_t font_dsc = {
    #endif
    .glyph_bitmap  = glyph_bitmap,
    .glyph_dsc     = glyph_dsc,
    .cmaps         = cmaps,
    .kern_dsc      = NULL,
    .kern_scale    = 0,
    .cmap_num      = 1,
    .bpp           = 1,
    .kern_classes  = 0,
    .bitmap_format = 0,
    #if LVGL_VERSION_MAJOR == 8
    .cache = &cache
    #endif
};

    /*-----------------
     *  PUBLIC FONT
     *----------------*/

    /*Initialize a public general font descriptor*/
    #if LVGL_VERSION_MAJOR >= 8
const lv_font_t cyberphont3b_14 = {
    #else
lv_font_t cyberphont3b_14 = {
    #endif
    .get_glyph_dsc    = lv_font_get_glyph_dsc_fmt_txt, /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height      = 9,                             /*The maximum line height required by the font*/
    .base_line        = 0,                             /*Baseline measured from the bottom of the line*/
    #if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
    #endif
    #if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position  = 1,
    .underline_thickness = 1,
    #endif
    .dsc = &font_dsc, /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
    #if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
    #endif
    .user_data = NULL,
};

#endif /*#if CYBERPHONT3B_14*/
