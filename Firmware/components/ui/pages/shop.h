#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_event.h"
#include "lvgl.h"

// Shop event types
ESP_EVENT_DECLARE_BASE(SHOP_EVENT);
typedef enum {
    SHOP_EVENT_INIT,
    SHOP_EVENT_SHOW_ITEM,
    SHOP_EVENT_PURCHASE_ITEM_START,
    SHOP_EVENT_PURCHASE_ITEM_SUCCESS,
    SHOP_EVENT_PURCHASE_ITEM_FAILURE,
} shop_event_t;

// Create the shop page
void shop_page_create(lv_obj_t *parent);

#ifdef __cplusplus
}
#endif
