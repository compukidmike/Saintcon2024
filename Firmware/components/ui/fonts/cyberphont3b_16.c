/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: --bpp 1 --size 16 --no-compress --no-prefilter --font /Users/dwarkentin/Library/CloudStorage/GoogleDrive-ev0rtex@gmail.com/.shortcut-targets-by-id/1M-9uKjsqE0vKNDFWxBemLGBB4Gobi9fB/Saintcon 2024 Badge/Assets/cyberphont 3.0/Cyberphont 3.0 B.ttf -r 0x20-0x7C --format lvgl -o /Users/dwarkentin/src/projects/Saintcon2024Dev/badge-firmware/components/ui/fonts/cyberphont3b_16.c --force-fast-kern-format
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

#ifndef CYBERPHONT3B_16
#define CYBERPHONT3B_16 1
#endif

#if CYBERPHONT3B_16

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xf0, 0xf0,

    /* U+0022 "\"" */
    0xcf, 0x3c, 0xf3,

    /* U+0023 "#" */
    0x33, 0xc, 0xcf, 0xff, 0xff, 0x33, 0xc, 0xcf,
    0xff, 0xff, 0x33, 0xc, 0xc0,

    /* U+0024 "$" */
    0x30, 0xcf, 0xff, 0xc3, 0xf, 0xff, 0x30, 0xc0,

    /* U+0025 "%" */
    0xc0, 0xf0, 0x30, 0x30, 0xc, 0xc, 0x3, 0x3,
    0x0, 0xc0, 0xc0, 0xf0, 0x30,

    /* U+0027 "'" */
    0xff,

    /* U+0028 "(" */
    0x33, 0xcc, 0xcc, 0xcc, 0x33,

    /* U+0029 ")" */
    0xcc, 0x33, 0x33, 0x33, 0xcc,

    /* U+002B "+" */
    0x30, 0xcf, 0xff, 0x30, 0xc0,

    /* U+002C "," */
    0x33, 0xcc,

    /* U+002D "-" */
    0xff, 0xf0,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x0, 0xc0, 0x30, 0x30, 0xc, 0xc, 0x3, 0x3,
    0x0, 0xc0, 0xc0, 0x30, 0x0,

    /* U+0030 "0" */
    0x3f, 0xcf, 0xfc, 0x3f, 0xf, 0xcc, 0xf3, 0x3f,
    0xf, 0xc3, 0xff, 0x3f, 0xc0,

    /* U+0031 "1" */
    0xff, 0xff, 0xf0,

    /* U+0032 "2" */
    0xff, 0x3f, 0xc0, 0xc, 0x3, 0xff, 0xff, 0xfc,
    0x3, 0x0, 0xff, 0xff, 0xf0,

    /* U+0033 "3" */
    0xff, 0x3f, 0xc0, 0xc, 0x3, 0xff, 0xff, 0xf0,
    0xc, 0x3, 0xff, 0xff, 0xf0,

    /* U+0034 "4" */
    0xc0, 0x30, 0xc, 0x3, 0x0, 0xc3, 0x30, 0xcf,
    0xff, 0xff, 0x3, 0x0, 0xc0,

    /* U+0035 "5" */
    0x3f, 0xcf, 0xfc, 0x3, 0x0, 0xff, 0xff, 0xf0,
    0xc, 0x3, 0xff, 0x3f, 0xc0,

    /* U+0036 "6" */
    0x3f, 0xcf, 0xfc, 0x3, 0x0, 0xff, 0xff, 0xfc,
    0xf, 0x3, 0xff, 0x3f, 0xc0,

    /* U+0037 "7" */
    0xff, 0x3f, 0xc0, 0xc, 0x3, 0x0, 0xc0, 0x30,
    0xc, 0x3, 0x0, 0xc0, 0x30,

    /* U+0038 "8" */
    0x3f, 0xcf, 0xfc, 0xf, 0x3, 0xff, 0xff, 0xfc,
    0xf, 0x3, 0xff, 0x3f, 0xc0,

    /* U+0039 "9" */
    0x3f, 0xcf, 0xfc, 0xf, 0x3, 0xff, 0xff, 0xf0,
    0xc, 0x3, 0xff, 0x3f, 0xc0,

    /* U+003A ":" */
    0xf0, 0xf0,

    /* U+003B ";" */
    0x33, 0x0, 0x33, 0xcc,

    /* U+003C "<" */
    0x33, 0xcc, 0x33,

    /* U+003E ">" */
    0xcc, 0x33, 0xcc,

    /* U+003F "?" */
    0xff, 0xff, 0x3, 0x3, 0xfc, 0xfc, 0x0, 0x0,
    0xc0, 0xc0,

    /* U+0041 "A" */
    0x3f, 0xcf, 0xfc, 0xf, 0x3, 0xff, 0xff, 0xfc,
    0xf, 0x3, 0xc0, 0xf0, 0x30,

    /* U+0042 "B" */
    0x3f, 0xcf, 0xfc, 0xf, 0x3, 0xff, 0x3f, 0xcc,
    0xf, 0x3, 0xff, 0xff, 0xf0,

    /* U+0043 "C" */
    0x3f, 0xcf, 0xfc, 0x3, 0x0, 0xc0, 0x30, 0xc,
    0x3, 0x0, 0xff, 0xff, 0xf0,

    /* U+0044 "D" */
    0xff, 0x3f, 0xcc, 0xf, 0x3, 0xc0, 0xf0, 0x3c,
    0xf, 0x3, 0xff, 0xff, 0xf0,

    /* U+0045 "E" */
    0x3f, 0xcf, 0xfc, 0x3, 0x0, 0xcf, 0x33, 0xcc,
    0x3, 0x0, 0xff, 0xff, 0xf0,

    /* U+0046 "F" */
    0x3f, 0xcf, 0xfc, 0x3, 0x0, 0xcf, 0x33, 0xcc,
    0x3, 0x0, 0xc0, 0x30, 0x0,

    /* U+0047 "G" */
    0x3f, 0xcf, 0xfc, 0x3, 0x0, 0xcf, 0xf3, 0xfc,
    0xf, 0x3, 0xff, 0x3f, 0xc0,

    /* U+0048 "H" */
    0xc0, 0xf0, 0x3c, 0xf, 0x3, 0xff, 0xff, 0xfc,
    0xf, 0x3, 0xc0, 0xf0, 0x30,

    /* U+0049 "I" */
    0xff, 0xff, 0xf0,

    /* U+004A "J" */
    0xff, 0xf3, 0xc, 0x30, 0xc3, 0xc, 0xf3, 0xc0,

    /* U+004B "K" */
    0xc3, 0xc3, 0xcc, 0xcc, 0xf0, 0xf0, 0xcc, 0xcc,
    0xc3, 0xc3,

    /* U+004C "L" */
    0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
    0x3f, 0x3f,

    /* U+004D "M" */
    0x3f, 0xcf, 0xfc, 0xcf, 0x33, 0xcc, 0xf3, 0x3c,
    0xcf, 0x33, 0xcc, 0xf3, 0x30,

    /* U+004E "N" */
    0x3f, 0xcf, 0xfc, 0xf, 0x3, 0xc0, 0xf0, 0x3c,
    0xf, 0x3, 0xc0, 0xf0, 0x30,

    /* U+004F "O" */
    0x3f, 0xcf, 0xfc, 0xf, 0x3, 0xc0, 0xf0, 0x3c,
    0xf, 0x3, 0xff, 0x3f, 0xc0,

    /* U+0050 "P" */
    0x3f, 0xcf, 0xfc, 0xf, 0x3, 0xff, 0xff, 0xfc,
    0x3, 0x0, 0xc0, 0x30, 0x0,

    /* U+0051 "Q" */
    0x3f, 0xf, 0xcc, 0x33, 0xc, 0xc3, 0x30, 0xcc,
    0x33, 0xc, 0xff, 0xff, 0xf0,

    /* U+0052 "R" */
    0x3f, 0xcf, 0xfc, 0xf, 0x3, 0xff, 0x3f, 0xcc,
    0xf, 0x3, 0xc0, 0xf0, 0x30,

    /* U+0053 "S" */
    0x3f, 0xcf, 0xfc, 0x3, 0x0, 0xff, 0xff, 0xf0,
    0xc, 0x3, 0xff, 0xff, 0xf0,

    /* U+0054 "T" */
    0xff, 0xff, 0xf0, 0xc0, 0x30, 0xc, 0x3, 0x0,
    0xc0, 0x30, 0xc, 0x3, 0x0,

    /* U+0055 "U" */
    0xc0, 0xf0, 0x3c, 0xf, 0x3, 0xc0, 0xf0, 0x3c,
    0xf, 0x3, 0x3f, 0xcf, 0xf0,

    /* U+0056 "V" */
    0xc0, 0xf0, 0x3c, 0xf, 0x3, 0xc0, 0xf0, 0x3f,
    0x3f, 0xcf, 0x3f, 0xf, 0xc0,

    /* U+0057 "W" */
    0xc0, 0xf0, 0x3c, 0xcf, 0x33, 0xcc, 0xf3, 0x3c,
    0xcf, 0x33, 0xff, 0x3f, 0xc0,

    /* U+0058 "X" */
    0xc0, 0xf0, 0x3c, 0xf, 0x3, 0x3f, 0xf, 0xcc,
    0xf, 0x3, 0xc0, 0xf0, 0x30,

    /* U+0059 "Y" */
    0xc0, 0xf0, 0x3c, 0xf, 0x3, 0x3f, 0xf, 0xc0,
    0xc0, 0x30, 0xc, 0x3, 0x0,

    /* U+005A "Z" */
    0xff, 0x3f, 0xc0, 0xc, 0x3, 0xff, 0xff, 0xfc,
    0x3, 0x0, 0x3f, 0xcf, 0xf0,

    /* U+005B "[" */
    0xff, 0xcc, 0xcc, 0xcc, 0xff,

    /* U+005D "]" */
    0xff, 0x33, 0x33, 0x33, 0xff,

    /* U+005E "^" */
    0x30, 0xcc, 0xf3,

    /* U+005F "_" */
    0xff, 0xf0,

    /* U+0061 "a" */
    0x3f, 0xcf, 0xfc, 0xf, 0x3, 0xff, 0xff, 0xfc,
    0xf, 0x3, 0xc0, 0xf0, 0x30,

    /* U+0062 "b" */
    0x3f, 0xcf, 0xfc, 0xf, 0x3, 0xff, 0x3f, 0xcc,
    0xf, 0x3, 0xff, 0xff, 0xf0,

    /* U+0063 "c" */
    0x3f, 0xcf, 0xfc, 0x3, 0x0, 0xc0, 0x30, 0xc,
    0x3, 0x0, 0xff, 0xff, 0xf0,

    /* U+0064 "d" */
    0xff, 0x3f, 0xcc, 0xf, 0x3, 0xc0, 0xf0, 0x3c,
    0xf, 0x3, 0xff, 0xff, 0xf0,

    /* U+0065 "e" */
    0x3f, 0xcf, 0xfc, 0x3, 0x0, 0xcf, 0x33, 0xcc,
    0x3, 0x0, 0xff, 0xff, 0xf0,

    /* U+0066 "f" */
    0x3f, 0xcf, 0xfc, 0x3, 0x0, 0xcf, 0x33, 0xcc,
    0x3, 0x0, 0xc0, 0x30, 0x0,

    /* U+0067 "g" */
    0x3f, 0xcf, 0xfc, 0x3, 0x0, 0xcf, 0xf3, 0xfc,
    0xf, 0x3, 0xff, 0x3f, 0xc0,

    /* U+0068 "h" */
    0xc0, 0xf0, 0x3c, 0xf, 0x3, 0xff, 0xff, 0xfc,
    0xf, 0x3, 0xc0, 0xf0, 0x30,

    /* U+0069 "i" */
    0xff, 0xff, 0xf0,

    /* U+006A "j" */
    0xff, 0xf3, 0xc, 0x30, 0xc3, 0xc, 0xf3, 0xc0,

    /* U+006B "k" */
    0xc3, 0xc3, 0xcc, 0xcc, 0xf0, 0xf0, 0xcc, 0xcc,
    0xc3, 0xc3,

    /* U+006C "l" */
    0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
    0x3f, 0x3f,

    /* U+006D "m" */
    0x3f, 0xcf, 0xfc, 0xcf, 0x33, 0xcc, 0xf3, 0x3c,
    0xcf, 0x33, 0xcc, 0xf3, 0x30,

    /* U+006E "n" */
    0x3f, 0xcf, 0xfc, 0xf, 0x3, 0xc0, 0xf0, 0x3c,
    0xf, 0x3, 0xc0, 0xf0, 0x30,

    /* U+006F "o" */
    0x3f, 0xcf, 0xfc, 0xf, 0x3, 0xc0, 0xf0, 0x3c,
    0xf, 0x3, 0xff, 0x3f, 0xc0,

    /* U+0070 "p" */
    0x3f, 0xcf, 0xfc, 0xf, 0x3, 0xff, 0xff, 0xfc,
    0x3, 0x0, 0xc0, 0x30, 0x0,

    /* U+0071 "q" */
    0x3f, 0xf, 0xcc, 0x33, 0xc, 0xc3, 0x30, 0xcc,
    0x33, 0xc, 0xff, 0xff, 0xf0,

    /* U+0072 "r" */
    0x3f, 0xcf, 0xfc, 0xf, 0x3, 0xff, 0x3f, 0xcc,
    0xf, 0x3, 0xc0, 0xf0, 0x30,

    /* U+0073 "s" */
    0x3f, 0xcf, 0xfc, 0x3, 0x0, 0xff, 0xff, 0xf0,
    0xc, 0x3, 0xff, 0xff, 0xf0,

    /* U+0074 "t" */
    0xff, 0xff, 0xf0, 0xc0, 0x30, 0xc, 0x3, 0x0,
    0xc0, 0x30, 0xc, 0x3, 0x0,

    /* U+0075 "u" */
    0xc0, 0xf0, 0x3c, 0xf, 0x3, 0xc0, 0xf0, 0x3c,
    0xf, 0x3, 0x3f, 0xcf, 0xf0,

    /* U+0076 "v" */
    0xc0, 0xf0, 0x3c, 0xf, 0x3, 0xc0, 0xf0, 0x3f,
    0x3f, 0xcf, 0x3f, 0xf, 0xc0,

    /* U+0077 "w" */
    0xc0, 0xf0, 0x3c, 0xcf, 0x33, 0xcc, 0xf3, 0x3c,
    0xcf, 0x33, 0xff, 0x3f, 0xc0,

    /* U+0078 "x" */
    0xc0, 0xf0, 0x3c, 0xf, 0x3, 0x3f, 0xf, 0xcc,
    0xf, 0x3, 0xc0, 0xf0, 0x30,

    /* U+0079 "y" */
    0xc0, 0xf0, 0x3c, 0xf, 0x3, 0x3f, 0xf, 0xc0,
    0xc0, 0x30, 0xc, 0x3, 0x0,

    /* U+007A "z" */
    0xff, 0x3f, 0xc0, 0xc, 0x3, 0xff, 0xff, 0xfc,
    0x3, 0x0, 0x3f, 0xcf, 0xf0,

    /* U+007C "|" */
    0xff, 0xff, 0xf0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 96, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 64, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 4, .adv_w = 128, .box_w = 6, .box_h = 4, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 7, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 20, .adv_w = 160, .box_w = 6, .box_h = 10, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 28, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 41, .adv_w = 64, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 42, .adv_w = 96, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 47, .adv_w = 96, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 52, .adv_w = 128, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 57, .adv_w = 96, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 59, .adv_w = 128, .box_w = 6, .box_h = 2, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 61, .adv_w = 64, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 62, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 75, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 88, .adv_w = 64, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 91, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 104, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 117, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 130, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 143, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 156, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 169, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 182, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 195, .adv_w = 64, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 197, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 201, .adv_w = 96, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 204, .adv_w = 96, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 207, .adv_w = 160, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 217, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 230, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 243, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 256, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 269, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 282, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 295, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 308, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 321, .adv_w = 64, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 324, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 332, .adv_w = 160, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 342, .adv_w = 160, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 352, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 365, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 378, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 391, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 404, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 417, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 430, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 443, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 456, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 469, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 482, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 495, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 508, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 521, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 534, .adv_w = 96, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 539, .adv_w = 96, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 544, .adv_w = 128, .box_w = 6, .box_h = 4, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 547, .adv_w = 128, .box_w = 6, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 549, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 562, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 575, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 588, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 601, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 614, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 627, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 640, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 653, .adv_w = 64, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 656, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 664, .adv_w = 160, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 674, .adv_w = 160, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 684, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 697, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 710, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 723, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 736, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 749, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 762, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 775, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 788, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 801, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 814, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 827, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 840, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 853, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 866, .adv_w = 64, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = 0}
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
const lv_font_t cyberphont3b_16 = {
#else
lv_font_t cyberphont3b_16 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 10,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = 2,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if CYBERPHONT3B_16*/

