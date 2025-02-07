#include <memory.h>
#include "esp_event.h"
#include "esp_log.h"

#include "badge.h"
#include "secret.h"
#include "theme.h"
#include "ui.h"
#include "components/modal.h"

static const char *TAG = "pages/secret";

ESP_EVENT_DEFINE_BASE(SECRET_EVENT);

typedef enum {
    SECRET_MENU_ITEM_TOWER_IR_SIMULATOR,
} secret_menu_item_t;
static const char *secret_menu_item_str[] = {
    "Tower IR Simulator",
};
static const int secret_menu_item_count = sizeof(secret_menu_item_str) / sizeof(secret_menu_item_str[0]);

typedef enum {
    SECRET_PAGE_INIT,
    SECRET_PAGE_TOWER_IR_SIMULATOR, // Tower IR simulator - list all the towers and when one is tapped, send the IR code for the
                                    // tower
} secret_page_state_t;

typedef struct {
    lv_obj_t *container;
    lv_obj_t *menu_list;
    secret_page_state_t state;
    secret_menu_item_t selected_menu_item;
    esp_event_loop_handle_t event_loop;
} secret_page_t;

static secret_page_t page = {0};

// Function prototypes
static void secret_menu_item_event_handler(lv_event_t *event);
static void secret_page_cleanup(lv_event_t *event);

static void on_secret_event(void *context, esp_event_base_t base, int32_t id, void *data);

static void render_state_menu();
static void render_state_tower_ir_simulator();

void secret_page_create(lv_obj_t *parent) {
    page.container = lv_obj_create(parent);
    lv_obj_set_size(page.container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_radius(page.container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(page.container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(page.container, 0, LV_PART_MAIN);
    lv_obj_remove_flag(page.container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(page.container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(page.container, secret_menu_item_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(page.container, secret_page_cleanup, LV_EVENT_DELETE, NULL);

    // Create the event loop
    esp_event_loop_args_t loop_args = {
        .queue_size      = 10,
        .task_name       = "secret_async_events",
        .task_stack_size = 4096,
        .task_priority   = 5,
    };
    esp_event_loop_create(&loop_args, &page.event_loop);
    esp_event_handler_register_with(page.event_loop, SECRET_EVENT, ESP_EVENT_ANY_ID, on_secret_event, NULL);

    render_state_menu();
}

static void secret_menu_item_event_handler(lv_event_t *event) {
    lv_event_code_t code = lv_event_get_code(event);
    if (code == LV_EVENT_CLICKED) {
        secret_menu_item_t item = (secret_menu_item_t)lv_event_get_user_data(event);
        page.selected_menu_item = item;
        switch (item) {
            case SECRET_MENU_ITEM_TOWER_IR_SIMULATOR:
                esp_event_post_to(page.event_loop, SECRET_EVENT, SECRET_EVENT_TOWER_SHOW_IR_SIMULATOR, NULL, 0, 0);
                break;
        }
    }
}

static void secret_page_cleanup(lv_event_t *event) {
    if (page.container != NULL) {
        esp_event_loop_delete(page.event_loop);
        memset(&page, 0, sizeof(secret_page_t));
    }
}

static void on_secret_event(void *context, esp_event_base_t base, int32_t id, void *data) {
    switch (id) {
        case SECRET_EVENT_TOWER_SHOW_IR_SIMULATOR: //
            lv_async_call(render_state_tower_ir_simulator, NULL);
            break;
    }
}

static void render_state_menu() {
    if (page.menu_list != NULL) {
        lv_obj_delete(page.menu_list);
    }

    lv_obj_t *menu_list = lv_list_create(page.container);
    lv_obj_set_size(menu_list, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(menu_list, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(menu_list, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(menu_list, lv_color_hex(CONTENT_BG), LV_PART_MAIN);
    lv_obj_set_style_border_width(menu_list, 0, LV_PART_MAIN);
    page.menu_list = menu_list;

    for (int i = 0; i < secret_menu_item_count; i++) {
        lv_obj_t *item = lv_list_add_button(menu_list, NULL, secret_menu_item_str[i]);

        lv_obj_t *label = lv_obj_get_child_by_type(item, 0, &lv_label_class);
        lv_obj_set_style_pad_ver(label, 7, LV_PART_MAIN);

        lv_obj_set_style_bg_color(item, lv_color_hex(CONTENT_BG), LV_PART_MAIN);
        lv_obj_set_style_border_width(item, 2, LV_PART_MAIN);
        lv_obj_set_style_border_color(item, lv_color_hex(WHITE), LV_PART_MAIN);
        lv_obj_set_style_text_font(item, &cyberphont3b_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(item, lv_color_hex(WHITE), LV_PART_MAIN);

        lv_obj_add_event_cb(item, secret_menu_item_event_handler, LV_EVENT_CLICKED, NULL);
    }
}

static void tower_ir_button_event_handler(lv_event_t *event) {
    lv_event_code_t code = lv_event_get_code(event);
    if (code == LV_EVENT_CLICKED) {
        int tower_id = lv_event_get_user_data(event);
        ESP_LOGI(TAG, "Tower IR button clicked: %d", tower_id);
        int idx = tower_id_to_idx(tower_id);
        if (idx != -1) {
            ESP_LOGI(TAG, "Tower IR code: %lu", tower_state[idx].info->ir_code);
            ir_transmit(tower_state[idx].info->ir_code >> 16, tower_state[idx].info->ir_code & 0xFFFF);
        }
    }
}

static void render_state_tower_ir_simulator() {
    lv_obj_t *modal     = modal_create_base(NULL, true);
    lv_obj_t *container = lv_obj_create(modal);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_radius(container, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(container, 0, LV_PART_MAIN);
    lv_obj_set_flex_grow(container, 1);

    // If we haven't refreshed the towers yet, try to do that now
    if (tower_state[0].info == NULL) {
        tower_info_refresh(REFRESH_ALL);
    }

    // Make a container for the buttons
    lv_obj_t *buttons = lv_obj_create(container);
    lv_obj_set_size(buttons, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(buttons, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(buttons, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_radius(buttons, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(buttons, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(buttons, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(buttons, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

    // Make buttons for each tower
    lv_style_t *button_styles[] = {
        &button_red, &button_orange, &button_yellow, &button_green, &button_blue, &button_purple, &button_bluegrey, &button_grey,
    };
    for (int i = 0; i < TOTAL_TOWERS; i++) {
        lv_obj_t *button = lv_btn_create(buttons);
        lv_obj_set_width(button, lv_pct(100));
        lv_obj_add_style(button, &button_style, LV_PART_MAIN);
        lv_obj_add_style(button, button_styles[i % (sizeof(button_styles) / sizeof(button_styles[0]))], LV_PART_MAIN);

        lv_obj_t *label = lv_label_create(button);
        lv_label_set_text_fmt(label, "%s", tower_state[i].info->name);
        lv_obj_set_style_text_font(label, &cyberphont3b_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(label, lv_color_hex(WHITE), LV_PART_MAIN);

        lv_obj_add_event_cb(button, tower_ir_button_event_handler, LV_EVENT_CLICKED, tower_state[i].info->id);
    }
}
