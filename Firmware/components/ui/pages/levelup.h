#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_event.h"
#include "lvgl.h"

#include "api.h"

// Level up event types
ESP_EVENT_DECLARE_BASE(LEVELUP_EVENT);
typedef enum {
    LEVELUP_EVENT_SEND_LEVELUP,
} levelup_event_t;

// Create the level up page
void levelup_page_create(lv_obj_t *parent);

// Handler for IR level up codes - called by `route_high_priority_code(...)` in `ir.c`
void handle_levelup_code(ir_code_t *ir_code);

#ifdef __cplusplus
}
#endif
