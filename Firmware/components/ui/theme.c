#include "theme.h"

#define CORNER_TL_WIDTH    20
#define CORNER_TL_HEIGHT   15
#define CORNER_TR_WIDTH    10
#define CORNER_TR_HEIGHT   14
#define CORNER_BL_WIDTH    4
#define CORNER_BL_HEIGHT   4
#define CORNER_BR_WIDTH    32
#define CORNER_BR_HEIGHT   5
#define EDGE_TOP_HEIGHT    15
#define EDGE_BOTTOM_HEIGHT 5
#define EDGE_LEFT_WIDTH    4
#define EDGE_RIGHT_WIDTH   5

// Button styles
lv_style_t button_style;

lv_style_t button_red;
lv_style_t button_orange;
lv_style_t button_yellow;
lv_style_t button_green;
lv_style_t button_blue;
lv_style_t button_purple;
lv_style_t button_bluegrey;
lv_style_t button_grey;

// Flag to track if the styles have been initialized - we don't want to re-initialize them because it can cause memory leaks
static bool styles_initialized = false;

static lv_theme_t *theme;

// Function prototypes for styles
static void style_button_init();

// Initialize the LVGL style
void style_init() {
    if (styles_initialized) {
        return;
    }

    style_button_init();
    styles_initialized = true;
}

// Initialize the LVGL button styles
static void style_button_init() {
    // Base button style
    lv_style_init(&button_style);
    lv_style_set_border_width(&button_style, 2);
    lv_style_set_radius(&button_style, 1);
    lv_style_set_text_color(&button_style, lv_color_hex(WHITE));
    lv_style_set_text_font(&button_style, &cyberphont3b_16);

    // Red button style
    lv_style_init(&button_red);
    lv_style_set_bg_color(&button_red, lv_color_hex(RED_MAIN));
    lv_style_set_border_color(&button_red, lv_color_hex(RED_DIMMER));

    // Orange button style
    lv_style_init(&button_orange);
    lv_style_set_bg_color(&button_orange, lv_color_hex(ORANGE_MAIN));
    lv_style_set_border_color(&button_orange, lv_color_hex(ORANGE_DIMMER));

    // Yellow button style
    lv_style_init(&button_yellow);
    lv_style_set_bg_color(&button_yellow, lv_color_hex(YELLOW_MAIN));
    lv_style_set_border_color(&button_yellow, lv_color_hex(YELLOW_DIMMER));

    // Green button style
    lv_style_init(&button_green);
    lv_style_set_bg_color(&button_green, lv_color_hex(GREEN_MAIN));
    lv_style_set_border_color(&button_green, lv_color_hex(GREEN_DIMMER));

    // Blue button style
    lv_style_init(&button_blue);
    lv_style_set_bg_color(&button_blue, lv_color_hex(BLUE_MAIN));
    lv_style_set_border_color(&button_blue, lv_color_hex(BLUE_DIMMER));

    // Purple button style
    lv_style_init(&button_purple);
    lv_style_set_bg_color(&button_purple, lv_color_hex(PURPLE_MAIN));
    lv_style_set_border_color(&button_purple, lv_color_hex(PURPLE_DIMMER));

    // Blue-grey button style
    lv_style_init(&button_bluegrey);
    lv_style_set_bg_color(&button_bluegrey, lv_color_hex(BLUEGRAY_MAIN));
    lv_style_set_border_color(&button_bluegrey, lv_color_hex(BLUEGRAY_DIMMER));

    // Grey button style
    lv_style_init(&button_grey);
    lv_style_set_bg_color(&button_grey, lv_color_hex(GRAY_BASE));
    lv_style_set_border_color(&button_grey, lv_color_hex(GRAY_SHADE_2));
}

// // Initialize the LVGL theme
// void theme_init() {
//     theme = lv_theme_default_init(lv_display_get_default(), lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
//                                   LV_THEME_DEFAULT_DARK, LV_FONT_DEFAULT);
//     lv_display_set_theme(lv_display_get_default(), theme);
// }

// lv_obj_t *content_container_large_create(lv_obj_t *parent, lv_coord_t width, lv_coord_t height) {
//     lv_obj_t *container = lv_obj_create(parent);
//     lv_obj_set_size(container, width, height);
//     lv_obj_set_style_border_width(container, 0, LV_PART_MAIN);
//     lv_obj_set_style_radius(container, 0, LV_PART_MAIN);
//     lv_obj_set_style_pad_all(container, 0, LV_PART_MAIN);
//     // lv_obj_remove_style_all(container);
//     // lv_obj_set_style_pad_all(container, 15, LV_PART_MAIN);

