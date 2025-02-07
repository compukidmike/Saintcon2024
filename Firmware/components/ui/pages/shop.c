#include <memory.h>

#include "esp_log.h"

#include "api.h"
#include "display.h"
#include "shop.h"
#include "theme.h"
#include "../loadanim.h"
#include "../components/modal.h"

static const char *TAG = "pages/shop";

ESP_EVENT_DEFINE_BASE(SHOP_EVENT);

typedef enum {
    SHOP_PAGE_INIT,      // Initialize the page - list all available items
    SHOP_PAGE_SHOW_ITEM, // Show a modal with details about a specific item in the shop
} shop_page_state_t;

typedef struct {
    lv_obj_t *container;
    lv_obj_t *item_list;
    lv_obj_t *loading_dots;
    shop_page_state_t state;
    esp_event_loop_handle_t event_loop;
    api_vend_items_t *items;
} shop_page_t;

static shop_page_t page = {0};

typedef enum {
    SHOP_BUTTON_CLOSE,
    SHOP_BUTTON_BUY,
} shop_button_t;

// Forward declarations
static void shop_page_cleanup(lv_event_t *event);

static void on_shop_event(void *context, esp_event_base_t base, int32_t id, void *data);

static void shop_page_item_click_handler(lv_event_t *event);
static void shop_page_details_click_handler(lv_event_t *event);
static void render_item_list();
static void render_item_details(api_vend_item_t *item);
static void render_purchase_result(bool success);

