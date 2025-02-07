/*******************************************************************************
 * Size: 12 px
 * Bpp: 1
 * Opts: --bpp 1 --size 12 --no-compress --no-prefilter --font /Users/dwarkentin/Library/CloudStorage/GoogleDrive-ev0rtex@gmail.com/.shortcut-targets-by-id/1M-9uKjsqE0vKNDFWxBemLGBB4Gobi9fB/Saintcon 2024 Badge/Assets/cyberphont 3.0/Cyberphont 3.0 B.ttf -r 0x20-0x7C --format lvgl -o /Users/dwarkentin/src/projects/Saintcon2024Dev/badge-firmware/components/ui/fonts/cyberphont3b_12.c --force-fast-kern-format
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

#ifndef CYBERPHONT3B_12
#define CYBERPHONT3B_12 1
#endif

#if CYBERPHONT3B_12

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xcf, 0x3,

    /* U+0022 "\"" */
    0xde, 0xc0,

    /* U+0023 "#" */
    0x24, 0x0, 0xff, 0x24, 0x0, 0xff, 0x0, 0x24,

    /* U+0024 "$" */
    0x20, 0xe, 0xf0, 0x3, 0xb0, 0x8,

    /* U+0025 "%" */
    0xc3, 0x0, 0x4, 0x18, 0x0, 0x20, 0x0, 0xc3,

    /* U+0027 "'" */
    0xf0,

    /* U+0028 "(" */
    0x23, 0x61, 0x81,

    /* U+0029 ")" */
    0xc0, 0x90, 0x46,

    /* U+002B "+" */
    0x20, 0xe, 0xc0, 0x20,

    /* U+002C "," */
    0x23, 0x0,

    /* U+002D "-" */
    0xec,

    /* U+002E "." */
    0xc0,

    /* U+002F "/" */
    0x3, 0x0, 0x4, 0x18, 0x0, 0x20, 0x0, 0xc0,

    /* U+0030 "0" */
    0x3f, 0x0, 0xc7, 0xdb, 0x0, 0xe3, 0x0, 0xfc,

    /* U+0031 "1" */
    0xcf, 0x33,

    /* U+0032 "2" */
    0xfc, 0x0, 0x3, 0xff, 0x0, 0xc0, 0x0, 0xff,

    /* U+0033 "3" */
    0xfc, 0x0, 0x3, 0xff, 0x0, 0x3, 0x0, 0xff,

    /* U+0034 "4" */
    0xc0, 0x0, 0xc0, 0xc4, 0x0, 0xff, 0x0, 0x4,

    /* U+0035 "5" */
    0x3f, 0x0, 0xc0, 0xff, 0x0, 0x3, 0x0, 0xfc,

    /* U+0036 "6" */
    0x3f, 0x0, 0xc0, 0xff, 0x0, 0xc3, 0x0, 0xfc,

    /* U+0037 "7" */
    0xfc, 0x0, 0x3, 0x3, 0x0, 0x3, 0x0, 0x3,

    /* U+0038 "8" */
    0x3f, 0x0, 0xc3, 0xff, 0x0, 0xc3, 0x0, 0xfc,

    /* U+0039 "9" */
    0x3f, 0x0, 0xc3, 0xff, 0x0, 0x3, 0x0, 0xfc,

    /* U+003A ":" */
    0xc0, 0xc0,

    /* U+003B ";" */
    0x20, 0x11, 0x80,

    /* U+003C "<" */
    0x23, 0x2,

    /* U+003E ">" */
    0xc0, 0x8c,

    /* U+003F "?" */
    0xfc, 0x0, 0x7e, 0x0, 0x0, 0x30,

    /* U+0041 "A" */
    0x3f, 0x0, 0xc3, 0xff, 0x0, 0xc3, 0x0, 0xc3,

    /* U+0042 "B" */
    0x3f, 0x0, 0xc3, 0xfc, 0x0, 0xc3, 0x0, 0xff,

    /* U+0043 "C" */
    0x3f, 0x0, 0xc0, 0xc0, 0x0, 0xc0, 0x0, 0xff,

    /* U+0044 "D" */
    0xfc, 0x0, 0xc3, 0xc3, 0x0, 0xc3, 0x0, 0xff,

    /* U+0045 "E" */
    0x3f, 0x0, 0xc0, 0xdc, 0x0, 0xc0, 0x0, 0xff,

    /* U+0046 "F" */
    0x3f, 0x0, 0xc0, 0xdc, 0x0, 0xc0, 0x0, 0xc0,

    /* U+0047 "G" */
    0x3f, 0x0, 0xc0, 0xdf, 0x0, 0xc3, 0x0, 0xfc,

    /* U+0048 "H" */
    0xc3, 0x0, 0xc3, 0xff, 0x0, 0xc3, 0x0, 0xc3,

    /* U+0049 "I" */
    0xcf, 0x33,

    /* U+004A "J" */
    0xec, 0x2, 0x8, 0x0, 0x80, 0x38,

    /* U+004B "K" */
    0xc4, 0xd, 0xb8, 0x3, 0x60, 0x31,

    /* U+004C "L" */
    0xc0, 0xc, 0x30, 0x3, 0x0, 0xf,

    /* U+004D "M" */
    0x3f, 0x0, 0xdb, 0xdb, 0x0, 0xdb, 0x0, 0xdb,

    /* U+004E "N" */
    0x3f, 0x0, 0xc3, 0xc3, 0x0, 0xc3, 0x0, 0xc3,

    /* U+004F "O" */
    0x3f, 0x0, 0xc3, 0xc3, 0x0, 0xc3, 0x0, 0xfc,

    /* U+0050 "P" */
    0x3f, 0x0, 0xc3, 0xff, 0x0, 0xc0, 0x0, 0xc0,

    /* U+0051 "Q" */
    0x3c, 0x0, 0xc4, 0xc4, 0x0, 0xc4, 0x0, 0xff,

    /* U+0052 "R" */
    0x3f, 0x0, 0xc3, 0xfc, 0x0, 0xc3, 0x0, 0xc3,

    /* U+0053 "S" */
    0x3f, 0x0, 0xc0, 0xff, 0x0, 0x3, 0x0, 0xff,

    /* U+0054 "T" */
    0xff, 0x0, 0x18, 0x18, 0x0, 0x18, 0x0, 0x18,

    /* U+0055 "U" */
    0xc3, 0x0, 0xc3, 0xc3, 0x0, 0xc3, 0x0, 0x3f,

    /* U+0056 "V" */
    0xc3, 0x0, 0xc3, 0xc3, 0x0, 0xe7, 0x0, 0x3c,

    /* U+0057 "W" */
    0xc3, 0x0, 0xdb, 0xdb, 0x0, 0xdb, 0x0, 0xfc,

    /* U+0058 "X" */
    0xc3, 0x0, 0xc3, 0x3c, 0x0, 0xc3, 0x0, 0xc3,

    /* U+0059 "Y" */
    0xc3, 0x0, 0xc3, 0x3c, 0x0, 0x18, 0x0, 0x18,

    /* U+005A "Z" */
    0xfc, 0x0, 0x3, 0xff, 0x0, 0xc0, 0x0, 0x3f,

    /* U+005B "[" */
    0xe3, 0x61, 0x87,

    /* U+005D "]" */
    0xe0, 0x90, 0x47,

    /* U+005E "^" */
    0x20, 0xc, 0xc0,

    /* U+005F "_" */
    0xec,

    /* U+0061 "a" */
    0x3f, 0x0, 0xc3, 0xff, 0x0, 0xc3, 0x0, 0xc3,

    /* U+0062 "b" */
    0x3f, 0x0, 0xc3, 0xfc, 0x0, 0xc3, 0x0, 0xff,

    /* U+0063 "c" */
    0x3f, 0x0, 0xc0, 0xc0, 0x0, 0xc0, 0x0, 0xff,

    /* U+0064 "d" */
    0xfc, 0x0, 0xc3, 0xc3, 0x0, 0xc3, 0x0, 0xff,

    /* U+0065 "e" */
    0x3f, 0x0, 0xc0, 0xdc, 0x0, 0xc0, 0x0, 0xff,

    /* U+0066 "f" */
    0x3f, 0x0, 0xc0, 0xdc, 0x0, 0xc0, 0x0, 0xc0,

    /* U+0067 "g" */
    0x3f, 0x0, 0xc0, 0xdf, 0x0, 0xc3, 0x0, 0xfc,

    /* U+0068 "h" */
    0xc3, 0x0, 0xc3, 0xff, 0x0, 0xc3, 0x0, 0xc3,

    /* U+0069 "i" */
    0xcf, 0x33,

    /* U+006A "j" */
    0xec, 0x2, 0x8, 0x0, 0x80, 0x38,

    /* U+006B "k" */
    0xc4, 0xd, 0xb8, 0x3, 0x60, 0x31,

    /* U+006C "l" */
    0xc0, 0xc, 0x30, 0x3, 0x0, 0xf,

    /* U+006D "m" */
    0x3f, 0x0, 0xdb, 0xdb, 0x0, 0xdb, 0x0, 0xdb,

    /* U+006E "n" */
    0x3f, 0x0, 0xc3, 0xc3, 0x0, 0xc3, 0x0, 0xc3,

    /* U+006F "o" */
    0x3f, 0x0, 0xc3, 0xc3, 0x0, 0xc3, 0x0, 0xfc,

    /* U+0070 "p" */
    0x3f, 0x0, 0xc3, 0xff, 0x0, 0xc0, 0x0, 0xc0,

    /* U+0071 "q" */
    0x3c, 0x0, 0xc4, 0xc4, 0x0, 0xc4, 0x0, 0xff,

    /* U+0072 "r" */
    0x3f, 0x0, 0xc3, 0xfc, 0x0, 0xc3, 0x0, 0xc3,

    /* U+0073 "s" */
    0x3f, 0x0, 0xc0, 0xff, 0x0, 0x3, 0x0, 0xff,

    /* U+0074 "t" */
    0xff, 0x0, 0x18, 0x18, 0x0, 0x18, 0x0, 0x18,

    /* U+0075 "u" */
    0xc3, 0x0, 0xc3, 0xc3, 0x0, 0xc3, 0x0, 0x3f,

    /* U+0076 "v" */
    0xc3, 0x0, 0xc3, 0xc3, 0x0, 0xe7, 0x0, 0x3c,

    /* U+0077 "w" */
    0xc3, 0x0, 0xdb, 0xdb, 0x0, 0xdb, 0x0, 0xfc,

    /* U+0078 "x" */
    0xc3, 0x0, 0xc3, 0x3c, 0x0, 0xc3, 0x0, 0xc3,

    /* U+0079 "y" */
    0xc3, 0x0, 0xc3, 0x3c, 0x0, 0x18, 0x0, 0x18,

    /* U+007A "z" */
    0xfc, 0x0, 0x3, 0xff, 0x0, 0xc0, 0x0, 0x3f,

    /* U+007C "|" */
    0xcf, 0x33
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 72, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 96, .box_w = 5, .box_h = 2, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 5, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 13, .adv_w = 120, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 19, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 27, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 28, .adv_w = 72, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 31, .adv_w = 72, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 34, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 38, .adv_w = 72, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 40, .adv_w = 96, .box_w = 6, .box_h = 1, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 41, .adv_w = 48, .box_w = 2, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 42, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 50, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 58, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 60, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 68, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 76, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 84, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 92, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 100, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 108, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 116, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 124, .adv_w = 48, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 126, .adv_w = 72, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 129, .adv_w = 72, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 131, .adv_w = 72, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 133, .adv_w = 120, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 139, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 147, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 155, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 163, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 171, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 179, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 187, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 195, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 203, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 205, .adv_w = 96, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 211, .adv_w = 120, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 217, .adv_w = 120, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 223, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 231, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 239, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 247, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 255, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 263, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 271, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 279, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 287, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 295, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 303, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 311, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 319, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 327, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 335, .adv_w = 72, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 338, .adv_w = 72, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 341, .adv_w = 96, .box_w = 6, .box_h = 3, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 344, .adv_w = 96, .box_w = 6, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 345, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 353, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 361, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 369, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 377, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 385, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 393, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 401, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 409, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 411, .adv_w = 96, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 417, .adv_w = 120, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 423, .adv_w = 120, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 429, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 437, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 445, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 453, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 461, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 469, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 477, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 485, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 493, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 501, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 509, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 517, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 525, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 533, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 541, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint8_t glyph_id_ofs_list_0[] = {
    0, 1, 2, 3, 4, 5, 0, 6,
    7, 8, 0, 9, 10, 11, 12, 13,
    14, 15, 16, 17, 18, 19, 20, 21,
    22, 23, 24, 25, 26, 0, 27, 28,
    0, 29, 30, 31, 32, 33, 34, 35,
    36, 37, 38, 39, 40, 41, 42, 43,
    44, 45, 46, 47, 48, 49, 50, 51,
    52, 53, 54, 55, 0, 56, 57, 58,
    0, 59, 60, 61, 62, 63, 64, 65,
    66, 67, 68, 69, 70, 71, 72, 73,
    74, 75, 76, 77, 78, 79, 80, 81,
    82, 83, 84, 0, 85
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 93, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = glyph_id_ofs_list_0, .list_length = 93, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 0,
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
const lv_font_t cyberphont3b_12 = {
#else
lv_font_t cyberphont3b_12 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 8,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = 1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if CYBERPHONT3B_12*/

