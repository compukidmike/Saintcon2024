#include <memory.h>
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"

#include "api.h"
#include "attack.h"
#include "display.h"
#include "minibadge.h"
#include "theme.h"

static const char *TAG = "components/attack";

#define MAX_ARROWS 8

// Define colors
#define ARROW_NEUTRAL_COLOR   lv_color_hex(GRAY_SHADE_2)
#define ARROW_CORRECT_COLOR   lv_color_hex(GREEN_MAIN)
#define ARROW_INCORRECT_COLOR lv_color_hex(RED_MAIN)
#define DPAD_BUTTON_BG        lv_color_hex(GREEN_LIGHT)
#define DPAD_BUTTON_BORDER    lv_color_hex(GREEN_DIM)
#define DPAD_BUTTON_TEXT      lv_color_hex(GREEN_DIMMEST)

// Badge-local power-up info we need to keep track of
battle_powerup_info_t battle_powerup_info[] = {
    {POWER_UP_NONE, -1},
    {POWER_UP_DPAD, -1},
    {POWER_UP_LOOT, -1},
    {POWER_UP_STRAT_SIMPLIFY, -1},
    {POWER_UP_SPELLCHECK, 1},
    {POWER_UP_QUAD_DAMAGE, -1},
    {POWER_UP_GLASS_CANNON, -1},
    {POWER_UP_SAVING_THROW, 1},
    {POWER_UP_REVERSE_SHELL, -1},
    {POWER_UP_HYDRA, -1},
    {POWER_UP_ZODIAKS_SANDALS, -1},
    {POWER_UP_GREED, -1},
};

// Define arrow enum
typedef enum {
    ARROW_LEFT,
    ARROW_UP,
    ARROW_RIGHT,
    ARROW_DOWN,
} arrow_t;

typedef struct {
    attack_config_t config;
    lv_obj_t *parent;
    lv_obj_t *current_screen;
    lv_obj_t *title_label;
    lv_obj_t *stratagem_label;
    lv_obj_t *stratagem_container;
    lv_obj_t *stratagem_arrows[MAX_ARROWS];
    lv_obj_t *tower_health;
    lv_obj_t *player_health;
    arrow_t current_stratagem[MAX_ARROWS];
    uint16_t stratagem_length; // Randomized between config.stratagem_min and config.stratagem_max
    uint16_t current_index;
    uint16_t correct_stratagems;
    uint16_t failed_stratagems;
    int64_t start_time;
    bool dpad_enabled;
} attack_t;

typedef struct {
    attack_t *attack;
    arrow_t arrow;
} attack_input_t;

// Function prototypes
static void create_tower_ui(attack_t *attack);
// static void create_pvp_ui(attack_t *attack);
static void update_stratagem(attack_t *attack);
static void check_input(attack_t *attack, arrow_t arrow);
static void attack_end(attack_t *attack, bool success);
static void attack_cleanup_event_cb(lv_event_t *e);
static void on_minibadge_dpad_event(void *arg, esp_event_base_t base, int32_t id, void *event_data);

// Map arrow to symbol
static const char *arrow_to_symbol(arrow_t arrow) {
    switch (arrow) {
        case ARROW_LEFT: return CLARITY_ARROW_LEFT;
        case ARROW_UP: return CLARITY_ARROW_UP;
        case ARROW_RIGHT: return CLARITY_ARROW_RIGHT;
        case ARROW_DOWN: return CLARITY_ARROW_DOWN;
        default: return "";
    }
}

// Callback for arrow button events
static void dpad_button_event_cb(lv_event_t *e) {
    lv_event_code_t code  = lv_event_get_code(e);
    attack_input_t *input = lv_event_get_user_data(e);

    switch (code) {
        case LV_EVENT_CLICKED:
            if (input != NULL) {
                ESP_LOGD(TAG, "Clicked arrow: %s", arrow_to_symbol(input->arrow));
                check_input(input->attack, input->arrow);
            } else {
                ESP_LOGE(TAG, "Input is NULL");
            }
            break;
        case LV_EVENT_DELETE:
            if (input != NULL) {
                free(input);
            }
            break;
        default: //
            break;
    }
}