void shop_page_create(lv_obj_t *parent) {
    // Clear the content area
    lv_obj_clean(parent);

    page.container = lv_obj_create(parent);
    lv_obj_set_size(page.container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(page.container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(page.container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(page.container, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(page.container, 0, LV_PART_MAIN);
    lv_obj_remove_flag(page.container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(page.container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(page.container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(page.container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_event_cb(page.container, shop_page_cleanup, LV_EVENT_DELETE, NULL);

    // Create the event loop
    esp_event_loop_args_t loop_args = {
        .queue_size      = 10,
        .task_name       = "shop_async_events",
        .task_stack_size = 4096,
        .task_priority   = 5,
    };
    esp_event_loop_create(&loop_args, &page.event_loop);
    esp_event_handler_register_with(page.event_loop, SHOP_EVENT, ESP_EVENT_ANY_ID, on_shop_event, NULL);

    // Show a loading animation
    loading_dots_anim_config_t config = {
        .parent            = page.container,
        .align_to          = NULL,
        .x_offset          = 0,
        .y_offset          = 0,
        .dot_color         = lv_color_hex(WHITE),
        .dot_size          = 10,
        .fade_in_duration  = 250,
        .fade_out_duration = 250,
        .sequence_delay    = 200,
        .repeat_delay      = 500,
    };
    page.loading_dots = loading_dots_anim(&config);

    esp_event_post_to(page.event_loop, SHOP_EVENT, SHOP_EVENT_INIT, NULL, 0, 0);
}

static void shop_page_cleanup(lv_event_t *event) {
    lv_event_code_t code = lv_event_get_code(event);
    if (code == LV_EVENT_DELETE && page.container != NULL) {
        esp_event_loop_delete(page.event_loop);
        api_free_result_data(page.items, API_VEND_ITEMS);
        memset(&page, 0, sizeof(shop_page_t));
    }
}

static void on_shop_event(void *context, esp_event_base_t base, int32_t id, void *data) {
    if (base != SHOP_EVENT) {
        return;
    }

    switch (id) {
        case SHOP_EVENT_INIT: {
            api_result_t *result = api_vend_get_items();
            if (result != NULL && result->status == true) {
                page.items = (api_vend_items_t *)result->data;
            }
            api_free_result(result, false);

            lv_async_call(render_item_list, NULL);
            break;
        }
        case SHOP_EVENT_SHOW_ITEM: {
            api_vend_item_t *item = *(api_vend_item_t **)data;
            lv_async_call(render_item_details, item);
            break;
        }
        case SHOP_EVENT_PURCHASE_ITEM_START: {
            api_vend_item_t *item = *(api_vend_item_t **)data;
            ESP_LOGD(TAG, "Purchasing item: %s", item->item_name);

            // Show a loading modal
            lv_obj_t *modal = NULL;
            if (lvgl_lock(pdMS_TO_TICKS(100), __FILE__, __LINE__)) {
                modal = loading_modal("Purchasing item...");
                lvgl_unlock(__FILE__, __LINE__);
                vTaskDelay(2);
            }

            // Simulate a purchase
            api_result_t *result = api_vend_buy_item(item->item_id);
            if (result != NULL && result->status == true) {
                esp_event_post_to(page.event_loop, SHOP_EVENT, SHOP_EVENT_PURCHASE_ITEM_SUCCESS, NULL, 0, 0);
            } else {
                esp_event_post_to(page.event_loop, SHOP_EVENT, SHOP_EVENT_PURCHASE_ITEM_FAILURE, NULL, 0, 0);
            }
            api_free_result(result, false);

            // Close the modal
            lv_obj_delete_async(modal);
            break;
        }
        case SHOP_EVENT_PURCHASE_ITEM_SUCCESS: {
            ESP_LOGD(TAG, "Purchased item successfully");
            lv_async_call(render_purchase_result, true);
            break;
        }
        case SHOP_EVENT_PURCHASE_ITEM_FAILURE: {
            ESP_LOGD(TAG, "Failed to purchase item");
            lv_async_call(render_purchase_result, false);
            break;
        }
    }
}

static void shop_page_item_click_handler(lv_event_t *event) {
    api_vend_item_t *item = (api_vend_item_t *)lv_event_get_user_data(event);
    ESP_LOGD(TAG, "Item clicked: %s", item->item_name);
    esp_event_post_to(page.event_loop, SHOP_EVENT, SHOP_EVENT_SHOW_ITEM, (void *)&item, sizeof(api_vend_item_t *), 0);
}

static void shop_page_details_click_handler(lv_event_t *event) {
    lv_event_code_t code = lv_event_get_code(event);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *button           = lv_event_get_target(event);
        lv_obj_t *button_container = lv_obj_get_parent(button);
        lv_obj_t *modal            = lv_obj_get_user_data(button_container);
        shop_button_t button_type  = (shop_button_t)lv_obj_get_user_data(button);
        api_vend_item_t *item      = (api_vend_item_t *)lv_event_get_user_data(event);

        // Close the modal
        lv_obj_delete(modal);

        if (button_type == SHOP_BUTTON_BUY) {
            esp_event_post_to(page.event_loop, SHOP_EVENT, SHOP_EVENT_PURCHASE_ITEM_START, (void *)&item,
                              sizeof(api_vend_item_t *), 0);
        }
    }
}

static void render_item_list() {
    if (page.item_list != NULL && lv_obj_is_valid(page.item_list)) {
        lv_obj_delete(page.item_list);
        page.item_list = NULL;
    }
    lv_obj_clean(page.container);

    // Let the user know if we couldn't fetch the items or if there are none available
    if (page.items == NULL || page.items->count == 0) {
        lv_obj_t *label = lv_label_create(page.container);
        lv_label_set_text(label, page.items == NULL ? "Couldn't fetch items" : "No items found");
        lv_obj_set_style_text_font(label, &cyberphont3b_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(label, lv_color_hex(WHITE), LV_PART_MAIN);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_flex_align(label, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        return;
    }

    // Create a list of items and their prices
    lv_obj_t *list = lv_list_create(page.container);
    lv_obj_set_size(list, lv_pct(100), lv_pct(100));
    lv_obj_set_style_radius(list, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(list, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(list, lv_color_hex(CONTENT_BG), LV_PART_MAIN);
    lv_obj_set_style_text_color(list, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_flex_grow(list, 1);
    page.item_list = list;

    // Add each item to the list
    for (int i = 0; i < page.items->count; i++) {
        api_vend_item_t *item = &page.items->items[i];
        lv_obj_t *button      = lv_list_add_button(list, NULL, item->item_name);

        // Style and align the label
        lv_obj_t *label_part = lv_obj_get_child_by_type(button, 0, &lv_label_class);
        lv_obj_set_style_pad_ver(label_part, 7, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(button, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_color(button, lv_color_hex(WHITE), LV_PART_MAIN);
        lv_obj_set_style_text_align(button, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
        lv_obj_set_style_text_font(button, &cyberphont3b_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(button, lv_color_hex(WHITE), LV_PART_MAIN);

        // Add the price label
        lv_obj_t *price_label = lv_label_create(button);
        lv_label_set_text_fmt(price_label, "$%d", (int)item->item_price);
        lv_obj_set_style_pad_ver(price_label, 7, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(price_label, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_color(price_label, lv_color_hex(WHITE), LV_PART_MAIN);
        lv_obj_set_style_text_align(price_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
        lv_obj_set_style_text_font(price_label, &cyberphont3b_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(price_label, lv_color_hex(GRAY_TINT_5), LV_PART_MAIN);

        // Add an event handler
        lv_obj_add_event_cb(button, shop_page_item_click_handler, LV_EVENT_CLICKED, item);
    }
}

static void render_item_details(api_vend_item_t *item) {
    lv_obj_t *modal = modal_create_base(NULL, true);

    // Add a title
    lv_obj_t *title = lv_label_create(modal);
    lv_obj_set_width(title, lv_pct(100));
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_text(title, "Item Details");
    lv_obj_set_style_text_font(title, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_hex(BLACK), LV_PART_MAIN);
    lv_obj_set_flex_grow(title, 0);

    // Add a container for the item details
    lv_obj_t *details_container = lv_obj_create(modal);
    lv_obj_set_size(details_container, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(details_container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(details_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_radius(details_container, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(details_container, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(details_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(details_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_remove_flag(details_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(details_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_grow(details_container, 1);

    // Add a label for the item name
    lv_obj_t *name_label = lv_label_create(details_container);
    lv_obj_set_width(name_label, lv_pct(100));
    lv_label_set_text(name_label, item->item_name);
    lv_obj_set_style_text_font(name_label, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(name_label, lv_color_hex(BLACK), LV_PART_MAIN);
    lv_obj_set_style_text_align(name_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_flex_grow(name_label, 0);

    // Add a label for the item price
    lv_obj_t *price_label = lv_label_create(details_container);
    lv_obj_set_width(price_label, lv_pct(100));
    lv_label_set_text_fmt(price_label, "$%d", (int)item->item_price);
    lv_obj_set_style_text_font(price_label, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(price_label, lv_color_hex(GRAY_TINT_5), LV_PART_MAIN);
    lv_obj_set_style_text_align(price_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_flex_grow(price_label, 0);

    if (item->sold_out) {
        lv_obj_t *sold_out_label = lv_label_create(details_container);
        lv_label_set_text(sold_out_label, "Sold Out");
        lv_obj_set_style_text_font(sold_out_label, &cyberphont3b_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(sold_out_label, lv_color_hex(RED_MAIN), LV_PART_MAIN);
        lv_obj_set_style_text_align(sold_out_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_flex_grow(sold_out_label, 0);
    }

    // Add a container for the close and buy buttons
    lv_obj_t *buttons = lv_obj_create(modal);
    lv_obj_set_size(buttons, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(buttons, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(buttons, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(buttons, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(buttons, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(buttons, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(buttons, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_flex_grow(buttons, 0);
    lv_obj_set_user_data(buttons, modal);

    // Add a close button
    lv_obj_t *close_btn = lv_button_create(buttons);
    lv_obj_add_style(close_btn, &button_style, LV_PART_MAIN);
    lv_obj_add_style(close_btn, &button_orange, LV_PART_MAIN);
    lv_obj_set_user_data(close_btn, (void *)SHOP_BUTTON_CLOSE);
    lv_obj_add_event_cb(close_btn, shop_page_details_click_handler, LV_EVENT_CLICKED, item);

    lv_obj_t *close_label = lv_label_create(close_btn);
    lv_label_set_text(close_label, "Close");
    lv_obj_center(close_label);

    // Add a buy button
    lv_obj_t *buy_btn = lv_button_create(buttons);
    lv_obj_add_style(buy_btn, &button_style, LV_PART_MAIN);
    lv_obj_add_style(buy_btn, &button_green, LV_PART_MAIN);
    lv_obj_set_user_data(buy_btn, (void *)SHOP_BUTTON_BUY);
    lv_obj_add_event_cb(buy_btn, shop_page_details_click_handler, LV_EVENT_CLICKED, item);

    lv_obj_t *buy_label = lv_label_create(buy_btn);
    lv_label_set_text(buy_label, "Buy");
    lv_obj_center(buy_label);

    if (item->sold_out) {
        lv_obj_set_state(buy_btn, LV_STATE_DISABLED, true);
    }
}

static void render_purchase_result(bool success) {
    lv_obj_t *modal = modal_create_base(NULL, true);

    // Container for the label to center it
    lv_obj_t *label_container = lv_obj_create(modal);
    lv_obj_set_size(label_container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(label_container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(label_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_radius(label_container, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(label_container, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(label_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(label_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_flex_grow(label_container, 1);

    lv_obj_t *label = lv_label_create(label_container);
    lv_obj_set_width(label, lv_pct(100));
    lv_label_set_text(label, success ? "Purchase successful" : "Purchase failed");
    lv_obj_set_style_text_font(label, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(BLACK), LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    // Add a close button
    lv_obj_t *close_btn = lv_button_create(modal);
    lv_obj_add_style(close_btn, &button_style, LV_PART_MAIN);
    lv_obj_add_style(close_btn, &button_orange, LV_PART_MAIN);
    lv_obj_add_event_cb(close_btn, base_modal_exit_cb, LV_EVENT_CLICKED, modal);

    lv_obj_t *close_label = lv_label_create(close_btn);
    lv_label_set_text(close_label, "OK");
    lv_obj_center(close_label);
}
