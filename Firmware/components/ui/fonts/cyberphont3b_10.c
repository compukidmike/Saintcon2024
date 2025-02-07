/*******************************************************************************
 * Size: 10 px
 * Bpp: 1
 * Opts: --bpp 1 --size 10 --no-compress --no-prefilter --font /Users/dwarkentin/Library/CloudStorage/GoogleDrive-ev0rtex@gmail.com/.shortcut-targets-by-id/1M-9uKjsqE0vKNDFWxBemLGBB4Gobi9fB/Saintcon 2024 Badge/Assets/cyberphont 3.0/Cyberphont 3.0 B.ttf -r 0x20-0x7C --format lvgl -o /Users/dwarkentin/src/projects/Saintcon2024Dev/badge-firmware/components/ui/fonts/cyberphont3b_10.c --force-fast-kern-format
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

#ifndef CYBERPHONT3B_10
#define CYBERPHONT3B_10 1
#endif

#if CYBERPHONT3B_10

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xe4,

    /* U+0022 "\"" */
    0x99,

    /* U+0023 "#" */
    0x4b, 0x74, 0x80, 0xdd, 0x20,

    /* U+0024 "$" */
    0x5e, 0xe, 0x80,

    /* U+0025 "%" */
    0x84, 0x21, 0x0, 0x42, 0x10,

    /* U+0027 "'" */
    0xc0,

    /* U+0028 "(" */
    0x68, 0x90,

    /* U+0029 ")" */
    0x94, 0x60,

    /* U+002B "+" */
    0x5d, 0x0,

    /* U+002C "," */
    0x60,

    /* U+002D "-" */
    0xe0,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x4, 0x21, 0x0, 0x42, 0x0,

    /* U+0030 "0" */
    0x5e, 0x39, 0x40, 0xc7, 0x60,

    /* U+0031 "1" */
    0xec,

    /* U+0032 "2" */
    0xd8, 0x1d, 0xc0, 0x83, 0x70,

    /* U+0033 "3" */
    0xd8, 0x1d, 0xc0, 0x7, 0x70,

    /* U+0034 "4" */
    0x82, 0x8, 0x80, 0xdc, 0x20,

    /* U+0035 "5" */
    0x5e, 0xd, 0xc0, 0x7, 0x60,

    /* U+0036 "6" */
    0x5e, 0xd, 0xc0, 0x87, 0x60,

    /* U+0037 "7" */
    0xd8, 0x10, 0x40, 0x4, 0x10,

    /* U+0038 "8" */
    0x5e, 0x1d, 0xc0, 0x87, 0x60,

    /* U+0039 "9" */
    0x5e, 0x1d, 0xc0, 0x7, 0x60,

    /* U+003A ":" */
    0xa0,

    /* U+003B ";" */
    0x41, 0x80,

    /* U+003C "<" */
    0x64,

    /* U+003E ">" */
    0x98,

    /* U+003F "?" */
    0xd8, 0x74, 0x0, 0x40,

    /* U+0041 "A" */
    0x5e, 0x1d, 0xc0, 0x86, 0x10,

    /* U+0042 "B" */
    0x5e, 0x1d, 0x80, 0x87, 0x70,

    /* U+0043 "C" */
    0x5e, 0x8, 0x0, 0x83, 0x70,

    /* U+0044 "D" */
    0xda, 0x18, 0x40, 0x87, 0x70,

    /* U+0045 "E" */
    0x5e, 0x9, 0x80, 0x83, 0x70,

    /* U+0046 "F" */
    0x5e, 0x9, 0x80, 0x82, 0x0,

    /* U+0047 "G" */
    0x5e, 0x9, 0xc0, 0x87, 0x60,

    /* U+0048 "H" */
    0x86, 0x1d, 0xc0, 0x86, 0x10,

    /* U+0049 "I" */
    0xec,

    /* U+004A "J" */
    0xe9, 0x5, 0x80,

    /* U+004B "K" */
    0x8c, 0xb0, 0x9, 0x44,

    /* U+004C "L" */
    0x84, 0x20, 0x8, 0x2c,

    /* U+004D "M" */
    0x5e, 0x59, 0x40, 0x96, 0x50,

    /* U+004E "N" */
    0x5e, 0x18, 0x40, 0x86, 0x10,

    /* U+004F "O" */
    0x5e, 0x18, 0x40, 0x87, 0x60,

    /* U+0050 "P" */
    0x5e, 0x1d, 0xc0, 0x82, 0x0,

    /* U+0051 "Q" */
    0x5a, 0x28, 0x80, 0x8b, 0x70,

    /* U+0052 "R" */
    0x5e, 0x1d, 0x80, 0x86, 0x10,

    /* U+0053 "S" */
    0x5e, 0xd, 0xc0, 0x7, 0x70,

    /* U+0054 "T" */
    0xdc, 0x41, 0x0, 0x10, 0x40,

    /* U+0055 "U" */
    0x86, 0x18, 0x40, 0x85, 0x70,

    /* U+0056 "V" */
    0x86, 0x18, 0x40, 0xcd, 0x60,

    /* U+0057 "W" */
    0x86, 0x59, 0x40, 0x97, 0x60,

    /* U+0058 "X" */
    0x86, 0x15, 0x80, 0x86, 0x10,

    /* U+0059 "Y" */
    0x86, 0x15, 0x80, 0x10, 0x40,

    /* U+005A "Z" */
    0xd8, 0x1d, 0xc0, 0x81, 0x70,

    /* U+005B "[" */
    0xe8, 0xb0,

    /* U+005D "]" */
    0xd4, 0x70,

    /* U+005E "^" */
    0x54,

    /* U+005F "_" */
    0xe0,

    /* U+0061 "a" */
    0x5e, 0x1d, 0xc0, 0x86, 0x10,

    /* U+0062 "b" */
    0x5e, 0x1d, 0x80, 0x87, 0x70,

    /* U+0063 "c" */
    0x5e, 0x8, 0x0, 0x83, 0x70,

    /* U+0064 "d" */
    0xda, 0x18, 0x40, 0x87, 0x70,

    /* U+0065 "e" */
    0x5e, 0x9, 0x80, 0x83, 0x70,

    /* U+0066 "f" */
    0x5e, 0x9, 0x80, 0x82, 0x0,

    /* U+0067 "g" */
    0x5e, 0x9, 0xc0, 0x87, 0x60,

    /* U+0068 "h" */
    0x86, 0x1d, 0xc0, 0x86, 0x10,

    /* U+0069 "i" */
    0xec,

    /* U+006A "j" */
    0xe9, 0x5, 0x80,

    /* U+006B "k" */
    0x8c, 0xb0, 0x9, 0x44,

    /* U+006C "l" */
    0x84, 0x20, 0x8, 0x2c,

    /* U+006D "m" */
    0x5e, 0x59, 0x40, 0x96, 0x50,

    /* U+006E "n" */
    0x5e, 0x18, 0x40, 0x86, 0x10,

    /* U+006F "o" */
    0x5e, 0x18, 0x40, 0x87, 0x60,

    /* U+0070 "p" */
    0x5e, 0x1d, 0xc0, 0x82, 0x0,

    /* U+0071 "q" */
    0x5a, 0x28, 0x80, 0x8b, 0x70,

    /* U+0072 "r" */
    0x5e, 0x1d, 0x80, 0x86, 0x10,

    /* U+0073 "s" */
    0x5e, 0xd, 0xc0, 0x7, 0x70,

    /* U+0074 "t" */
    0xdc, 0x41, 0x0, 0x10, 0x40,

    /* U+0075 "u" */
    0x86, 0x18, 0x40, 0x85, 0x70,

    /* U+0076 "v" */
    0x86, 0x18, 0x40, 0xcd, 0x60,

    /* U+0077 "w" */
    0x86, 0x59, 0x40, 0x97, 0x60,

    /* U+0078 "x" */
    0x86, 0x15, 0x80, 0x86, 0x10,

    /* U+0079 "y" */
    0x86, 0x15, 0x80, 0x10, 0x40,

    /* U+007A "z" */
    0xd8, 0x1d, 0xc0, 0x81, 0x70,

    /* U+007C "|" */
    0xec
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 60, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 40, .box_w = 1, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2, .adv_w = 80, .box_w = 4, .box_h = 2, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 3, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 8, .adv_w = 100, .box_w = 3, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 11, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 16, .adv_w = 40, .box_w = 1, .box_h = 2, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 17, .adv_w = 60, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 19, .adv_w = 60, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 21, .adv_w = 80, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 23, .adv_w = 60, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 24, .adv_w = 80, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 25, .adv_w = 40, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 26, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 31, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 36, .adv_w = 40, .box_w = 1, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 37, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 42, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 47, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 52, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 57, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 62, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 67, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 72, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 77, .adv_w = 40, .box_w = 1, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 78, .adv_w = 60, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 80, .adv_w = 60, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 81, .adv_w = 60, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 82, .adv_w = 100, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 86, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 91, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 96, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 101, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 106, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 111, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 116, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 121, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 126, .adv_w = 40, .box_w = 1, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 127, .adv_w = 80, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 130, .adv_w = 100, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 134, .adv_w = 100, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 138, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 143, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 148, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 153, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 158, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 163, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 168, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 173, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 178, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 183, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 188, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 193, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 198, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 203, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 208, .adv_w = 60, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 210, .adv_w = 60, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 212, .adv_w = 80, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 213, .adv_w = 80, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 214, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 219, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 224, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 229, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 234, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 239, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 244, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 249, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 254, .adv_w = 40, .box_w = 1, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 255, .adv_w = 80, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 258, .adv_w = 100, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 262, .adv_w = 100, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 266, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 271, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 276, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 281, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 286, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 291, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 296, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 301, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 306, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 311, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 316, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 321, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 326, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 331, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 336, .adv_w = 40, .box_w = 1, .box_h = 6, .ofs_x = 0, .ofs_y = 0}
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
const lv_font_t cyberphont3b_10 = {
#else
lv_font_t cyberphont3b_10 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 6,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = 1,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if CYBERPHONT3B_10*/

