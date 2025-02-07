/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: --bpp 1 --size 16 --no-compress --no-prefilter --font /Users/dwarkentin/Downloads/bm_mini/BMmini.TTF -r 0x20-0x7E --format lvgl -o /Users/dwarkentin/src/projects/Saintcon2024Dev/badge-firmware/components/ui/fonts/bm_mini_16.c --force-fast-kern-format
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

#ifndef BM_MINI_16
#define BM_MINI_16 1
#endif

#if BM_MINI_16

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xff, 0xf,

    /* U+0022 "\"" */
    0xcf, 0x3c, 0xf3,

    /* U+0023 "#" */
    0x33, 0xc, 0xcf, 0xff, 0xff, 0x33, 0xc, 0xcf,
    0xff, 0xff, 0x33, 0xc, 0xc0,

    /* U+0024 "$" */
    0x33, 0xc, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xf3,
    0xf0, 0xfc, 0xc, 0x3, 0x0,

    /* U+0025 "%" */
    0x30, 0xc0, 0xc3, 0xc, 0xcc, 0x33, 0x30, 0x33,
    0x0, 0xcc, 0x0, 0x33, 0x0, 0xcc, 0xc, 0xcc,
    0x33, 0x30, 0xc3, 0x3, 0xc,

    /* U+0026 "&" */
    0x30, 0xc, 0xc, 0xc3, 0x30, 0x30, 0xcc, 0x3c,
    0xf3, 0x3c, 0xc3, 0x30, 0xc3, 0xcc, 0xf3,

    /* U+0027 "'" */
    0xff,

    /* U+0028 "(" */
    0x33, 0xcc, 0xcc, 0xcc, 0xcc, 0x33,

    /* U+0029 ")" */
    0xcc, 0x33, 0x33, 0x33, 0x33, 0xcc,

    /* U+002A "*" */
    0xcf, 0x33, 0xc, 0xcf, 0x30,

    /* U+002B "+" */
    0x30, 0xcf, 0xff, 0x30, 0xc0,

    /* U+002C "," */
    0x33, 0xcc,

    /* U+002D "-" */
    0xff, 0xf0,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0xc, 0x30, 0xc3, 0x30, 0xc3, 0xc, 0xc3, 0xc,
    0x30,

    /* U+0030 "0" */
    0x3c, 0x3c, 0xc3, 0xc3, 0xcf, 0xcf, 0xf3, 0xf3,
    0xc3, 0xc3, 0x3c, 0x3c,

    /* U+0031 "1" */
    0x33, 0xff, 0x33, 0x33, 0x33, 0x33,

    /* U+0032 "2" */
    0x3c, 0x3c, 0xc3, 0xc3, 0xc, 0xc, 0x30, 0x30,
    0xc0, 0xc0, 0xff, 0xff,

    /* U+0033 "3" */
    0x3c, 0x3c, 0xc3, 0xc3, 0xc, 0xc, 0x3, 0x3,
    0xc3, 0xc3, 0x3c, 0x3c,

    /* U+0034 "4" */
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xff, 0xff,
    0xc, 0xc, 0xc, 0xc,

    /* U+0035 "5" */
    0xff, 0xff, 0xc0, 0xc0, 0xfc, 0xfc, 0x3, 0x3,
    0x3, 0x3, 0xfc, 0xfc,

    /* U+0036 "6" */
    0x3c, 0x3c, 0xc3, 0xc3, 0xfc, 0xfc, 0xc3, 0xc3,
    0xc3, 0xc3, 0x3c, 0x3c,

    /* U+0037 "7" */
    0xff, 0xff, 0xc3, 0xc3, 0xc, 0xc, 0xc, 0xc,
    0x30, 0x30, 0x30, 0x30,

    /* U+0038 "8" */
    0x3c, 0x3c, 0xc3, 0xc3, 0x3c, 0x3c, 0xc3, 0xc3,
    0xc3, 0xc3, 0x3c, 0x3c,

    /* U+0039 "9" */
    0x3c, 0x3c, 0xc3, 0xc3, 0xc3, 0xc3, 0x3f, 0x3f,
    0xc3, 0xc3, 0x3c, 0x3c,

    /* U+003A ":" */
    0xf0, 0xf,

    /* U+003B ";" */
    0xf0, 0xf, 0xf0,

    /* U+003C "<" */
    0xc, 0x33, 0xc, 0xc3, 0x3, 0xc, 0xc, 0x30,

    /* U+003D "=" */
    0xff, 0xf0, 0x0, 0xff, 0xf0,

    /* U+003E ">" */
    0xc3, 0x3, 0xc, 0xc, 0x33, 0xc, 0xc3, 0x0,

    /* U+003F "?" */
    0x3c, 0x3c, 0xc3, 0xc3, 0x3, 0x3, 0x3c, 0x3c,
    0x0, 0x0, 0x30, 0x30,

    /* U+0040 "@" */
    0x3f, 0xf, 0xcc, 0xf, 0x3, 0xcf, 0xf3, 0xfc,
    0xcf, 0x33, 0xcf, 0xf3, 0xf3, 0x0, 0xc0,

    /* U+0041 "A" */
    0x3f, 0x3f, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xff,
    0xc3, 0xc3, 0xc3, 0xc3,

    /* U+0042 "B" */
    0xfc, 0xfc, 0xc3, 0xc3, 0xfc, 0xfc, 0xc3, 0xc3,
    0xc3, 0xc3, 0xfc, 0xfc,

    /* U+0043 "C" */
    0x3f, 0x3f, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
    0xc0, 0xc0, 0x3f, 0x3f,

    /* U+0044 "D" */
    0xfc, 0xfc, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3,
    0xc3, 0xc3, 0xfc, 0xfc,

    /* U+0045 "E" */
    0x3f, 0x3f, 0xc0, 0xc0, 0xfc, 0xfc, 0xc0, 0xc0,
    0xc0, 0xc0, 0xff, 0xff,

    /* U+0046 "F" */
    0x3f, 0x3f, 0xc0, 0xc0, 0xfc, 0xfc, 0xc0, 0xc0,
    0xc0, 0xc0, 0xc0, 0xc0,

    /* U+0047 "G" */
    0x3f, 0x3f, 0xc0, 0xc0, 0xcf, 0xcf, 0xc3, 0xc3,
    0xc3, 0xc3, 0x3f, 0x3f,

    /* U+0048 "H" */
    0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xff, 0xc3, 0xc3,
    0xc3, 0xc3, 0xc3, 0xc3,

    /* U+0049 "I" */
    0xff, 0xf3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xcf,
    0xff,

    /* U+004A "J" */
    0xf, 0xf, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
    0x3, 0x3, 0xfc, 0xfc,

    /* U+004B "K" */
    0xc3, 0xc3, 0xc3, 0xc3, 0xfc, 0xfc, 0xc3, 0xc3,
    0xc3, 0xc3, 0xc3, 0xc3,

    /* U+004C "L" */
    0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
    0xc0, 0xc0, 0x3f, 0x3f,

    /* U+004D "M" */
    0xc0, 0xf0, 0x3f, 0x3f, 0xcf, 0xcc, 0xf3, 0x3c,
    0xf, 0x3, 0xc0, 0xf0, 0x3c, 0xf, 0x3,

    /* U+004E "N" */
    0xc3, 0xc3, 0xf3, 0xf3, 0xcf, 0xcf, 0xc3, 0xc3,
    0xc3, 0xc3, 0xc3, 0xc3,

    /* U+004F "O" */
    0x3c, 0x3c, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3,
    0xc3, 0xc3, 0x3c, 0x3c,

    /* U+0050 "P" */
    0xfc, 0xfc, 0xc3, 0xc3, 0xc3, 0xc3, 0xfc, 0xfc,
    0xc0, 0xc0, 0xc0, 0xc0,

    /* U+0051 "Q" */
    0x3c, 0x3c, 0xc3, 0xc3, 0xc3, 0xc3, 0xf3, 0xf3,
    0xcf, 0xcf, 0x3f, 0x3f,

    /* U+0052 "R" */
    0xfc, 0xfc, 0xc3, 0xc3, 0xc3, 0xc3, 0xfc, 0xfc,
    0xc3, 0xc3, 0xc3, 0xc3,

    /* U+0053 "S" */
    0x3f, 0x3f, 0xc0, 0xc0, 0x30, 0x30, 0xc, 0xc,
    0x3, 0x3, 0xfc, 0xfc,

    /* U+0054 "T" */
    0xff, 0xff, 0xf0, 0xc0, 0x30, 0xc, 0x3, 0x0,
    0xc0, 0x30, 0xc, 0x3, 0x0, 0xc0, 0x30,

    /* U+0055 "U" */
    0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3,
    0xc3, 0xc3, 0x3c, 0x3c,

    /* U+0056 "V" */
    0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xcc, 0xcc,
    0xf0, 0xf0, 0xc0, 0xc0,

    /* U+0057 "W" */
    0xc0, 0xf0, 0x3c, 0xcf, 0x33, 0xcc, 0xf3, 0x3c,
    0xcf, 0x33, 0xcc, 0xf3, 0x33, 0x30, 0xcc,

    /* U+0058 "X" */
    0xc3, 0xc3, 0xc3, 0xc3, 0x3c, 0x3c, 0x3c, 0x3c,
    0xc3, 0xc3, 0xc3, 0xc3,

    /* U+0059 "Y" */
    0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x3f, 0x3f,
    0x3, 0x3, 0x3c, 0x3c,

    /* U+005A "Z" */
    0xff, 0xff, 0x3, 0x3, 0xc, 0xc, 0x30, 0x30,
    0xc0, 0xc0, 0xff, 0xff,

    /* U+005B "[" */
    0xff, 0xcc, 0xcc, 0xcc, 0xcc, 0xff,

    /* U+005C "\\" */
    0xc3, 0xc, 0x30, 0x30, 0xc3, 0xc, 0xc, 0x30,
    0xc3,

    /* U+005D "]" */
    0xff, 0x33, 0x33, 0x33, 0x33, 0xff,

    /* U+005E "^" */
    0x30, 0xcc, 0xf3,

    /* U+005F "_" */
    0xff, 0xff,

    /* U+0061 "a" */
    0x3f, 0x3f, 0xc3, 0xc3, 0xc3, 0xc3, 0x3f, 0x3f,

    /* U+0062 "b" */
    0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xfc, 0xc3, 0xc3,
    0xc3, 0xc3, 0xfc, 0xfc,

    /* U+0063 "c" */
    0x3f, 0x3f, 0xc0, 0xc0, 0xc0, 0xc0, 0x3f, 0x3f,

    /* U+0064 "d" */
    0x3, 0x3, 0x3, 0x3, 0x3f, 0x3f, 0xc3, 0xc3,
    0xc3, 0xc3, 0x3f, 0x3f,

    /* U+0065 "e" */
    0x3c, 0x3c, 0xcf, 0xcf, 0xf0, 0xf0, 0x3f, 0x3f,

    /* U+0066 "f" */
    0x3c, 0xfc, 0x30, 0xf3, 0xcc, 0x30, 0xc3, 0xc,
    0x30,

    /* U+0067 "g" */
    0x3f, 0x3f, 0xc3, 0xc3, 0xc3, 0xc3, 0x3f, 0x3f,
    0x3, 0x3, 0x3c, 0x3c,

    /* U+0068 "h" */
    0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xfc, 0xc3, 0xc3,
    0xc3, 0xc3, 0xc3, 0xc3,

    /* U+0069 "i" */
    0xf0, 0xff, 0xff,

    /* U+006A "j" */
    0x33, 0x0, 0x33, 0x33, 0x33, 0x33, 0x33, 0xcc,

    /* U+006B "k" */
    0xc0, 0xc0, 0xc0, 0xc0, 0xc3, 0xc3, 0xfc, 0xfc,
    0xc3, 0xc3, 0xc3, 0xc3,

    /* U+006C "l" */
    0xff, 0xff, 0xff,

    /* U+006D "m" */
    0xff, 0x3f, 0xcc, 0xcf, 0x33, 0xcc, 0xf3, 0x3c,
    0xcf, 0x33,

    /* U+006E "n" */
    0xfc, 0xfc, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3,

    /* U+006F "o" */
    0x3c, 0x3c, 0xc3, 0xc3, 0xc3, 0xc3, 0x3c, 0x3c,

    /* U+0070 "p" */
    0xfc, 0xfc, 0xc3, 0xc3, 0xc3, 0xc3, 0xfc, 0xfc,
    0xc0, 0xc0, 0xc0, 0xc0,

    /* U+0071 "q" */
    0x3f, 0x3f, 0xc3, 0xc3, 0xc3, 0xc3, 0x3f, 0x3f,
    0x3, 0x3, 0x3, 0x3,

    /* U+0072 "r" */
    0xcf, 0xcf, 0xf0, 0xf0, 0xc0, 0xc0, 0xc0, 0xc0,

    /* U+0073 "s" */
    0x3f, 0x3f, 0xf0, 0xf0, 0xf, 0xf, 0xfc, 0xfc,

    /* U+0074 "t" */
    0xc3, 0xf, 0xff, 0xc3, 0xc, 0x30, 0x3c, 0xf0,

    /* U+0075 "u" */
    0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x3f, 0x3f,

    /* U+0076 "v" */
    0xc3, 0xc3, 0xc3, 0xc3, 0x3c, 0x3c, 0x3c, 0x3c,

    /* U+0077 "w" */
    0xcc, 0xf3, 0x3c, 0xcf, 0x33, 0xcc, 0xf3, 0x33,
    0xfc, 0xff,

    /* U+0078 "x" */
    0xc3, 0xc3, 0x3c, 0x3c, 0x3c, 0x3c, 0xc3, 0xc3,

    /* U+0079 "y" */
    0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x3f, 0x3f,
    0x3, 0x3, 0x3c, 0x3c,

    /* U+007A "z" */
    0xff, 0xff, 0xc, 0xc, 0x30, 0x30, 0xff, 0xff,

    /* U+007B "{" */
    0xc, 0x33, 0xc, 0x30, 0xcc, 0x30, 0x30, 0xc3,
    0xc, 0xc, 0x30,

    /* U+007C "|" */
    0xff, 0xff, 0xff,

    /* U+007D "}" */
    0xc3, 0x3, 0xc, 0x30, 0xc0, 0xc3, 0x30, 0xc3,
    0xc, 0xc3, 0x0,

    /* U+007E "~" */
    0x33, 0x33, 0xcc, 0xcc
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 96, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 64, .box_w = 2, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 4, .adv_w = 128, .box_w = 6, .box_h = 4, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 7, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 20, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 33, .adv_w = 256, .box_w = 14, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 54, .adv_w = 192, .box_w = 10, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 69, .adv_w = 64, .box_w = 2, .box_h = 4, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 70, .adv_w = 96, .box_w = 4, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 76, .adv_w = 96, .box_w = 4, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 82, .adv_w = 128, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 87, .adv_w = 128, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 92, .adv_w = 96, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 94, .adv_w = 128, .box_w = 6, .box_h = 2, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 96, .adv_w = 64, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 97, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 106, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 118, .adv_w = 160, .box_w = 4, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 124, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 136, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 148, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 160, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 172, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 184, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 196, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 208, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 220, .adv_w = 64, .box_w = 2, .box_h = 8, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 222, .adv_w = 64, .box_w = 2, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 225, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 233, .adv_w = 128, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 238, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 246, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 258, .adv_w = 192, .box_w = 10, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 273, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 285, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 297, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 309, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 321, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 333, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 345, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 357, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 369, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 378, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 390, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 402, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 414, .adv_w = 192, .box_w = 10, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 429, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 441, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 453, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 465, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 477, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 489, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 501, .adv_w = 192, .box_w = 10, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 516, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 528, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 540, .adv_w = 192, .box_w = 10, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 555, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 567, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 579, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 591, .adv_w = 96, .box_w = 4, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 597, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 606, .adv_w = 96, .box_w = 4, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 612, .adv_w = 128, .box_w = 6, .box_h = 4, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 615, .adv_w = 160, .box_w = 8, .box_h = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 617, .adv_w = 160, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 625, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 637, .adv_w = 160, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 645, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 657, .adv_w = 160, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 665, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 674, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 686, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 698, .adv_w = 64, .box_w = 2, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 701, .adv_w = 96, .box_w = 4, .box_h = 16, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 709, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 721, .adv_w = 64, .box_w = 2, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 724, .adv_w = 192, .box_w = 10, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 734, .adv_w = 160, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 742, .adv_w = 160, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 750, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 762, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 774, .adv_w = 160, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 782, .adv_w = 160, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 790, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 798, .adv_w = 160, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 806, .adv_w = 160, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 814, .adv_w = 192, .box_w = 10, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 824, .adv_w = 160, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 832, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 844, .adv_w = 160, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 852, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 863, .adv_w = 64, .box_w = 2, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 866, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 877, .adv_w = 160, .box_w = 8, .box_h = 4, .ofs_x = 1, .ofs_y = 8}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 64, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 97, .range_length = 30, .glyph_id_start = 65,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
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
    .cmap_num = 2,
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
const lv_font_t bm_mini_16 = {
#else
lv_font_t bm_mini_16 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 16,          /*The maximum line height required by the font*/
    .base_line = 4,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -3,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if BM_MINI_16*/

