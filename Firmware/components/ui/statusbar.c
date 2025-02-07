#include "esp_log.h"

#include "badge.h"
#include "content.h"
#include "display.h"
#include "statusbar.h"
#include "theme.h"
#include "wifi_manager.h"

static const char *TAG = "screens/main [statusbar]";

// Icons
static lv_obj_t *wifi_icon                = NULL;
static lv_obj_t *battery_icon             = NULL;
static lv_obj_t *minibadge_icon           = NULL;
static lv_obj_t *minibadge_icon_container = NULL;
static lv_obj_t *alert_icon               = NULL;
static lv_obj_t *alert_icon_container     = NULL;
static lv_obj_t *status_label             = NULL;

// Timer for WiFi icon animation
static lv_timer_t *wifi_state_timer = NULL;
static bool wifi_connecting_blink   = false;

// Alert icon flash
static lv_timer_t *alert_flash_timer = NULL;
static bool alert_icon_visible       = false;

// Status label event descriptor
static lv_event_dsc_t *status_label_event_dsc = NULL;

// Menu for content selection
static lv_obj_t *menu_container = NULL;

// Function prototypes
static void update_wifi_icon(wifi_status_t wifi_status);
static void wifi_state_timer_cb(lv_timer_t *timer);
static void status_bar_event_handler(lv_event_t *e);
static void status_label_event_handler(lv_event_t *e);
static void create_label(lv_obj_t *parent);
static void create_menu(lv_obj_t *parent);
static void alert_icon_event_handler(lv_event_t *e);
static void minibadge_icon_event_handler(lv_event_t *e);
static void alert_flash_timer_cb(lv_timer_t *timer);
void create_status_bar(lv_obj_t *parent) {
    // Create a container for the status bar
    lv_obj_t *status_bar = lv_obj_create(parent);
    lv_obj_align(status_bar, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_size(status_bar, 40, LV_VER_RES);
    lv_obj_set_style_bg_color(status_bar, lv_color_hex(STATUS_BG), LV_PART_MAIN);
    lv_obj_set_style_border_width(status_bar, 3, LV_PART_MAIN);
    lv_obj_set_style_border_color(status_bar, lv_color_hex(STATUS_BORDER), LV_PART_MAIN);

    // Disable scrolling and dragging on the status bar
    lv_obj_set_scrollbar_mode(status_bar, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(status_bar, LV_OBJ_FLAG_SCROLLABLE);

    // Event handler for the status bar
    status_label_event_dsc = lv_obj_add_event_cb(status_bar, status_bar_event_handler, LV_EVENT_CLICKED, NULL);

    //
    // Create all the icons
    //

    // WiFi icon
    wifi_icon = lv_label_create(status_bar);
    lv_label_set_text(wifi_icon, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(wifi_icon, lv_color_hex(STATUS_DIM), LV_PART_MAIN);
    lv_obj_align(wifi_icon, LV_ALIGN_TOP_MID, 0, 0);

    // Battery icon
    battery_icon = lv_label_create(status_bar);
    lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_EMPTY);
    lv_obj_set_style_text_color(battery_icon, lv_color_hex(STATUS_BRIGHT), LV_PART_MAIN);
    lv_obj_align(battery_icon, LV_ALIGN_TOP_MID, 0, 20);

    // Minibadge icon container
    minibadge_icon_container = lv_obj_create(status_bar);
    lv_obj_set_size(minibadge_icon_container, 26, 26);
    lv_obj_set_style_pad_all(minibadge_icon_container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(minibadge_icon_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(minibadge_icon_container, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(minibadge_icon_container, 2, LV_PART_MAIN);
    lv_obj_align(minibadge_icon_container, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_remove_flag(minibadge_icon_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(minibadge_icon_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(minibadge_icon_container, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(minibadge_icon_container, minibadge_icon_event_handler, LV_EVENT_CLICKED, NULL);

    // Minibadge icon
    minibadge_icon = lv_label_create(minibadge_icon_container);
    lv_label_set_text(minibadge_icon, FA_CHIP);
    lv_obj_set_style_text_font(minibadge_icon, &fa_sharp_icons_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(minibadge_icon, lv_color_hex(STATUS_DIM), LV_PART_MAIN);
    lv_obj_align(minibadge_icon, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(minibadge_icon, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_EVENT_BUBBLE);

    // Alert icon container to handle clicks even when the icon is hidden
    alert_icon_container = lv_obj_create(status_bar);
    lv_obj_set_size(alert_icon_container, 26, 26);
    lv_obj_set_style_pad_all(alert_icon_container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(alert_icon_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(alert_icon_container, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(alert_icon_container, 2, LV_PART_MAIN);
    lv_obj_align(alert_icon_container, LV_ALIGN_BOTTOM_MID, 0, 6);
    lv_obj_remove_flag(alert_icon_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(alert_icon_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(alert_icon_container, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(alert_icon_container, alert_icon_event_handler, LV_EVENT_CLICKED, NULL);

    // Alert icon
    alert_icon = lv_label_create(alert_icon_container);
    lv_label_set_text(alert_icon, FA_TOWER);
    lv_obj_set_style_text_font(alert_icon, &fa_sharp_icons_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(alert_icon, lv_color_hex(STATUS_DIM), LV_PART_MAIN);
    lv_obj_align(alert_icon, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(alert_icon, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_EVENT_BUBBLE);

    //
    // Create the status label
    //

    // Label itself
    status_label = lv_label_create(status_bar);
    lv_obj_set_style_text_font(status_label, &clarity_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(status_label, lv_color_hex(STATUS_BRIGHT), LV_PART_MAIN);
    lv_label_set_text(status_label, "------"); // Leaving this blank will cause LVGL to get stuck in a loop for some reason (bug?)
                                               //   NOTE: disabling the click handler callback or removing rotation seems to fix
                                               //   it too?
    lv_obj_set_style_transform_pivot_x(status_label, lv_pct(50), LV_PART_MAIN);
    lv_obj_set_style_transform_pivot_y(status_label, lv_pct(50), LV_PART_MAIN);
    lv_obj_set_style_transform_rotation(status_label, -900, LV_PART_MAIN);
    lv_obj_align(status_label, LV_ALIGN_CENTER, 0, 0);

    // Padding for the label
    lv_obj_set_style_pad_left(status_label, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_right(status_label, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_top(status_label, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(status_label, 5, LV_PART_MAIN);

    // Event handler for the status label to detect clicks
    lv_obj_add_flag(status_label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(status_label, status_label_event_handler, LV_EVENT_CLICKED, NULL);

    // Create the menu but keep it hidden initially
    create_menu(parent);
    lv_obj_add_flag(menu_container, LV_OBJ_FLAG_HIDDEN);

    // //
    // // Create the hint triangle on the status bar
    // //

    // lv_obj_add_flag(status_bar, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

    // const uint32_t triangle_width  = 16;
    // const uint32_t triangle_height = 30;
    // LV_DRAW_BUF_DEFINE_STATIC(triangle_hint_buf, 16, 30, LV_COLOR_FORMAT_ARGB8888);
    // LV_DRAW_BUF_INIT_STATIC(triangle_hint_buf);

    // lv_obj_t *triangle_hint_canvas = lv_canvas_create(lv_screen_active());
    // lv_canvas_set_draw_buf(triangle_hint_canvas, &triangle_hint_buf);
    // lv_obj_set_size(triangle_hint_canvas, triangle_width, triangle_height);
    // lv_canvas_fill_bg(triangle_hint_canvas, lv_color_hex(GRAY_TINT_2), LV_OPA_TRANSP);

    // lv_layer_t triangle_hint_layer;
    // lv_canvas_init_layer(triangle_hint_canvas, &triangle_hint_layer);

    // // Define points for a outward-pointing triangle
    // lv_point_precise_t triangle_hint_points[] = {{0, 0}, {0, triangle_height}, {triangle_width, triangle_height / 2}};

    // lv_draw_triangle_dsc_t triangle_hint_dsc;
    // lv_draw_triangle_dsc_init(&triangle_hint_dsc);
    // triangle_hint_dsc.bg_opa   = LV_OPA_COVER;
    // triangle_hint_dsc.bg_color = lv_color_hex(WHITE);
    // triangle_hint_dsc.p[0]     = triangle_hint_points[0];
    // triangle_hint_dsc.p[1]     = triangle_hint_points[1];
    // triangle_hint_dsc.p[2]     = triangle_hint_points[2];
    // lv_draw_triangle(&triangle_hint_layer, &triangle_hint_dsc);

    // lv_canvas_finish_layer(triangle_hint_canvas, &triangle_hint_layer);

    // // Align the triangle to the right of the status label
    // lv_obj_align(triangle_hint_canvas, LV_ALIGN_TOP_MID, 0, 0);
    // // lv_obj_set_pos(triangle_hint_canvas, 0, 0);
    // // lv_obj_align(triangle_hint_canvas, LV_ALIGN_CENTER, 5, 0);
    // // lv_obj_align_to(triangle_hint_canvas, status_bar, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    // // lv_obj_align_to(triangle_hint_canvas, status_bar, LV_ALIGN_TOP_MID, 0, 50);
    // // lv_obj_set_pos(triangle_hint_canvas, LV_HOR_RES / 2, LV_VER_RES / 2);
    // lv_obj_move_foreground(triangle_hint_canvas);
    // lv_obj_clear_flag(triangle_hint_canvas, LV_OBJ_FLAG_HIDDEN);
}

void status_bar_shutdown() {
    // Delete the wifi state timer
    if (wifi_state_timer != NULL) {
        lv_timer_delete(wifi_state_timer);
        wifi_state_timer = NULL;
    }

    if (alert_flash_timer != NULL) {
        lv_timer_delete(alert_flash_timer);
        alert_flash_timer = NULL;
    }

    if (status_label != NULL) {
        lv_obj_del(status_label);
        status_label = NULL;
    }
}

void render_status(ui_state_t *state) {
    ESP_LOGD(TAG, "Updating status bar");

    update_wifi_icon(state->wifi_state);

    // Update the battery icon
    const char *battery_symbol = NULL;
    if (state->battery_charging) {
        battery_symbol = LV_SYMBOL_CHARGE;
    } else {
        switch (state->battery_level) {
            case BATTERY_LEVEL_FULL: battery_symbol = LV_SYMBOL_BATTERY_FULL; break;
            case BATTERY_LEVEL_3: battery_symbol = LV_SYMBOL_BATTERY_3; break;
            case BATTERY_LEVEL_2: battery_symbol = LV_SYMBOL_BATTERY_2; break;
            case BATTERY_LEVEL_1: battery_symbol = LV_SYMBOL_BATTERY_1; break;
            default: battery_symbol = LV_SYMBOL_BATTERY_EMPTY; break;
        }
    }
    lv_label_set_text(battery_icon, battery_symbol);
    if (state->power_connected) {
        lv_obj_set_style_text_color(battery_icon, lv_color_hex(STATUS_GOOD), LV_PART_MAIN);
    } else {
        lv_obj_set_style_text_color(battery_icon, lv_color_hex(STATUS_BRIGHT), LV_PART_MAIN);
    }

    // Update the minibadge icon visibility
    if (state->minibadge_count > 0) {
        if (badge_state.minibadges->slot1.valid || badge_state.minibadges->slot2.valid) {
            lv_obj_set_style_text_color(minibadge_icon, lv_color_hex(STATUS_BRIGHT), LV_PART_MAIN);
        } else {
            lv_obj_set_style_text_color(minibadge_icon, lv_color_hex(STATUS_DIM), LV_PART_MAIN);
        }
    }

    // Update the alert icon visibility
    if (state->alert_count > 0) {
        lv_obj_set_style_text_color(alert_icon, lv_color_hex(BLUE_LIGHTER), LV_PART_MAIN);
        lv_obj_add_flag(alert_icon_container, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(alert_icon, LV_OBJ_FLAG_CLICKABLE);
        if (alert_flash_timer == NULL) {
            alert_flash_timer = lv_timer_create(alert_flash_timer_cb, 500, NULL);
            lv_timer_ready(alert_flash_timer);
        }
        if (alert_icon_visible) {
            lv_obj_remove_flag(alert_icon, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(alert_icon, LV_OBJ_FLAG_HIDDEN);
        }
    } else {
        lv_obj_set_style_text_color(alert_icon, lv_color_hex(STATUS_DIM), LV_PART_MAIN);
        lv_obj_remove_flag(alert_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(alert_icon, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_flag(alert_icon_container, LV_OBJ_FLAG_CLICKABLE);
        if (alert_flash_timer != NULL) {
            lv_timer_delete(alert_flash_timer);
            alert_flash_timer = NULL;
        }
        alert_icon_visible = true;
    }

    // Update the status label
    lv_label_set_text(status_label, state->label);

    ESP_LOGD(TAG, "Status bar updated");
}

static void update_wifi_icon(wifi_status_t wifi_status) {
    if (wifi_state_timer == NULL) {
        wifi_state_timer = lv_timer_create(wifi_state_timer_cb, 500, NULL);
        lv_timer_ready(wifi_state_timer);
    }
    if (wifi_status == WIFI_STATUS_CONNECTING) {
        ESP_LOGD(TAG, "WiFi is connecting");
        lv_timer_resume(wifi_state_timer);
    } else {
        ESP_LOGD(TAG, "WiFi is %s",
                 wifi_status == WIFI_STATUS_CONNECTED    ? "connected"
                 : wifi_status == WIFI_STATUS_CONNECTING ? "connecting"
                                                         : "disconnected");
        lv_timer_pause(wifi_state_timer);
        if (wifi_status == WIFI_STATUS_CONNECTED) {
            lv_obj_set_style_text_color(wifi_icon, lv_color_hex(STATUS_BRIGHT), LV_PART_MAIN);
            // } else if (wifi_status == WIFI_STATUS_SERVER_ERROR) {
            //     lv_obj_set_style_text_color(wifi_icon, lv_color_hex(RED_LIGHT), LV_PART_MAIN);
        } else {
            lv_obj_set_style_text_color(wifi_icon, lv_color_hex(STATUS_DIM), LV_PART_MAIN);
        }
    }
}

static void wifi_state_timer_cb(lv_timer_t *_timer) {
    (void)_timer;
    if (wifi_icon != NULL) { // In case the wifi icon hasn't been created yet
        if (wifi_connecting_blink) {
            lv_obj_set_style_text_color(wifi_icon, lv_color_hex(STATUS_BRIGHT), LV_PART_MAIN);
        } else {
            lv_obj_set_style_text_color(wifi_icon, lv_color_hex(STATUS_DIM), LV_PART_MAIN);
        }
    } else {
        ESP_LOGE(TAG, "WiFi icon is NULL but the timer callback was still triggered");
    }
    wifi_connecting_blink = !wifi_connecting_blink;
}

// Event handler for the status bar
static void status_bar_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (!lv_obj_has_flag(menu_container, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_add_flag(menu_container, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

// Event handler for the status label
static void status_label_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_has_flag(menu_container, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_clear_flag(menu_container, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(menu_container);
        } else {
            lv_obj_add_flag(menu_container, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static const char *menu_items[] = {"Home", "Settings", "Map", "Tower Battle", "Shop", "Level Up", NULL, "Stats"};

static void menu_button_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        int i = (int)lv_event_get_user_data(e);
        ESP_LOGD(TAG, "Selected menu item %s", content_labels[i]);
        lv_obj_add_flag(menu_container, LV_OBJ_FLAG_HIDDEN);
        switch (i) {
            case 0: set_content_page(PAGE_HOME); break;
            case 1: set_content_page(PAGE_SETTINGS); break;
            case 2: set_content_page(PAGE_MAP); break;
            case 3: set_content_page(PAGE_TOWER_BATTLE); break;
            case 4: set_content_page(PAGE_SHOP); break;
            case 5: set_content_page(PAGE_LEVELUP); break;
            case 6: set_content_page(PAGE_SECRET); break;
            case 7: set_content_page(PAGE_STATS); break;
            default: //
                ESP_LOGW(TAG, "Menu item %d not implemented", i);
                break;
        }
    }
}

// Function to create the pop-up menu
static void create_menu(lv_obj_t *parent) {
    // Main outer container
    menu_container = lv_obj_create(parent);
    lv_obj_set_size(menu_container, LV_HOR_RES - 34, LV_VER_RES);
    lv_obj_align(menu_container, LV_ALIGN_LEFT_MID, 34, 0);
    lv_obj_set_style_pad_left(menu_container, 16, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(menu_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(menu_container, 0, LV_PART_MAIN);

    // Inner visible container for the menu items
    lv_obj_t *menu_box = lv_obj_create(menu_container);
    lv_obj_set_size(menu_box, lv_pct(100), lv_pct(100));
    lv_obj_align(menu_box, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_top(menu_box, 2, LV_PART_MAIN);
    lv_obj_set_style_bg_color(menu_box, lv_color_hex(GRAY_SHADE_4), LV_PART_MAIN);
    lv_obj_set_style_border_width(menu_box, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(menu_box, lv_color_hex(GRAY_TINT_2), LV_PART_MAIN);
    lv_obj_set_style_radius(menu_box, 3, LV_PART_MAIN);
    lv_obj_set_style_opa(menu_box, LV_OPA_COVER, LV_PART_MAIN);

    // Create a triangle to point to the status label
    const uint32_t triangle_width  = 16;
    const uint32_t triangle_height = 30;
    LV_DRAW_BUF_DEFINE_STATIC(triangle_buf, 16, 30, LV_COLOR_FORMAT_ARGB8888);
    LV_DRAW_BUF_INIT_STATIC(triangle_buf);

    lv_obj_t *triangle_canvas = lv_canvas_create(menu_container);
    lv_canvas_set_draw_buf(triangle_canvas, &triangle_buf);
    lv_obj_set_size(triangle_canvas, triangle_width, triangle_height);
    lv_canvas_fill_bg(triangle_canvas, lv_color_hex(GRAY_TINT_2), LV_OPA_TRANSP);

    lv_layer_t triangle_layer;
    lv_canvas_init_layer(triangle_canvas, &triangle_layer);

    lv_point_precise_t triangle_points[] = {{0, triangle_height / 2}, {triangle_width, 0}, {triangle_width, triangle_height}};
    lv_draw_triangle_dsc_t triangle_dsc;
    lv_draw_triangle_dsc_init(&triangle_dsc);
    triangle_dsc.bg_opa   = lv_obj_get_style_opa(menu_box, LV_PART_MAIN);
    triangle_dsc.bg_color = lv_obj_get_style_border_color(menu_box, LV_PART_MAIN);
    triangle_dsc.p[0]     = triangle_points[0];
    triangle_dsc.p[1]     = triangle_points[1];
    triangle_dsc.p[2]     = triangle_points[2];
    lv_draw_triangle(&triangle_layer, &triangle_dsc);

    lv_canvas_finish_layer(triangle_canvas, &triangle_layer);

    lv_obj_align_to(triangle_canvas, menu_box, LV_ALIGN_OUT_LEFT_MID, 0, 0);

    // Create buttons or labels for each screen selection
    for (int i = 0, c = 0; i < sizeof(menu_items) / sizeof(menu_items[0]); i++) {
        const char *item_text = menu_items[i];
        if (item_text == NULL) {
            // Enable the secret menu item if the badge team flag is set
            if (i == PAGE_SECRET /* && badge_config.badge_team */) { // Post-con: just let everyone access this... it's just the
                                                                     // tower IR simulator anyway
                // item_text = "Badge Team";
                item_text = "Extras";
            } else {
                continue;
            }
        }
        lv_obj_t *btn = lv_button_create(menu_box);
        lv_obj_set_size(btn, lv_pct(100), 30);
        lv_obj_add_style(btn, &button_blue, LV_PART_MAIN);
        lv_obj_set_style_text_color(btn, lv_color_hex(MENU_BUTTON_TEXT), LV_PART_MAIN);
        lv_obj_set_style_border_width(btn, 2, LV_PART_MAIN);
        lv_obj_set_style_radius(btn, 0, LV_PART_MAIN);
        lv_obj_set_style_text_font(btn, &cyberphont3b_16, LV_PART_MAIN);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, c * 50 + 10);

        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, item_text);
        lv_obj_center(label);

        lv_obj_add_event_cb(btn, menu_button_handler, LV_EVENT_CLICKED, (void *)i);
        ++c;
    }
}

static void minibadge_icon_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        // set_content_page(PAGE_MINIBADGE);
        ESP_LOGD(TAG, "Minibadge icon clicked");
    }
}

static void alert_icon_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        set_content_page(PAGE_TOWER_BATTLE);
    }
}

static void alert_flash_timer_cb(lv_timer_t *timer) {
    (void)timer;
    if (alert_icon == NULL) {
        return;
    }

    if (alert_icon_visible) {
        lv_obj_clear_flag(alert_icon, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(alert_icon, LV_OBJ_FLAG_HIDDEN);
    }
    alert_icon_visible = !alert_icon_visible;
}
