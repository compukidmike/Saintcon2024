#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"
#include "lvgl.h"

typedef enum {
    PAGE_HOME,
    PAGE_SETTINGS,
    PAGE_MAP,
    PAGE_TOWER_BATTLE,
    PAGE_SHOP,
    PAGE_LEVELUP,
    PAGE_SECRET,
    PAGE_STATS,
} content_page_t;

typedef struct {
    content_page_t page;
} content_state_t;

extern const char *content_labels[];

// Create a container for the main content on the right side of the main screen
lv_obj_t *create_content_area(lv_obj_t *parent);

// Set the content to display
void set_content_page(content_page_t page);

// Get the current content page
content_page_t get_content_page();

// Content rendering
void render_content();

#ifdef __cplusplus
}
#endif
