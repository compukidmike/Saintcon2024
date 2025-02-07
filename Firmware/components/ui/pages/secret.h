#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_event.h"
#include "lvgl.h"

#include "api.h"

// Secret page event types
ESP_EVENT_DECLARE_BASE(SECRET_EVENT);
typedef enum {
    SECRET_EVENT_TOWER_SHOW_IR_SIMULATOR,
} secret_event_t;

// Create the secret menu for badge team stuff
void secret_page_create(lv_obj_t *parent);

#ifdef __cplusplus
}
#endif
