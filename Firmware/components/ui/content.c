#include "content.h"
#include "display.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "theme.h"

// Main screen components
#include "statusbar.h"

// Content area screens
#include "pages/home.h"
#include "pages/levelup.h"
#include "pages/map.h"
#include "pages/secret.h"
#include "pages/settings.h"
#include "pages/shop.h"
#include "pages/stats.h"
#include "pages/tower_battle.h"

static const char *TAG = "screens/main [content]";

// Content labels
const char *content_labels[] = {
    "Home", "Settings", "Map", "Battle", "Shop", "Level Up", "Secret", "Stats",
};

// Keep track of the content area
lv_obj_t *content_area = NULL;

// Content state
static content_state_t content_state = {0};

lv_obj_t *create_content_area(lv_obj_t *parent) {
    content_area = lv_obj_create(parent);
    lv_obj_set_size(content_area, LV_HOR_RES - 40, LV_VER_RES);
    lv_obj_set_style_bg_color(content_area, lv_color_hex(CONTENT_BG), LV_PART_MAIN);
    lv_obj_set_style_border_width(content_area, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(content_area, 0, LV_PART_MAIN);
    lv_obj_align(content_area, LV_ALIGN_RIGHT_MID, 0, 0);

    content_state.page = PAGE_HOME;

    return content_area;
}

void set_content_page(content_page_t page) {
    if (content_state.page == page) {
        return;
    }

    content_state.page = page;

    // Make sure we call render_content from within the LVGL context
    lv_async_call(render_content, NULL);
    vTaskDelay(pdMS_TO_TICKS(10));
}

content_page_t get_content_page() {
    return content_state.page;
}

void render_content() {
    // Set the status label
    if (content_state.page < sizeof(content_labels) / sizeof(content_labels[0])) {
        ESP_LOGD(TAG, "Setting status label to: %s", content_labels[content_state.page]);
        set_status_label(content_labels[content_state.page]);
    }

    lv_obj_clean(content_area);

    // Render the appropriate content
    switch (content_state.page) {
        case PAGE_HOME: //
            home_page_create(content_area);
            break;
        case PAGE_SETTINGS: //
            settings_page_create(content_area);
            break;
        case PAGE_MAP: //
            map_page_create(content_area);
            break;
        case PAGE_TOWER_BATTLE: //
            tower_battle_page_create(content_area);
            break;
        case PAGE_SHOP: //
            shop_page_create(content_area);
            break;
        case PAGE_LEVELUP: //
            levelup_page_create(content_area);
            break;
        case PAGE_SECRET: //
            secret_page_create(content_area);
            break;
        case PAGE_STATS: //
            stats_page_create(content_area);
            break;
        default: //
            ESP_LOGW(TAG, "Unknown content page: %d", content_state.page);
            break;
    }
}