//     // const uint8_t scale      = 2;
//     // const uint8_t src_width  = lv_obj_get_width(&theme_screens);
//     // const uint8_t src_height = lv_obj_get_height(&theme_screens);

//     // static uint8_t cbuf[LV_CANVAS_BUF_SIZE(CORNER_TL_WIDTH, CORNER_TL_HEIGHT, LV_COLOR_DEPTH, LV_DRAW_BUF_STRIDE_ALIGN)];
//     // static uint8_t cbuf[LV_CANVAS_BUF_SIZE(75, 75, 16, LV_DRAW_BUF_STRIDE_ALIGN)];

//     // lv_obj_t *canvas = lv_canvas_create(container);
//     // lv_obj_t *canvas = lv_canvas_create(lv_screen_active());
//     // lv_canvas_set_buffer(canvas, cbuf, 50, 50, LV_COLOR_FORMAT_RGB565A8);
//     // lv_canvas_set_buffer(canvas, cbuf, 75, 75, LV_COLOR_FORMAT_NATIVE_WITH_ALPHA);
//     // lv_canvas_fill_bg(canvas, lv_color_hex(0x000000), LV_OPA_COVER);
//     // lv_canvas_fill_bg(canvas, lv_color_hex3(0xccc), LV_OPA_COVER);

//     LV_DRAW_BUF_DEFINE_STATIC(dbuf, CORNER_TL_WIDTH, CORNER_TL_HEIGHT, LV_COLOR_FORMAT_ARGB8888);
//     // LV_DRAW_BUF_DEFINE_STATIC(dbuf, 75, 75, LV_COLOR_FORMAT_RGB565A8);
//     LV_DRAW_BUF_INIT_STATIC(dbuf);

//     lv_obj_t *canvas = lv_canvas_create(container);
//     lv_canvas_set_draw_buf(canvas, &dbuf);

//     // lv_canvas_fill_bg(canvas, lv_palette_main(LV_PALETTE_BLUE_GREY), LV_OPA_COVER);
//     lv_canvas_fill_bg(canvas, lv_palette_main(LV_PALETTE_BLUE_GREY), LV_OPA_TRANSP);

//     lv_layer_t layer;
//     lv_canvas_init_layer(canvas, &layer);

//     lv_draw_image_dsc_t dsc;
//     lv_draw_image_dsc_init(&dsc);
//     dsc.src = &theme_screens;
//     // dsc.image_area = (lv_area_t){
//     //     .x1 = 7,
//     //     .y1 = 24,
//     //     .x2 = 7 + CORNER_TL_WIDTH - 1,
//     //     .y2 = 24 + CORNER_TL_HEIGHT - 1,
//     // };
//     // dsc.image_area = (lv_area_t){
//     //     .x1 = 0,
//     //     .y1 = 0,
//     //     .x2 = 167 - 1,
//     //     .y2 = 87 - 1,
//     // };

//     // lv_area_t corner_tl_coords = {
//     //     .x1 = -7,
//     //     .y1 = -24,
//     //     .x2 = -7 + CORNER_TL_WIDTH - 1,
//     //     .y2 = -24 + CORNER_TL_HEIGHT - 1,
//     // };
//     lv_area_t corner_tl_coords = {
//         .x1 = -7,
//         .y1 = -24,
//         .x2 = -7 + theme_screens.header.w - 1,
//         .y2 = -24 + theme_screens.header.h - 1,
//     };
//     lv_draw_image(&layer, &dsc, &corner_tl_coords);
//     lv_canvas_finish_layer(canvas, &layer);
//     lv_image_set_pivot(canvas, 0, 0);
//     // lv_image_set_rotation(canvas, 0);
//     lv_image_set_antialias(canvas, false);
//     lv_image_set_scale(canvas, 512);

//     lv_obj_align(canvas, LV_ALIGN_TOP_LEFT, 0, 0);
//     // lv_obj_set_style_border_color(canvas, lv_color_hex(0x000000), LV_PART_MAIN);
//     // lv_obj_set_style_border_width(canvas, 1, LV_PART_MAIN);

//     // lv_color_t canvas_buffer[src_width * src_height];
//     // lv_obj_t *canvas = lv_canvas_create(container);
//     // lv_canvas_set_buffer(canvas, canvas_buffer, src_width, src_height, LV_COLOR_FORMAT_RGB565A8);