static void close_button_event_cb(lv_event_t *e) {
    attack_t *attack = lv_event_get_user_data(e);
    lv_obj_delete_async(attack->current_screen);

    if (attack->config.exit_callback != NULL) {
        attack->config.exit_callback(attack->config.battle_id, false);
    }
}

/**
 * @brief Create a D-Pad button given a central anchor and an arrow direction
 *
 * @param anchor The central anchor object
 * @param arrow The arrow direction
 * @param user_data User data to be passed to the event callback
 * @return The created button object
 */
static lv_obj_t *create_dpad_button(lv_obj_t *anchor, arrow_t arrow, void *user_data) {
    ESP_LOGD(TAG, "Creating D-Pad button for arrow: %s", arrow_to_symbol(arrow));
    lv_obj_t *container = lv_obj_get_parent(anchor); // We need to position buttons outside of the anchor object
    lv_obj_t *button    = lv_button_create(container);
    lv_obj_t *label     = lv_label_create(button);

    // Add the arrow direction to the button
    lv_obj_set_user_data(button, (void *)arrow);

    // Set the button size
    const int32_t btn_size = 40;
    lv_obj_set_size(button, btn_size, btn_size);

    // Set the button style
    lv_obj_set_style_radius(button, 3, LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, DPAD_BUTTON_BG, LV_PART_MAIN);
    lv_obj_set_style_border_width(button, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(button, DPAD_BUTTON_BORDER, LV_PART_MAIN);
    lv_obj_set_style_text_color(button, DPAD_BUTTON_TEXT, LV_PART_MAIN);
    lv_obj_set_style_text_font(button, &clarity_24, LV_PART_MAIN);

    // Set the position relative to the anchor
    int32_t x_offset, y_offset;
    const int32_t btn_offset     = 7;
    const int32_t anchor_size    = 10;
    const int32_t container_size = btn_size * 2 + btn_offset * 2 + anchor_size + 4; // +4 for 2px padding on the container
    // clang-format off
    switch (arrow) {
        case ARROW_LEFT:    x_offset = 1;                                                       y_offset = container_size / 2 + btn_size / 2 - anchor_size / 2 - btn_offset / 2 - 10;    break;
        case ARROW_UP:      x_offset = container_size / 2 - anchor_size / 2 - btn_offset / 2;   y_offset = -btn_size / 2 +anchor_size;     break;
        case ARROW_RIGHT:   x_offset = container_size - btn_size;                               y_offset = container_size / 2 - btn_size / 2 - 1;    break;
        case ARROW_DOWN:    x_offset = 1;                                                       y_offset = container_size - btn_size + btn_offset + 1;      break;
    }
    // clang-format on
    lv_obj_align_to(button, anchor, LV_ALIGN_CENTER, x_offset, y_offset - 12);

    // Set the label text
    lv_label_set_text(label, arrow_to_symbol(arrow));

    // Offsets to fix the arrow text so it's centered
    const int32_t arrow_center_offsets[4][2] = {
        {2, -1}, // Left
        {2, -3}, // Up
        {2, -1}, // Right
        {2, -3}, // Down
    };
    lv_obj_align(label, LV_ALIGN_CENTER, arrow_center_offsets[arrow][0], arrow_center_offsets[arrow][1]);

    // Create a struct to pass data to the event callback
    attack_input_t *input = calloc(1, sizeof(attack_input_t));
    if (!input) {
        ESP_LOGE(TAG, "Failed to create attack input - couldn't allocate memory");
        return NULL;
    }
    input->attack = user_data;
    input->arrow  = arrow;

    // Add event callback for the button and pass the input struct
    lv_obj_add_event_cb(button, dpad_button_event_cb, LV_EVENT_ALL, input);

    return button;
}

lv_obj_t *attack_create(const attack_config_t *config) {
    // Make sure the config is valid
    if (config == NULL) {
        ESP_LOGE(TAG, "Failed to create attack UI - config is NULL");
        return NULL;
    } else if (esp_log_level_get(TAG) <= ESP_LOG_DEBUG) {
        // Debug: Print the config
        char buffer[256];
        snprintf(buffer, sizeof(buffer),
                 "Creating attack UI with config:\n"
                 "  battle_id: %lu\n"
                 "  mode: %s\n"
                 "  stratagem_min: %d\n"
                 "  stratagem_max: %d\n"
                 "  stratagem_count: %d\n"
                 "  tower_health: %d\n"
                 "  player_health: %d\n"
                 "  stratagem_callback: %p\n"
                 "  result_callback: %p\n"
                 "  exit_callback: %p\n",
                 config->battle_id, config->mode == ATTACK_MODE_TOWER ? "TOWER" : "PVP", config->stratagem_min,
                 config->stratagem_max, config->stratagem_count, config->tower_health, config->player_health,
                 config->stratagem_callback, config->result_callback, config->exit_callback);
        ESP_LOGD(TAG, "%s", buffer);
    }

    // Create an attack object to hold the attack state and tracked objects
    attack_t *attack = calloc(1, sizeof(attack_t));
    if (!attack) {
        ESP_LOGE(TAG, "Failed to create attack UI - couldn't allocate memory");
        return NULL;
    }

    // Initialize the attack object with the config
    memcpy(&attack->config, config, sizeof(attack_config_t));
    attack->current_index      = 0;
    attack->correct_stratagems = 0;
    attack->stratagem_length   = esp_random() % (config->stratagem_max - config->stratagem_min + 1) + config->stratagem_min;

    // Create the attack main object and have it fill the screen
    attack->current_screen = lv_obj_create(lv_screen_active());
    lv_obj_set_size(attack->current_screen, LV_HOR_RES, LV_VER_RES);
    lv_obj_align(attack->current_screen, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_scrollbar_mode(attack->current_screen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(attack->current_screen, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(attack->current_screen, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_style_border_width(attack->current_screen, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(attack->current_screen, lv_color_hex(ORANGE_DIM), LV_PART_MAIN);
    lv_obj_set_style_pad_all(attack->current_screen, 0, LV_PART_MAIN);
    lv_obj_set_user_data(attack->current_screen, attack);

    // Create a container for the labels
    lv_obj_t *label_container = lv_obj_create(attack->current_screen);
    lv_obj_set_size(label_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(label_container, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_opa(label_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_layout(label_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(label_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(label_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_top(label_container, 3, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(label_container, 10, LV_PART_MAIN);
    lv_obj_set_style_border_width(label_container, 0, LV_PART_MAIN);

    // Create a label for the attack title
    attack->title_label = lv_label_create(label_container);
    lv_label_set_text(attack->title_label, "Attack");
    lv_obj_set_style_text_font(attack->title_label, &clarity_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(attack->title_label, lv_color_hex(GRAY_SHADE_5), LV_PART_MAIN);

    // Create a label for the stratagem count
    attack->stratagem_label = lv_label_create(label_container);
    lv_label_set_text_fmt(attack->stratagem_label, "%d/%d", attack->correct_stratagems + 1, attack->config.stratagem_count);
    lv_obj_set_style_text_font(attack->stratagem_label, &clarity_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(attack->stratagem_label, lv_color_hex(GRAY_SHADE_5), LV_PART_MAIN);

    // Container for the stratagem arrows
    attack->stratagem_container = lv_obj_create(attack->current_screen);
    lv_obj_set_size(attack->stratagem_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(attack->stratagem_container, LV_ALIGN_TOP_MID, 0, 40);
    // lv_obj_align_to(attack->stratagem_container, label_container, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_layout(attack->stratagem_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(attack->stratagem_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(attack->stratagem_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_ver(attack->stratagem_container, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(attack->stratagem_container, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_top(attack->stratagem_container, 2, LV_PART_MAIN);
    lv_obj_set_style_bg_color(attack->stratagem_container, lv_color_hex(GRAY_TINT_7), LV_PART_MAIN);
    lv_obj_set_style_radius(attack->stratagem_container, 1, LV_PART_MAIN);
    lv_obj_set_style_border_width(attack->stratagem_container, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(attack->stratagem_container, lv_color_hex(GRAY_BASE), LV_PART_MAIN);

    // Add the stratagem arrows to the container
    for (int i = 0; i < attack->stratagem_length; i++) {
        attack->stratagem_arrows[i] = lv_label_create(attack->stratagem_container);
        lv_label_set_text(attack->stratagem_arrows[i], "-");
        lv_obj_set_style_text_align(attack->stratagem_arrows[i], LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_text_font(attack->stratagem_arrows[i], &clarity_20, LV_PART_MAIN);
        lv_obj_set_style_text_color(attack->stratagem_arrows[i], ARROW_NEUTRAL_COLOR, LV_PART_MAIN);
    }

    // Generate the initial stratagem
    update_stratagem(attack);

    // Create a container to hold the D-Pad buttons
    lv_obj_t *dpad_container = lv_obj_create(attack->current_screen);
    lv_obj_set_size(dpad_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(dpad_container, 2, LV_PART_MAIN);
    lv_obj_align(dpad_container, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_radius(dpad_container, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(dpad_container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(dpad_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_remove_flag(dpad_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(dpad_container, LV_SCROLLBAR_MODE_OFF);

    // Create a central anchor object for the D-Pad buttons
    lv_obj_t *dpad_anchor = lv_obj_create(dpad_container);
    lv_obj_set_size(dpad_anchor, 10, 10);
    lv_obj_set_style_pad_all(dpad_anchor, 0, LV_PART_MAIN);
    lv_obj_set_style_margin_all(dpad_anchor, 0, LV_PART_MAIN);
    lv_obj_align(dpad_anchor, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(dpad_anchor, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(dpad_anchor, lv_color_hex(GRAY_BASE), LV_PART_MAIN);
    lv_obj_set_style_border_width(dpad_anchor, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(dpad_anchor, lv_color_hex(GRAY_TINT_6), LV_PART_MAIN);

    // Create the D-Pad buttons
    create_dpad_button(dpad_anchor, ARROW_LEFT, attack);
    create_dpad_button(dpad_anchor, ARROW_UP, attack);
    create_dpad_button(dpad_anchor, ARROW_RIGHT, attack);
    create_dpad_button(dpad_anchor, ARROW_DOWN, attack);

    // Create additional attack UI elements based on the mode
    switch (config->mode) {
        case ATTACK_MODE_TOWER: //
            create_tower_ui(attack);
            break;
        case ATTACK_MODE_SAVING_THROW: //
            // For now we don't need to do anything special for saving throws
            break;
        case ATTACK_MODE_PVP: //
            break;
    }

    // Create a close button
    lv_obj_t *close_button = lv_button_create(attack->current_screen);
    lv_obj_set_size(close_button, 30, 30);
    lv_obj_align(close_button, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_set_style_bg_color(close_button, lv_color_hex(ORANGE_MAIN), LV_PART_MAIN);
    lv_obj_set_style_radius(close_button, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(close_button, close_button_event_cb, LV_EVENT_CLICKED, attack);
    lv_obj_t *close_label = lv_label_create(close_button);
    lv_label_set_text(close_label, LV_SYMBOL_LEFT);
    lv_obj_center(close_label);

    // Start the timer
    attack->start_time = esp_timer_get_time();

    // Add an event handler to clean up when the screen is deleted
    lv_obj_add_event_cb(attack->current_screen, attack_cleanup_event_cb, LV_EVENT_DELETE, attack);

    // Check the power-ups to see if we have a D-pad minibadge
    minibadge_slot_t minibadge_dpad_slots[MINIBADGE_SLOT_COUNT] = {0};
    int minibadge_dpad_count                                    = 0;
    for (int i = 0; i < MINIBADGE_SLOT_COUNT; i++) {
        ESP_LOGI(TAG, "Power-up [slot %d]: [%d] %s", i + 1, config->power_ups[i],
                 minibadge_buff_type_map[config->power_ups[i]].name);
        if (config->power_ups[i] == POWER_UP_DPAD) {
            minibadge_dpad_slots[minibadge_dpad_count++] = i;
        }
    }

    if (minibadge_dpad_count > 0) {
        // Enable polling for the D-pad minibadges
        for (int i = 0; i < minibadge_dpad_count; i++) {
            minibadge_dpad_poll(true, minibadge_dpad_slots[i]);
        }

        // Add an event handler to listen for D-pad events
        esp_event_handler_register_with(minibadge_event_loop_handle, MINIBADGE_DPAD_EVENT, MINIBADGE_DPAD_EVENT_PRESS,
                                        on_minibadge_dpad_event, attack);

        // Set the flag to indicate that the D-pad is enabled
        attack->dpad_enabled = true;
    }

    return attack->current_screen;
}

static void create_tower_ui(attack_t *attack) {

    static lv_style_t health_style_bg;
    lv_style_init(&health_style_bg);
    lv_style_set_border_color(&health_style_bg, lv_color_hex(GREEN_DIM));
    lv_style_set_border_width(&health_style_bg, 2);
    lv_style_set_pad_all(&health_style_bg, 4);
    lv_style_set_radius(&health_style_bg, 2);

    static lv_style_t health_style_indicator;
    lv_style_init(&health_style_indicator);
    lv_style_set_bg_opa(&health_style_indicator, LV_OPA_COVER);
    lv_style_set_bg_color(&health_style_indicator, lv_palette_main(LV_PALETTE_GREEN));
    lv_style_set_bg_grad_color(&health_style_indicator, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_bg_grad_dir(&health_style_indicator, LV_GRAD_DIR_VER);
    lv_style_set_radius(&health_style_indicator, 1);

    //
    // Tower health bar on the left side
    //

    lv_obj_t *tower_label = lv_label_create(attack->current_screen);
    lv_label_set_text(tower_label, "Tower Health");
    lv_obj_set_style_text_font(tower_label, &cyberphont3b_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(tower_label, lv_color_hex(GRAY_BASE), LV_PART_MAIN);
    lv_obj_set_style_transform_pivot_x(tower_label, 0, LV_PART_MAIN);
    lv_obj_set_style_transform_pivot_y(tower_label, lv_pct(50), LV_PART_MAIN);
    lv_obj_set_style_transform_rotation(tower_label, -900, LV_PART_MAIN);
    lv_obj_align(tower_label, LV_ALIGN_BOTTOM_LEFT, 20, -10);

    attack->tower_health = lv_bar_create(attack->current_screen);
    lv_obj_add_style(attack->tower_health, &health_style_bg, 0);
    lv_obj_add_style(attack->tower_health, &health_style_indicator, LV_PART_INDICATOR);
    lv_obj_set_size(attack->tower_health, 20, 125);
    lv_bar_set_range(attack->tower_health, 0, 100);
    lv_obj_align(attack->tower_health, LV_ALIGN_BOTTOM_LEFT, 27, -10);
    lv_bar_set_value(attack->tower_health, attack->config.tower_health, LV_ANIM_ON);

    //
    // Player health / connection status on the right side
    //

    lv_obj_t *player_status = lv_label_create(attack->current_screen);
    lv_label_set_text(player_status, "Connection");
    lv_obj_set_style_text_font(player_status, &cyberphont3b_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(player_status, lv_color_hex(GRAY_BASE), LV_PART_MAIN);
    lv_obj_set_style_transform_pivot_x(player_status, lv_pct(100), LV_PART_MAIN);
    lv_obj_set_style_transform_pivot_y(player_status, lv_pct(50), LV_PART_MAIN);
    lv_obj_set_style_transform_rotation(player_status, 900, LV_PART_MAIN);
    lv_obj_align(player_status, LV_ALIGN_BOTTOM_RIGHT, -20, -10);

    attack->player_health = lv_bar_create(attack->current_screen);
    lv_obj_add_style(attack->player_health, &health_style_bg, 0);
    lv_obj_add_style(attack->player_health, &health_style_indicator, LV_PART_INDICATOR);
    lv_obj_set_size(attack->player_health, 20, 125);
    lv_bar_set_range(attack->player_health, 0, 100);
    lv_obj_align(attack->player_health, LV_ALIGN_BOTTOM_RIGHT, -27, -10);
    lv_bar_set_value(attack->player_health, attack->config.player_health, LV_ANIM_ON);
}

static void update_stratagem(attack_t *attack) {
    const int32_t arrow_center_offsets[4][2] = {
        {1, 2}, // Left
        {1, 0}, // Up
        {1, 2}, // Right
        {1, 0}, // Down
    };
    for (int i = 0; i < attack->stratagem_length; i++) {
        attack->current_stratagem[i] = (arrow_t)(esp_random() % 4);
        lv_label_set_text(attack->stratagem_arrows[i], arrow_to_symbol(attack->current_stratagem[i]));
        lv_obj_set_style_margin_left(attack->stratagem_arrows[i], arrow_center_offsets[attack->current_stratagem[i]][0],
                                     LV_PART_MAIN);
        lv_obj_set_style_margin_top(attack->stratagem_arrows[i], arrow_center_offsets[attack->current_stratagem[i]][1],
                                    LV_PART_MAIN);
        lv_obj_set_style_text_color(attack->stratagem_arrows[i], ARROW_NEUTRAL_COLOR, LV_PART_MAIN);
    }
}

static void check_input(attack_t *attack, arrow_t arrow) {
    if (attack->current_stratagem[attack->current_index] == arrow) {
        lv_obj_set_style_text_color(attack->stratagem_arrows[attack->current_index], ARROW_CORRECT_COLOR, LV_PART_MAIN);
        lv_refr_now(NULL);
        attack->current_index++;
        if (attack->current_index == attack->stratagem_length) {
            attack->correct_stratagems++;
            attack->current_index = 0;
            bool continue_attack  = true;
            if (attack->config.stratagem_callback != NULL) {
                int64_t cb_time = esp_timer_get_time();
                continue_attack = attack->config.stratagem_callback(
                    attack->config.battle_id, attack->stratagem_length, attack->stratagem_length, attack->correct_stratagems,
                    attack->failed_stratagems, (cb_time - attack->start_time) / 1000);
                // Adjust the start time to account for the time taken by the callback since it may have taken a while to run
                attack->start_time += esp_timer_get_time() - cb_time;
            }
            if (continue_attack) {
                if (attack->correct_stratagems == attack->config.stratagem_count) {
                    attack_end(attack, true);
                } else {
                    lv_label_set_text_fmt(attack->stratagem_label, "%d/%d", attack->correct_stratagems + 1,
                                          attack->config.stratagem_count);
                    update_stratagem(attack);
                }
            } else {
                attack_end(attack, true);
            }
        }
    } else {
        attack->failed_stratagems++;
        lv_obj_set_style_text_color(attack->stratagem_arrows[attack->current_index], ARROW_INCORRECT_COLOR, LV_PART_MAIN);
        bool continue_attack = false;
        if (attack->config.stratagem_callback) {
            int64_t cb_time = esp_timer_get_time();
            continue_attack = attack->config.stratagem_callback(attack->config.battle_id, attack->current_index,
                                                                attack->stratagem_length, attack->correct_stratagems,
                                                                attack->failed_stratagems, (cb_time - attack->start_time) / 1000);
            // Adjust the start time to account for the time taken by the callback since it may have taken a while to run
            attack->start_time += esp_timer_get_time() - cb_time;
        }
        if (continue_attack) {
            attack->current_index = 0;
            update_stratagem(attack);
        } else {
            attack_end(attack, false);
        }
    }
}

static void attack_end(attack_t *attack, bool success) {
    int64_t end_time = esp_timer_get_time();
    int64_t duration = end_time - attack->start_time;

    // Destroy the attack object
    lv_obj_delete_async(attack->current_screen);

    ESP_LOGI(TAG, "Battle ended in %lld ms", duration / 1000);

    // Call the attack end callback
    if (attack->config.result_callback) {
        attack->config.result_callback(attack->config.battle_id, success, attack->stratagem_length,
                                       attack->config.stratagem_count, attack->correct_stratagems, duration / 1000);
    }
}

static void attack_cleanup_event_cb(lv_event_t *e) {
    ESP_LOGD(TAG, "Attack cleanup: deleting attack object");
    attack_t *attack = lv_event_get_user_data(e);
    if (attack == NULL) {
        ESP_LOGE(TAG, "Attack cleanup: attack object is somehow NULL");
        return;
    }

    if (minibadge_event_loop_handle != NULL && attack->dpad_enabled) {
        // Disable polling for the D-pad minibadges
        minibadge_dpad_poll(false, MINIBADGE_SLOT_1);
        minibadge_dpad_poll(false, MINIBADGE_SLOT_2);

        // Unregister the D-pad event handler
        esp_event_handler_unregister_with(minibadge_event_loop_handle, MINIBADGE_DPAD_EVENT, MINIBADGE_DPAD_EVENT_PRESS,
                                          on_minibadge_dpad_event);
    }

    free(attack);
}

void attack_update_status(lv_obj_t *attack_obj, int16_t tower_health, int16_t player_health, uint16_t players_in_battle,
                          uint16_t players_disconnected) {
    if (lvgl_lock(pdMS_TO_TICKS(100), __FILE__, __LINE__)) {
        if (attack_obj == NULL || lv_obj_is_valid(attack_obj) == false) {
            ESP_LOGI(TAG, "Invalid attack UI... deleted already? Not updating status");
            lvgl_unlock(__FILE__, __LINE__);
            return;
        }

        attack_t *attack = lv_obj_get_user_data(attack_obj);
        if (attack->config.mode == ATTACK_MODE_TOWER) {
            if (attack == NULL || attack->tower_health == NULL || attack->player_health == NULL) {
                ESP_LOGI(TAG, "Invalid attack object ... not updating status");
            } else {
                attack->config.tower_health  = tower_health;
                attack->config.player_health = player_health;
                lv_bar_set_value(attack->tower_health, tower_health, LV_ANIM_ON);
                lv_bar_set_value(attack->player_health, player_health, LV_ANIM_ON);
            }
        }
        lvgl_unlock(__FILE__, __LINE__);
    }
}

static arrow_t dpad_state_to_arrow(minibadge_dpad_state_t state) {
    switch (state) {
        case MINIBADGE_DPAD_UP: return ARROW_UP;
        case MINIBADGE_DPAD_DOWN: return ARROW_DOWN;
        case MINIBADGE_DPAD_LEFT: return ARROW_LEFT;
        case MINIBADGE_DPAD_RIGHT: return ARROW_RIGHT;
        default: return -1;
    }
}

typedef struct {
    attack_t *attack;
    arrow_t arrow;
} check_input_args_t;

static void check_input_async(check_input_args_t *args) {
    check_input(args->attack, args->arrow);
    free(args);
}

static void on_minibadge_dpad_event(void *arg, esp_event_base_t base, int32_t id, void *event_data) {
    attack_t *attack = (attack_t *)arg;
    if (attack == NULL) {
        ESP_LOGE(TAG, "on_minibadge_dpad_event: attack object is NULL");
        return;
    }
    minibadge_dpad_event_t *event = (minibadge_dpad_event_t *)event_data;
    if (id == MINIBADGE_DPAD_EVENT_PRESS && event->state != MINIBADGE_DPAD_NONE && event->state != MINIBADGE_DPAD_PRESS) {
        check_input_args_t *args = (check_input_args_t *)malloc(sizeof(check_input_args_t));
        args->attack             = attack;
        args->arrow              = dpad_state_to_arrow(event->state);
        lv_async_call(check_input_async, args);
    }
}