//     // // Top-left corner
//     // lv_obj_t *corner_tl = lv_image_create(container);
//     // lv_image_set_src(corner_tl, &theme_screens);
//     // lv_image_set_inner_align(corner_tl, LV_IMAGE_ALIGN_TOP_LEFT);
//     // lv_image_set_offset_x(corner_tl, -7);
//     // lv_image_set_offset_y(corner_tl, -24);
//     // lv_image_set_antialias(corner_tl, false);
//     // lv_obj_set_size(corner_tl, 20, 15);
//     // lv_obj_set_align(corner_tl, LV_ALIGN_TOP_LEFT);

//     // Top-right corner
//     lv_obj_t *corner_tr = lv_image_create(container);
//     lv_image_set_src(corner_tr, &theme_screens);
//     lv_image_set_pivot(corner_tr, CORNER_TR_WIDTH, 0);
//     lv_image_set_antialias(corner_tr, false);
//     lv_image_set_scale(corner_tr, 512);
//     lv_image_set_offset_x(corner_tr, -81 - 2 /* - 12 */);
//     lv_image_set_offset_y(corner_tr, -24 + 14);
//     lv_obj_set_size(corner_tr, CORNER_TR_WIDTH, CORNER_TR_HEIGHT);
//     lv_obj_align(corner_tr, LV_ALIGN_TOP_RIGHT, 0, 0);
//     // lv_obj_set_style_border_color(corner_tr, lv_color_hex(0x000000), LV_PART_MAIN);
//     // lv_obj_set_style_border_width(corner_tr, 1, LV_PART_MAIN);

//     // // Bottom-left corner
//     // lv_obj_t *corner_bl = lv_image_create(container);
//     // lv_image_set_src(corner_bl, &theme_screens);
//     // lv_image_set_pivot(corner_bl, 0, CORNER_BL_HEIGHT);
//     // lv_image_set_antialias(corner_bl, false);
//     // lv_image_set_scale(corner_bl, 512);
//     // lv_image_set_offset_x(corner_bl, -7);
//     // lv_image_set_offset_y(corner_bl, -47);
//     // lv_obj_set_size(corner_bl, CORNER_BL_WIDTH, CORNER_BL_HEIGHT);
//     // lv_obj_align(corner_bl, LV_ALIGN_BOTTOM_LEFT, 0, 0);
//     // lv_obj_set_style_border_color(corner_bl, lv_color_hex(0x000000), LV_PART_MAIN);
//     // lv_obj_set_style_border_width(corner_bl, 1, LV_PART_MAIN);

//     // Bottom-right corner
//     lv_obj_t *corner_br = lv_image_create(container);
//     lv_image_set_src(corner_br, &theme_screens);
//     lv_image_set_pivot(corner_br, CORNER_BR_WIDTH, CORNER_BR_HEIGHT);
//     lv_image_set_antialias(corner_br, false);
//     lv_image_set_align(corner_br, LV_IMAGE_ALIGN_STRETCH);
//     // lv_image_set_scale(corner_br, 512);
//     lv_image_set_offset_x(corner_br, -59 + 9);
//     lv_image_set_offset_y(corner_br, -46 - 4);
//     lv_obj_set_size(corner_br, CORNER_BR_WIDTH * 2, CORNER_BR_HEIGHT * 2);
//     lv_obj_align(corner_br, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
//     // lv_obj_set_style_border_color(corner_br, lv_color_hex(0x000000), LV_PART_MAIN);
//     // lv_obj_set_style_border_width(corner_br, 1, LV_PART_MAIN);

//     // // Top edge
//     // lv_obj_t *edge_top = lv_image_create(container);
//     // lv_image_set_src(edge_top, &theme_screens);
//     // lv_image_set_offset_x(edge_top, 27);
//     // lv_image_set_offset_y(edge_top, 24);
//     // lv_obj_set_size(edge_top, width - 29, 15);
//     // lv_obj_align(edge_top, LV_ALIGN_TOP_MID, 0, 0);

//     // // Bottom edge
//     // lv_obj_t *edge_bottom = lv_image_create(container);
//     // lv_image_set_src(edge_bottom, &theme_screens);
//     // lv_image_set_offset_x(edge_bottom, 27);
//     // lv_image_set_offset_y(edge_bottom, 47);
//     // lv_obj_set_size(edge_bottom, width - 36, 5);
//     // lv_obj_align(edge_bottom, LV_ALIGN_BOTTOM_MID, 0, 0);

//     // // Left edge
//     // lv_obj_t *edge_left = lv_image_create(container);
//     // lv_image_set_src(edge_left, &theme_screens);
//     // lv_image_set_offset_x(edge_left, 7);
//     // lv_image_set_offset_y(edge_left, 39);
//     // lv_obj_set_size(edge_left, 4, height - 19);
//     // lv_obj_align(edge_left, LV_ALIGN_LEFT_MID, 0, 0);

//     // // Right edge
//     // lv_obj_t *edge_right = lv_image_create(container);
//     // lv_image_set_src(edge_right, &theme_screens);
//     // lv_image_set_offset_x(edge_right, 87);
//     // lv_image_set_offset_y(edge_right, 38);
//     // lv_obj_set_size(edge_right, 5, height - 19);
//     // lv_obj_align(edge_right, LV_ALIGN_RIGHT_MID, 0, 0);

//     return container;
// }

void style_msgbox(lv_obj_t *msgbox) {
    lv_obj_set_style_pad_all(msgbox, 10, LV_PART_MAIN);
    lv_obj_set_style_border_width(msgbox, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(msgbox, lv_color_hex(STATUS_BORDER), LV_PART_MAIN);
    lv_obj_set_style_radius(msgbox, 3, LV_PART_MAIN);

    const lv_obj_t *header = lv_msgbox_get_header(msgbox);
    lv_obj_set_style_text_font(header, &cyberphont3b_16, LV_PART_MAIN);

    const lv_obj_t *content = lv_msgbox_get_content(msgbox);
    lv_obj_set_style_text_font(content, &cyberphont3b_14, LV_PART_MAIN);
    lv_obj_set_style_text_line_space(content, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(content, 10, LV_PART_MAIN);

    // Style each button in the footer
    const lv_obj_t *footer = lv_msgbox_get_footer(msgbox);
    uint32_t btn_count     = lv_obj_get_child_count_by_type(footer, &lv_msgbox_footer_button_class);
    for (uint32_t i = 0; i < btn_count; i++) {
        lv_obj_t *btn = lv_obj_get_child_by_type(footer, i, &lv_msgbox_footer_button_class);
        lv_obj_set_style_text_font(btn, &cyberphont3b_16, LV_PART_MAIN);
        lv_obj_set_style_bg_color(btn, lv_color_hex(STATUS_BG), LV_PART_MAIN);
        lv_obj_set_style_border_color(btn, lv_color_hex(STATUS_BORDER), LV_PART_MAIN);
        lv_obj_set_style_border_width(btn, 2, LV_PART_MAIN);
        lv_obj_set_style_radius(btn, 3, LV_PART_MAIN);
        lv_obj_set_height(btn, 30);
    }
}

static void msgbox_button_cb(lv_event_t *e) {
    lv_obj_t *msgbox = lv_event_get_user_data(e);
    lv_msgbox_close(msgbox);
}

void show_msgbox(const char *text) {
    lv_obj_t *msgbox = lv_msgbox_create(NULL);
    lv_msgbox_add_text(msgbox, text);
    lv_obj_set_style_pad_all(msgbox, 10, LV_PART_MAIN);
    lv_obj_set_style_border_width(msgbox, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(msgbox, lv_color_hex(STATUS_BORDER), LV_PART_MAIN);
    lv_obj_set_style_radius(msgbox, 3, LV_PART_MAIN);
    lv_obj_t *msgbox_content = lv_msgbox_get_content(msgbox);
    lv_obj_set_style_text_font(msgbox_content, &cyberphont3b_14, LV_PART_MAIN);
    lv_obj_set_style_text_line_space(msgbox_content, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(msgbox_content, 10, LV_PART_MAIN);
    lv_obj_t *close_btn = lv_msgbox_add_footer_button(msgbox, "OK");
    lv_obj_set_style_text_font(close_btn, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_bg_color(close_btn, lv_color_hex(STATUS_BG), LV_PART_MAIN);
    lv_obj_set_style_border_color(close_btn, lv_color_hex(STATUS_BORDER), LV_PART_MAIN);
    lv_obj_set_style_border_width(close_btn, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(close_btn, 3, LV_PART_MAIN);
    lv_obj_set_height(close_btn, 30);
    lv_obj_add_event_cb(close_btn, msgbox_button_cb, LV_EVENT_CLICKED, msgbox);
}