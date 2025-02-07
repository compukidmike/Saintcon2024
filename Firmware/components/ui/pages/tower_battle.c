#include <math.h>
#include <memory.h>
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "api.h"
#include "badge.h"
#include "loadanim.h"
#include "theme.h"
#include "tower_battle.h"
#include "ui.h"
#include "components/attack.h"
#include "components/modal.h"

static const char *TAG = "pages/tower_battle";

#define TOWER_REFRESH_INTERVAL 5 * 1000 * 1000 // 5 seconds
#define BATTLE_STATUS_INTERVAL 5 * 1000 * 1000 // 1 second
#define REVIVAL_SECONDS        10              // 10 seconds
#define REVIVAL_TX_UPDATE_MS   500             // Update the revival countdown and send the IR code every 500ms
typedef enum {
    TOWER_BATTLE_PAGE_INIT,    // Initial state
    TOWER_BATTLE_PAGE_LOADING, // Searching for towers on a timer (every 5 seconds)
    TOWER_BATTLE_PAGE_LISTING, // Listing towers that have been seen recently (and still refreshing every 5 seconds)
    TOWER_BATTLE_PAGE_BATTLE,  // In a battle with a tower
    TOWER_BATTLE_PAGE_RESULT,  // Showing the result of a battle
} tower_battle_page_state_t;

typedef enum {
    BATTLE_STATE_NONE,         // Not in a battle currently
    BATTLE_STATE_JOIN_TOWER,   // Joining a tower
    BATTLE_STATE_LEAVE_TOWER,  // Leaving a tower
    BATTLE_STATE_JOIN_BATTLE,  // Joining a battle
    BATTLE_STATE_ATTACKING,    // Attacking
    BATTLE_STATE_POST_ATTACK,  // Post-attack state
    BATTLE_STATE_DISCONNECTED, // Disconnected from the battle
    BATTLE_STATE_SAVIOR,       // In savior mode
    BATTLE_STATE_SELF_SAVE,    // Self-save mode - only if they have the saving throw or reverse shell power-up
    BATTLE_STATE_POST_REVIVAL, // Post-save message
    BATTLE_STATE_RESULT,       // Showing the result of a battle
} battle_state_t;

typedef enum {
    BTN_NONE,
    BTN_JOIN_BATTLE,
    BTN_LEAVE_BATTLE,
    BTN_ATTACK,
} battle_btn_t;

ESP_EVENT_DEFINE_BASE(TOWER_BATTLE_EVENT);
ESP_EVENT_DEFINE_BASE(TOWER_BATTLE_API_EVENT);

typedef struct {
    minibadge_power_up_t buff;
    int use_count;
} battle_buff_t;

typedef struct {
    lv_obj_t *container;
    lv_obj_t *check_towers_label;
    lv_obj_t *loading_dots;
    lv_obj_t *tower_list;
    lv_obj_t *attack_modal;
    esp_event_loop_handle_t battle_async_events; // Event loop to deal with UI transitions after API calls
    esp_timer_handle_t tower_refresh_timer;
    esp_timer_handle_t status_timer;
    tower_state_t *towers[MAX_NEARBY_TOWERS];
    int nearby_count;
    tower_battle_page_state_t state;
    battle_state_t battle_state;
    int post_attack_percentage;
    tower_info_t *selected_tower;
    uint32_t battle_id;
    api_battle_status_t *battle_status; // To track the battle status
    api_badge_data_t *player_status;    // To track the player status
    battle_buff_t battle_buffs[MINIBADGE_SLOT_COUNT];
    ir_code_t revival_code;      // The IR code to send to revive other players
    int revival_timer_ticks;     // Countdown timer for the revival mode
    char revival_msg[128];       // The message to display for the revival mode
    bool executing_saving_throw; // Whether the player is executing a saving throw - we need to have them do a complicated attack
} tower_battle_page_t;

static tower_battle_page_t page = {0};

// Linked list to track open modal objects that need to be cleaned up
typedef struct modal_node_t {
    lv_obj_t *modal;
    struct modal_node_t *next;
} modal_node_t;
typedef struct {
    modal_node_t *head;
    modal_node_t *tail;
} modal_list_t;
static modal_list_t modal_list = {0};

// Function prototypes
static void render_state();
static void render_state_loading(const char *label_text, bool loadanim);
static void tower_item_event_cb(lv_event_t *event);
static void render_state_listing();
static void render_state_battle();
static void tower_battle_page_cleanup(lv_event_t *event);

static void handle_battle_state();
static void set_battle_state(battle_state_t state);
static void render_tower_join(void *data);
static void render_battle_join(void *data);
static void render_battle_post_attack();
static void render_battle_disconnected();
static void render_battle_savior_prompt();
static void render_battle_savior_run(bool code_received);
static void render_battle_self_save();
static void render_battle_post_revival();
static void render_battle_result();
static void status_updated();
static bool sync_player_status();
static bool sync_battle_status();
static bool sync_status();

static void tower_refresh_timer_callback(void *arg);
static void status_timer_callback(void *arg);
static bool stratagem_callback(uint32_t battle_id, uint16_t stratagem_index, uint16_t stratagem_length, uint16_t score,
                               uint16_t failures, int32_t time_elapsed);
static void attack_result_callback(uint32_t battle_id, bool success, uint16_t stratagem_length, uint16_t stratagem_count,
                                   uint16_t score, int32_t attack_duration);
static void attack_exit_callback(uint32_t battle_id, bool completed);

// Event handlers
static void on_battle_event(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void on_battle_api_event(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

// Modal functions
static void add_modal(lv_obj_t *modal);
static void remove_modal(lv_obj_t *modal);
static void cleanup_modals();

void tower_battle_page_create(lv_obj_t *parent) {
    // Create a parent container
    page.container = lv_obj_create(parent);
    lv_obj_set_size(page.container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_radius(page.container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(page.container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(page.container, 0, LV_PART_MAIN);
    lv_obj_remove_flag(page.container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(page.container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(page.container, tower_battle_page_cleanup, LV_EVENT_DELETE, NULL);

    // Create timers
    esp_timer_create_args_t tower_refresh_timer_args = {
        .callback = tower_refresh_timer_callback,
        .arg      = NULL,
    };
    esp_timer_create(&tower_refresh_timer_args, &page.tower_refresh_timer);
    esp_timer_create_args_t status_timer_args = {
        .callback = status_timer_callback,
        .arg      = NULL,
    };
    esp_timer_create(&status_timer_args, &page.status_timer);

    // Create an event loop for handling async events
    esp_event_loop_args_t loop_args = {
        .queue_size      = 10,
        .task_name       = "tower_battle_async_events",
        .task_stack_size = 4096,
        .task_priority   = 5,
    };
    esp_event_loop_create(&loop_args, &page.battle_async_events);

    // Register event handlers
    esp_event_handler_register_with(page.battle_async_events, TOWER_BATTLE_EVENT, ESP_EVENT_ANY_ID, on_battle_event, NULL);
    esp_event_handler_register_with(page.battle_async_events, TOWER_BATTLE_API_EVENT, ESP_EVENT_ANY_ID, on_battle_api_event,
                                    NULL);

    // Render the current state
    render_state();

    // Clear the alert count
    set_status_alert_count(0);
}

static void tower_battle_page_cleanup(lv_event_t *event) {
    if (page.container != NULL) {
        // Delete timers
        if (esp_timer_is_active(page.tower_refresh_timer)) {
            esp_timer_stop(page.tower_refresh_timer);
        }
        esp_timer_delete(page.tower_refresh_timer);
        if (esp_timer_is_active(page.status_timer)) {
            esp_timer_stop(page.status_timer);
        }
        esp_timer_delete(page.status_timer);

        // Stop any ongoing battle
        if (page.battle_state != BATTLE_STATE_NONE && page.battle_id != 0) {
            api_leave_tower();
        }

        // Delete the event loop
        esp_event_loop_delete(page.battle_async_events);

        // Free the battle status and player status
        if (page.battle_status != NULL) {
            api_free_result_data(page.battle_status, API_BATTLE_STATUS);
        }
        if (page.player_status != NULL) {
            api_free_result_data(page.player_status, API_BADGE_DATA);
        }

        // Reset the page object
        memset(&page, 0, sizeof(tower_battle_page_t));
    }
}

static void render_state() {
    tower_battle_page_state_t prev_state = page.state;

    if (page.battle_state != BATTLE_STATE_NONE) {
        page.state = TOWER_BATTLE_PAGE_BATTLE;
    } else {
        // Check if the towers have changed
        page.nearby_count = get_recent_towers(page.towers, TOWER_RECENCY_WINDOW * 1000);
        if (page.nearby_count == 0) {
            page.state = TOWER_BATTLE_PAGE_LOADING;
        } else {
            page.state = TOWER_BATTLE_PAGE_LISTING;
        }
    }

    if (page.state != prev_state) {
        // Clear the page and re-render
        lv_obj_clean(page.container);
        page.check_towers_label = NULL;
        page.loading_dots       = NULL;
        page.tower_list         = NULL;
        lv_refr_now(NULL);
        if (page.state == TOWER_BATTLE_PAGE_LOADING) {
            render_state_loading("Searching for towers...", true);
        } else if (page.state == TOWER_BATTLE_PAGE_LISTING) {
            render_state_listing();
        } else if (page.state == TOWER_BATTLE_PAGE_BATTLE) {
            render_state_battle();
        }
    } else if (page.state == TOWER_BATTLE_PAGE_LISTING) {
        // Update the list of nearby towers while we are in this state
        render_state_listing();
    }

    if (page.state == TOWER_BATTLE_PAGE_BATTLE) {
        // Start the battle status timer
        if (esp_timer_is_active(page.tower_refresh_timer)) {
            esp_timer_stop(page.tower_refresh_timer);
        }
        if (!esp_timer_is_active(page.status_timer)) {
            esp_timer_start_periodic(page.status_timer, BATTLE_STATUS_INTERVAL);
        }
    } else {
        // Start the tower refresh timer
        if (!esp_timer_is_active(page.tower_refresh_timer)) {
            esp_timer_start_periodic(page.tower_refresh_timer, TOWER_REFRESH_INTERVAL);
        }
        if (esp_timer_is_active(page.status_timer)) {
            esp_timer_stop(page.status_timer);
        }
    }
}

static void render_state_loading(const char *label_text, bool loadanim) {
    page.check_towers_label = lv_label_create(page.container);
    lv_label_set_text(page.check_towers_label, label_text);
    lv_obj_set_style_text_color(page.check_towers_label, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_style_text_font(page.check_towers_label, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_align(page.check_towers_label, LV_ALIGN_CENTER, 0, -10);
    if (loadanim) {
        loading_dots_anim_config_t config = {
            .parent            = page.container,
            .align_to          = page.check_towers_label,
            .x_offset          = 0,
            .y_offset          = 20,
            .dot_color         = lv_color_hex(WHITE),
            .dot_size          = 10,
            .fade_in_duration  = 250,
            .fade_out_duration = 250,
            .sequence_delay    = 200,
            .repeat_delay      = 500,
        };
        page.loading_dots = loading_dots_anim(&config);
    }
    lv_refr_now(NULL);
}

static void tower_item_event_cb(lv_event_t *event) {
    tower_info_t *tower_info = (tower_info_t *)lv_event_get_user_data(event);
    ESP_LOGI(TAG, "Tower item clicked: %s", tower_info->name);

    page.selected_tower = tower_info;
    set_battle_state(BATTLE_STATE_JOIN_TOWER);
}

static void render_state_listing() {
    if (page.tower_list != NULL) {
        lv_obj_delete(page.tower_list);
    }

    // Make a list for the nearby towers
    page.tower_list = lv_list_create(page.container);
    lv_obj_set_size(page.tower_list, lv_pct(100), lv_pct(100));
    lv_obj_set_style_radius(page.tower_list, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(page.tower_list, 0, LV_PART_MAIN);
    // lv_obj_set_style_bg_opa(page.tower_list, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_color(page.tower_list, lv_color_hex(CONTENT_BG), LV_PART_MAIN);
    lv_obj_set_style_text_color(page.tower_list, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_flex_grow(page.tower_list, 1);

    // Add the towers to the list
    for (int i = 0; i < page.nearby_count; i++) {
        lv_obj_t *button = lv_list_add_button(page.tower_list, NULL, page.towers[i]->info->name);

        // Style and alignment
        lv_obj_t *label_part = lv_obj_get_child_by_type(button, 0, &lv_label_class);
        lv_obj_set_style_pad_ver(label_part, 7, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(button, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_color(button, lv_color_hex(WHITE), LV_PART_MAIN);
        lv_obj_set_flex_align(button, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

        // Button label
        lv_obj_set_style_text_font(button, &cyberphont3b_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(button, lv_color_hex(WHITE), LV_PART_MAIN);

        // Add an event handler
        lv_obj_add_event_cb(button, tower_item_event_cb, LV_EVENT_CLICKED, page.towers[i]->info);
    }

    lv_refr_now(NULL);
}

static void render_state_battle() {
    // Just have a centered label indicating the battle state
    lv_obj_t *label = lv_label_create(page.container);
    lv_label_set_text(label, page.battle_state == BATTLE_STATE_JOIN_TOWER ? "Connecting to tower..." : "In battle...");
    lv_obj_set_style_text_font(label, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

static void handle_battle_state() {
    switch (page.battle_state) {
        // Not in a battle - do nothing
        case BATTLE_STATE_NONE: //
            break;
        // Joining a battle - show prompt to confirm
        case BATTLE_STATE_JOIN_TOWER: {
            lv_obj_t *modal = loading_modal("Connecting to tower...");
            esp_event_post_to(page.battle_async_events, TOWER_BATTLE_EVENT, BATTLE_EVENT_JOIN_TOWER, &modal, sizeof(lv_obj_t **),
                              0);
            break;
        }
        // Leaving a battle
        case BATTLE_STATE_LEAVE_TOWER: {
            ESP_LOGI(TAG, "Leaving tower");
            lv_obj_t *modal = loading_modal("Leaving...");
            esp_event_post_to(page.battle_async_events, TOWER_BATTLE_EVENT, BATTLE_EVENT_LEAVE_TOWER, &modal, sizeof(lv_obj_t **),
                              0);
            break;
        }
        // Joined a tower - prompt to join battle
        case BATTLE_STATE_JOIN_BATTLE: {
            ESP_LOGI(TAG, "Joining battle");
            lv_obj_t *modal = loading_modal("Joining battle...");
            esp_event_post_to(page.battle_async_events, TOWER_BATTLE_EVENT, BATTLE_EVENT_JOIN_BATTLE, &modal, sizeof(lv_obj_t **),
                              0);
            break;
        }
        // Attacking - show the attack screen
        case BATTLE_STATE_ATTACKING: {
            ESP_LOGI(TAG, "Attacking");
            lv_obj_t *modal = loading_modal("Loading battle...");
            esp_event_post_to(page.battle_async_events, TOWER_BATTLE_EVENT, BATTLE_EVENT_START_ATTACK, &modal,
                              sizeof(lv_obj_t **), 0);
            break;
        }
        // Post-attack state - show attack results
        case BATTLE_STATE_POST_ATTACK: //
            ESP_LOGI(TAG, "Post-attack");
            if (page.post_attack_percentage != 0) { // TODO: Have an intermediate modal to show this to the user
                ESP_LOGI(TAG, "Post-attack percentage: %d", page.post_attack_percentage);
            }

            // If the battle is over, show the result screen
            if (!page.battle_status->battle_active) {
                set_battle_state(BATTLE_STATE_RESULT);
                break;
            }

            // If the player is a savior (due to getting disconnected and having the Hydra power-up), show the savior screen
            if (page.battle_status->is_savior) {
                set_battle_state(BATTLE_STATE_SAVIOR);
                break;
            }

            if (page.battle_status->player_status == PLAYER_STATUS_DISCONNECTED) {
                set_battle_state(BATTLE_STATE_DISCONNECTED);
                break;
            }

            // Otherwise, show the post-attack screen
            render_battle_post_attack();
            break;
        // Disconnected from the battle - show disconnected screen
        case BATTLE_STATE_DISCONNECTED: //
            ESP_LOGI(TAG, "Disconnected from battle");

            // If the player has the saving throw or reverse shell power-up, try to self-save
            bool self_save = false;
            for (int i = 0; i < MINIBADGE_SLOT_COUNT; i++) {
                ESP_LOGD(TAG, "Battle buff %d: [%d] %s - %d uses : %d per battle", i, page.battle_buffs[i].buff,
                         minibadge_buff_type_map[page.battle_buffs[i].buff].name, page.battle_buffs[i].use_count,
                         battle_powerup_info[page.battle_buffs[i].buff].uses_per_battle);
                if (page.battle_buffs[i].buff == POWER_UP_SAVING_THROW || page.battle_buffs[i].buff == POWER_UP_REVERSE_SHELL) {
                    if (battle_powerup_info[page.battle_buffs[i].buff].uses_per_battle == -1 ||
                        page.battle_buffs[i].use_count < battle_powerup_info[page.battle_buffs[i].buff].uses_per_battle) {
                        self_save = true;
                    }
                    break;
                }
            }
            if (self_save) {
                set_battle_state(BATTLE_STATE_SELF_SAVE);
            } else {
                render_battle_disconnected();
            }

            break;
        // In savior mode - show the savior screen if the player is a savior
        case BATTLE_STATE_SAVIOR: //
            ESP_LOGI(TAG, "In savior mode");
            render_battle_savior_prompt();
            break;
        // In self-save mode - show the self-save screen if the player has the saving throw or reverse shell power-up
        case BATTLE_STATE_SELF_SAVE: //
            ESP_LOGI(TAG, "In self-save mode");
            render_battle_self_save();
            break;
        // In revival mode - show the revival screen
        case BATTLE_STATE_POST_REVIVAL: //
            ESP_LOGI(TAG, "In revival mode");
            render_battle_post_revival();
            break;
        // Showing the result of a battle - show the result screen
        case BATTLE_STATE_RESULT: //
            ESP_LOGI(TAG, "Showing battle result");
            render_battle_result();
            break;
    }
}

static void set_battle_state(battle_state_t state) {
    // Set the battle state
    bool state_changed = state != page.battle_state;
    page.battle_state  = state;
    ESP_LOGI(TAG, "Battle state: %d", state);

    // Handle transitioning to/from battle state
    if (state == BATTLE_STATE_NONE && page.state == TOWER_BATTLE_PAGE_BATTLE) {
        render_state();
    } else if (state != BATTLE_STATE_NONE && page.state != TOWER_BATTLE_PAGE_BATTLE) {
        render_state();
    }

    // Cleanup the modals if we are transitioning to a different state
    if (state_changed) {
        cleanup_modals();
    }

    // Handle the battle state
    handle_battle_state();
}

static void on_battle_event(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    switch (event_id) {
        case BATTLE_EVENT_JOIN_TOWER: {
            // Make the API calls and close the loading modal
            api_result_t *result = api_join_tower(page.selected_tower->ir_code);
            if (result != NULL && result->status == true) {
                tower_info_refresh(page.selected_tower->id);
            }

            // Close the loading modal
            lv_obj_t *modal = *(lv_obj_t **)event_data;
            if (modal != NULL && lv_obj_is_valid(modal)) {
                lv_obj_delete_async(modal);
                remove_modal(modal);
            }

            // Post the API response event
            if (result != NULL && result->status == true) {
                esp_event_post_to(page.battle_async_events, TOWER_BATTLE_API_EVENT, API_EVENT_BATTLE_JOIN_TOWER_SUCCESS, NULL, 0,
                                  0);
            } else {
                if (result != NULL && result->detail != NULL) {
                    esp_event_post_to(page.battle_async_events, TOWER_BATTLE_API_EVENT, API_EVENT_BATTLE_JOIN_TOWER_FAILED,
                                      result->detail, strlen(result->detail), 0);
                } else {
                    esp_event_post_to(page.battle_async_events, TOWER_BATTLE_API_EVENT, API_EVENT_BATTLE_JOIN_TOWER_FAILED, NULL,
                                      0, 0);
                }
            }
            api_free_result(result, true);
            break;
        }
        case BATTLE_EVENT_LEAVE_TOWER: {
            // Make the API call and close the loading modal
            api_err_t res   = api_leave_tower();
            lv_obj_t *modal = *(lv_obj_t **)event_data;
            if (modal != NULL && lv_obj_is_valid(modal)) {
                lv_obj_delete_async(modal);
                remove_modal(modal);
            }

            // Post the API response event
            esp_event_post_to(page.battle_async_events, TOWER_BATTLE_API_EVENT, API_EVENT_BATTLE_LEAVE_TOWER_DONE, NULL, 0, 0);
            break;
        }
        case BATTLE_EVENT_JOIN_BATTLE: {
            // Make the API call and close the loading modal
            api_result_t *result = api_join_battle();
            lv_obj_t *modal      = *(lv_obj_t **)event_data;

            // See if we joined the battle successfully
            if (result != NULL && result->status == true) {
                int battle_id = ((api_join_battle_t *)result->data)->battle_id;
                if (battle_id != 0 && battle_id != page.battle_id) {
                    page.battle_id = battle_id;

                    // Set the current buffs (minibadges) available to the player for this battle
                    if (badge_state.minibadges != NULL) {
                        api_minibadge_slot_info_t *slots[MINIBADGE_SLOT_COUNT] = {&badge_state.minibadges[0].slot1,
                                                                                  &badge_state.minibadges[0].slot2};
                        for (int i = 0; i < MINIBADGE_SLOT_COUNT; i++) {
                            if (slots[i]->buff_type != MINIBADGE_BUFF_TYPE_NONE) {
                                page.battle_buffs[i].buff      = slots[i]->buff_type;
                                page.battle_buffs[i].use_count = 0;
                            } else {
                                page.battle_buffs[i].buff      = POWER_UP_NONE;
                                page.battle_buffs[i].use_count = 0;
                            }
                        }
                    }
                }

                // Sync the status and delete the loading modal afterward
                sync_status();
                if (modal != NULL && lv_obj_is_valid(modal)) {
                    lv_obj_delete_async(modal);
                    remove_modal(modal);
                }

                // Make sure the player isn't disconnected
                if (page.battle_status != NULL && page.battle_status->player_status == PLAYER_STATUS_DISCONNECTED) {
                    if (page.battle_status->is_savior) {
                        lv_async_call(set_battle_state, (void *)BATTLE_STATE_SAVIOR);
                    } else {
                        lv_async_call(set_battle_state, (void *)BATTLE_STATE_DISCONNECTED);
                    }
                } else {
                    // Post the API response event
                    esp_event_post_to(page.battle_async_events, TOWER_BATTLE_API_EVENT, API_EVENT_BATTLE_JOIN_BATTLE_SUCCESS,
                                      NULL, 0, 0);
                }
            } else {
                // Delete the loading modal
                if (modal != NULL && lv_obj_is_valid(modal)) {
                    lv_obj_delete_async(modal);
                    remove_modal(modal);
                }

                // Post the API response failure event
                if (result != NULL && result->detail != NULL) {
                    esp_event_post_to(page.battle_async_events, TOWER_BATTLE_API_EVENT, API_EVENT_BATTLE_JOIN_BATTLE_FAILED,
                                      result->detail, strlen(result->detail), 0);
                } else {
                    esp_event_post_to(page.battle_async_events, TOWER_BATTLE_API_EVENT, API_EVENT_BATTLE_JOIN_BATTLE_FAILED, NULL,
                                      0, 0);
                }
            }

            // Free the result
            api_free_result(result, true);
            break;
        }
        case BATTLE_EVENT_START_ATTACK: {
            // Grab the modal pointer so we can delete it when we're ready to start the attack
            lv_obj_t *modal = *(lv_obj_t **)event_data;

            // Fetch the battle status
            sync_status();

            // If the battle ID is 0, we don't have a valid battle to join
            if (page.battle_id == 0 || page.battle_status == NULL) {
                lv_async_call(set_battle_state, (void *)BATTLE_STATE_NONE);
                esp_event_post_to(page.battle_async_events, TOWER_BATTLE_API_EVENT, API_EVENT_BATTLE_START_ATTACK_FAILED,
                                  "Battle ID is 0", 17, 0);
                break;
            }

            // Delete the loading modal
            if (modal != NULL && lv_obj_is_valid(modal)) {
                lv_obj_delete_async(modal);
                remove_modal(modal);
            }

            // Calculate the current tower and player health percentages
            int16_t tower_health =
                (int16_t)roundf((float)page.battle_status->tower_health / page.battle_status->tower_max_health * 100);
            int16_t player_health =
                (int16_t)roundf((float)page.battle_status->player_level / page.battle_status->player_hp * 100);

            // Create the attack modal configuration
            attack_config_t config = {
                .battle_id          = page.battle_id,
                .mode               = ATTACK_MODE_TOWER,
                .stratagem_min      = page.battle_status->strategem_min,
                .stratagem_max      = page.battle_status->strategem_max,
                .stratagem_count    = page.battle_status->stratagem_amount,
                .tower_health       = tower_health,
                .player_health      = player_health,
                .stratagem_callback = stratagem_callback,
                .result_callback    = attack_result_callback,
                .exit_callback      = attack_exit_callback,
            };
            for (int i = 0; i < MINIBADGE_SLOT_COUNT; i++) {
                ESP_LOGI(TAG, "Battle buff [slot %d]: [%d] %s", i + 1, page.battle_buffs[i].buff,
                         minibadge_buff_type_map[page.battle_buffs[i].buff].name);
                if (page.battle_buffs[i].buff != POWER_UP_NONE &&
                    (page.battle_buffs[i].use_count <= battle_powerup_info[page.battle_buffs[i].buff].uses_per_battle ||
                     battle_powerup_info[page.battle_buffs[i].buff].uses_per_battle == -1)) {
                    config.power_ups[i] = page.battle_buffs[i].buff;
                }
            }

            // Override the attack parameters if we are executing a saving throw
            if (page.executing_saving_throw) {
                config.mode            = ATTACK_MODE_SAVING_THROW;
                config.stratagem_count = 8;
                config.stratagem_min   = 8;
                config.stratagem_max   = 8;
            }

            // Create the attack modal
            if (lvgl_lock(pdMS_TO_TICKS(100), __FILE__, __LINE__)) {
                page.attack_modal = attack_create(&config);
                lvgl_unlock(__FILE__, __LINE__);
            } else {
                ESP_LOGE(TAG, "Failed to create attack modal - LVGL lock failed");
            }

            // Post a failure event
            esp_event_post_to(page.battle_async_events, TOWER_BATTLE_API_EVENT, API_EVENT_BATTLE_START_ATTACK_SUCCESS, NULL, 0,
                              0);
            break;
        }
        case BATTLE_EVENT_SAVIOR_START: {
            api_result_t *result = api_get_savior_code();
            lv_obj_t *modal      = *(lv_obj_t **)event_data;

            // Close the loading modal
            if (modal != NULL && lv_obj_is_valid(modal)) {
                lv_obj_delete_async(modal);
                remove_modal(modal);
            }

            if (result != NULL && result->status == true) {
                api_ir_code_only_t *ir_code_data = (api_ir_code_only_t *)result->data;
                page.revival_code                = badge_ir_get_code(ir_code_data->code);

                if (page.revival_code.message_type != IR_MTI_SAVIOR) { // Should never happen
                    ESP_LOGE(TAG, "Received non-savior code: %lu", ir_code_data->code);
                } else {
                    ESP_LOGI(TAG, "Savior code: %lu", ir_code_data->code);
                }

                esp_event_post_to(page.battle_async_events, TOWER_BATTLE_API_EVENT, API_EVENT_BATTLE_SAVIOR_START_SUCCESS, NULL,
                                  0, 0);
            } else {
                esp_event_post_to(page.battle_async_events, TOWER_BATTLE_API_EVENT, API_EVENT_BATTLE_SAVIOR_START_FAILED,
                                  result->detail, strlen(result->detail) + 1, 0);
            }

            // Free the result
            api_free_result(result, true);
            break;
        }
        case BATTLE_EVENT_SELF_SAVE: {
            lv_obj_t *modal = *(lv_obj_t **)event_data;
            if (page.executing_saving_throw) {
                // Post BATTLE_EVENT_START_ATTACK to start the attack to do the saving throw first
                esp_event_post_to(page.battle_async_events, TOWER_BATTLE_EVENT, BATTLE_EVENT_START_ATTACK, &modal,
                                  sizeof(lv_obj_t **), 0);
                break;
            }

            api_result_t *result = api_self_save(page.battle_id);

            if (result != NULL && result->status == true) {
                if (modal != NULL && lv_obj_is_valid(modal)) {
                    esp_event_post_to(page.battle_async_events, TOWER_BATTLE_EVENT, BATTLE_EVENT_JOIN_BATTLE, &modal,
                                      sizeof(lv_obj_t **), 0);
                } else {
                    esp_event_post_to(page.battle_async_events, TOWER_BATTLE_EVENT, BATTLE_EVENT_JOIN_BATTLE, NULL, 0, 0);
                }
            } else {
                // Close the loading modal
                if (modal != NULL && lv_obj_is_valid(modal)) {
                    lv_obj_delete_async(modal);
                    remove_modal(modal);
                }
                lv_async_call(set_battle_state, (void *)BATTLE_STATE_JOIN_BATTLE);
            }

            // Free the result
            api_free_result(result, true);
            break;
        }
        default: break;
    }
}

static void on_battle_api_event(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    switch (event_id) {
        case API_EVENT_BATTLE_JOIN_TOWER_SUCCESS:
            ESP_LOGI(TAG, "Tower joined successfully");
            lv_async_call(render_tower_join, (void *)true);
            break;
        case API_EVENT_BATTLE_JOIN_TOWER_FAILED: //
            if (event_data != NULL) {
                ESP_LOGI(TAG, "Tower join failed: %s", (char *)event_data);
            } else {
                ESP_LOGI(TAG, "Tower join failed");
            }
            lv_async_call(render_tower_join, (void *)false);
            break;
        case API_EVENT_BATTLE_LEAVE_TOWER_DONE:
            ESP_LOGI(TAG, "Tower left successfully");
            lv_async_call(set_battle_state, (void *)BATTLE_STATE_NONE);
            break;
        case API_EVENT_BATTLE_JOIN_BATTLE_SUCCESS:
            ESP_LOGI(TAG, "Battle joined successfully");
            lv_async_call(render_battle_join, (void *)true);
            break;
        case API_EVENT_BATTLE_JOIN_BATTLE_FAILED:
            if (event_data != NULL) {
                ESP_LOGI(TAG, "Battle join failed: %s", (char *)event_data);
            } else {
                ESP_LOGI(TAG, "Battle join failed");
            }
            lv_async_call(render_battle_join, (void *)false);
            break;
        case API_EVENT_BATTLE_SAVIOR_START_SUCCESS:
            ESP_LOGI(TAG, "Savior start success");
            lv_async_call(render_battle_savior_run, (void *)true);
            break;
        case API_EVENT_BATTLE_SAVIOR_START_FAILED:
            ESP_LOGI(TAG, "Savior start failed: %s", (char *)event_data);
            lv_async_call(render_battle_savior_run, (void *)false);
            break;
        default: break;
    }
}

static void battle_btn_event_cb(lv_event_t *event) {
    lv_obj_t *btn           = lv_event_get_target(event);
    battle_btn_t btn_type   = (battle_btn_t)lv_event_get_user_data(event);
    lv_obj_t *btn_container = lv_obj_get_parent(btn);
    lv_obj_t *modal         = lv_obj_get_user_data(btn_container);

    if (modal != NULL && lv_obj_is_valid(modal)) {
        lv_obj_delete(modal);
        remove_modal(modal);
    }

    ESP_LOGI(TAG, "Battle button event: %d", btn_type);

    switch (btn_type) {
        case BTN_JOIN_BATTLE: {
            ESP_LOGI(TAG, "Button pressed: Joining battle");
            set_battle_state(BATTLE_STATE_JOIN_BATTLE);
            break;
        }
        case BTN_LEAVE_BATTLE: {
            ESP_LOGI(TAG, "Button pressed: Leaving battle");
            set_battle_state(BATTLE_STATE_LEAVE_TOWER);
            break;
        }
        case BTN_ATTACK: {
            ESP_LOGI(TAG, "Button pressed: Attacking");
            set_battle_state(BATTLE_STATE_ATTACKING);
            break;
        }
        default: break;
    }
}

static void modal_exit_cb(lv_event_t *event) {
    lv_obj_t *modal = lv_event_get_user_data(event);
    if (lv_obj_is_valid(modal)) {
        lv_obj_delete_async(modal);
        remove_modal(modal);
    }

    set_battle_state(BATTLE_STATE_NONE);
}

static void render_tower_join(void *data) {
    bool success    = (bool)data;
    lv_obj_t *modal = modal_create_base(NULL, !success);
    add_modal(modal);

    // Add a title with the tower name
    lv_obj_t *title = lv_label_create(modal);
    lv_obj_set_width(title, lv_pct(100));
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_text_fmt(title, "Tower: %s", page.selected_tower->name);
    lv_obj_set_style_text_font(title, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_hex(BLACK), LV_PART_MAIN);
    lv_obj_set_flex_grow(title, 0);

    // Add a container for the tower info or error message
    lv_obj_t *info_content = lv_obj_create(modal);
    lv_obj_set_size(info_content, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(info_content, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(info_content, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_radius(info_content, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(info_content, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(info_content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(info_content, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_remove_flag(info_content, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(info_content, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_grow(info_content, 1);

    if (success) {
        // Create a grid container for tower info
        lv_obj_t *grid = lv_obj_create(info_content);
        lv_obj_set_size(grid, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_pad_all(grid, 0, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_radius(grid, 0, LV_PART_MAIN);
        lv_obj_set_style_border_width(grid, 0, LV_PART_MAIN);
        lv_obj_set_layout(grid, LV_LAYOUT_GRID);

        static lv_coord_t column_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
        static lv_coord_t row_dsc[]    = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
        lv_obj_set_grid_dsc_array(grid, column_dsc, row_dsc);

        // Info: level
        lv_obj_t *level_label = lv_label_create(grid);
        lv_label_set_text(level_label, "Level");
        lv_obj_set_style_text_font(level_label, &clarity_16, LV_PART_MAIN);
        lv_obj_set_grid_cell(level_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

        lv_obj_t *level_value = lv_label_create(grid);
        lv_label_set_text_fmt(level_value, "%d", page.selected_tower->level);
        lv_obj_set_style_text_font(level_value, &bm_mini_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(level_value, lv_color_hex(GRAY_SHADE_3), LV_PART_MAIN);
        lv_obj_set_grid_cell(level_value, LV_GRID_ALIGN_END, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

        // Info: health
        lv_obj_t *health_label = lv_label_create(grid);
        lv_label_set_text(health_label, "Health");
        lv_obj_set_style_text_font(health_label, &clarity_16, LV_PART_MAIN);
        lv_obj_set_grid_cell(health_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

        lv_obj_t *health_value = lv_label_create(grid);
        lv_label_set_text_fmt(health_value, "%d/%d", page.selected_tower->health, page.selected_tower->max_health);
        lv_obj_set_style_text_font(health_value, &bm_mini_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(health_value, lv_color_hex(GRAY_SHADE_3), LV_PART_MAIN);
        lv_obj_set_grid_cell(health_value, LV_GRID_ALIGN_END, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

        // Info: status
        lv_obj_t *status_label = lv_label_create(grid);
        lv_label_set_text(status_label, "Status");
        lv_obj_set_style_text_font(status_label, &clarity_16, LV_PART_MAIN);
        lv_obj_set_grid_cell(status_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);

        lv_obj_t *status_value = lv_label_create(grid);
        lv_label_set_text(status_value, tower_status_map[page.selected_tower->status].name);
        lv_obj_set_style_text_font(status_value, &bm_mini_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(status_value, lv_color_hex(GRAY_SHADE_3), LV_PART_MAIN);
        lv_obj_set_grid_cell(status_value, LV_GRID_ALIGN_END, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    } else {
        // Add a label indicating the tower is full
        lv_obj_t *label = lv_label_create(info_content);
        lv_obj_set_size(label, lv_pct(100), LV_SIZE_CONTENT);
        lv_label_set_text(label, "Couldn't join tower. Try again later.");
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_font(label, &bm_mini_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(label, lv_color_hex(GRAY_SHADE_3), LV_PART_MAIN);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_align(label, LV_ALIGN_CENTER, LV_PART_MAIN);
    }

    // Add a container for buttons at the bottom
    lv_obj_t *buttons = lv_obj_create(modal);
    lv_obj_set_size(buttons, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(buttons, 2, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(buttons, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_radius(buttons, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(buttons, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(buttons, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(buttons, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_flex_grow(buttons, 0);
    lv_obj_set_user_data(buttons, modal);

    // Add a button to cancel the join
    lv_obj_t *cancel_btn = lv_button_create(buttons);
    lv_obj_add_style(cancel_btn, &button_style, LV_PART_MAIN);
    lv_obj_add_style(cancel_btn, &button_orange, LV_PART_MAIN);
    lv_obj_add_event_cb(cancel_btn, modal_exit_cb, LV_EVENT_CLICKED, modal);

    lv_obj_t *cancel_label = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_label, success ? "Cancel" : "OK");
    lv_obj_center(cancel_label);

    if (success) {
        // Add a Join button to join the battle
        lv_obj_t *join_btn = lv_button_create(buttons);
        lv_obj_add_style(join_btn, &button_style, LV_PART_MAIN);
        lv_obj_t *join_label = lv_label_create(join_btn);
        lv_label_set_text(join_label, "Join");
        lv_obj_center(join_label);

        if (page.selected_tower->status == TOWER_STATUS_VULNERABLE) {
            lv_obj_add_style(join_btn, &button_red, LV_PART_MAIN);
            lv_obj_add_event_cb(join_btn, battle_btn_event_cb, LV_EVENT_CLICKED, (void *)BTN_JOIN_BATTLE);
        } else {
            lv_obj_add_style(join_btn, &button_orange, LV_PART_MAIN);
            lv_obj_set_state(join_btn, LV_STATE_DISABLED, true);
        }
    } else {
        set_battle_state(BATTLE_STATE_NONE);
    }
}

static void render_battle_join(void *data) {
    bool success    = (bool)data;
    lv_obj_t *modal = modal_create_base(NULL, false);
    add_modal(modal);
    // Add a title with the tower name
    lv_obj_t *title = lv_label_create(modal);
    lv_obj_set_width(title, lv_pct(100));
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_text_fmt(title, "Tower: %s", page.selected_tower->name);
    lv_obj_set_style_text_font(title, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_hex(BLACK), LV_PART_MAIN);
    lv_obj_set_flex_grow(title, 0);

    // Add a container for the tower info or error message
    lv_obj_t *info_content = lv_obj_create(modal);
    lv_obj_set_size(info_content, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(info_content, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(info_content, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_radius(info_content, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(info_content, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(info_content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(info_content, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_remove_flag(info_content, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(info_content, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_grow(info_content, 1);

    // Add a label indicating the player has joined the battle
    lv_obj_t *label = lv_label_create(info_content);
    lv_obj_set_size(label, lv_pct(100), lv_pct(100));
    lv_label_set_text(label, success ? "You have joined the battle!" : "Couldn't join battle. Try again later.");
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(label, &bm_mini_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(GRAY_SHADE_3), LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_align(label, LV_ALIGN_CENTER, LV_PART_MAIN);

    // Add a container for buttons at the bottom
    lv_obj_t *buttons = lv_obj_create(modal);
    lv_obj_set_size(buttons, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(buttons, 2, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(buttons, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_radius(buttons, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(buttons, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(buttons, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(buttons, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_flex_grow(buttons, 0);
    lv_obj_set_user_data(buttons, modal);

    // Add a button to leave the battle
    lv_obj_t *leave_btn = lv_button_create(buttons);
    lv_obj_add_style(leave_btn, &button_style, LV_PART_MAIN);
    lv_obj_add_style(leave_btn, &button_orange, LV_PART_MAIN);
    lv_obj_add_event_cb(leave_btn, battle_btn_event_cb, LV_EVENT_CLICKED, (void *)BTN_LEAVE_BATTLE);

    lv_obj_t *leave_label = lv_label_create(leave_btn);
    lv_label_set_text(leave_label, success ? "Leave" : "OK");
    lv_obj_center(leave_label);

    if (success) {
        // Add a button to attack the tower
        lv_obj_t *attack_btn = lv_button_create(buttons);
        lv_obj_add_style(attack_btn, &button_style, LV_PART_MAIN);
        lv_obj_add_style(attack_btn, &button_red, LV_PART_MAIN);
        lv_obj_add_event_cb(attack_btn, battle_btn_event_cb, LV_EVENT_CLICKED, (void *)BTN_ATTACK);

        lv_obj_t *attack_label = lv_label_create(attack_btn);
        lv_label_set_text(attack_label, "Attack");
        lv_obj_center(attack_label);
    }
}

static void render_battle_post_attack() {
    lv_obj_t *modal = modal_create_base(NULL, false);
    add_modal(modal);
    // Add a title with the tower name
    lv_obj_t *title = lv_label_create(modal);
    lv_obj_set_width(title, lv_pct(100));
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_text_fmt(title, "Tower: %s", page.selected_tower->name);
    lv_obj_set_style_text_font(title, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_hex(BLACK), LV_PART_MAIN);
    lv_obj_set_flex_grow(title, 0);

    // Add a container for the message
    lv_obj_t *info_content = lv_obj_create(modal);
    lv_obj_set_size(info_content, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(info_content, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(info_content, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_radius(info_content, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(info_content, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(info_content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(info_content, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_remove_flag(info_content, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(info_content, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_grow(info_content, 1);

    // Add a label indicating the tower is full
    lv_obj_t *label = lv_label_create(info_content);
    lv_obj_set_size(label, lv_pct(100), LV_SIZE_CONTENT);
    lv_label_set_text_fmt(label, "Attack success!\n\nSpeed/accuracy was %d%% for that attack combo.",
                          page.post_attack_percentage);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(label, &bm_mini_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(GRAY_SHADE_3), LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_align(label, LV_ALIGN_CENTER, LV_PART_MAIN);

    // Add a container for buttons at the bottom
    lv_obj_t *buttons = lv_obj_create(modal);
    lv_obj_set_size(buttons, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(buttons, 2, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(buttons, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_radius(buttons, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(buttons, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(buttons, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(buttons, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_flex_grow(buttons, 0);
    lv_obj_set_user_data(buttons, modal);

    // Add a button to leave the battle
    lv_obj_t *leave_btn = lv_button_create(buttons);
    lv_obj_add_style(leave_btn, &button_style, LV_PART_MAIN);
    lv_obj_add_style(leave_btn, &button_orange, LV_PART_MAIN);
    lv_obj_add_event_cb(leave_btn, battle_btn_event_cb, LV_EVENT_CLICKED, (void *)BTN_LEAVE_BATTLE);

    lv_obj_t *leave_label = lv_label_create(leave_btn);
    lv_label_set_text(leave_label, "Leave");
    lv_obj_center(leave_label);

    // Add a button to attack the tower again
    lv_obj_t *attack_btn = lv_button_create(buttons);
    lv_obj_add_style(attack_btn, &button_style, LV_PART_MAIN);
    lv_obj_add_style(attack_btn, &button_red, LV_PART_MAIN);
    lv_obj_add_event_cb(attack_btn, battle_btn_event_cb, LV_EVENT_CLICKED, (void *)BTN_ATTACK);

    lv_obj_t *attack_label = lv_label_create(attack_btn);
    lv_label_set_text(attack_label, "Attack");
    lv_obj_center(attack_label);
}

// Clean up the bump animation when the modal is deleted
static void bump_cleanup(lv_event_t *event) {
    lv_obj_t *icon = lv_event_get_user_data(event);
    if (icon != NULL && lv_obj_is_valid(icon)) {
        lv_obj_delete(icon);
    }
}

static void render_battle_disconnected() {
    // Clear the revival message if it's been set previously
    page.revival_msg[0] = '\0';

    lv_obj_t *modal = modal_create_base(NULL, false);
    add_modal(modal);
    // Add a title with the tower name
    lv_obj_t *title = lv_label_create(modal);
    lv_obj_set_width(title, lv_pct(100));
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_text_fmt(title, "%s - Disconnected", page.selected_tower->name);
    lv_obj_set_style_text_font(title, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_hex(BLACK), LV_PART_MAIN);
    lv_obj_set_flex_grow(title, 0);

    // Add a container for the rest of the inner content
    lv_obj_t *main_content = lv_obj_create(modal);
    lv_obj_set_size(main_content, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(main_content, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(main_content, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_radius(main_content, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(main_content, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(main_content, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(main_content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_remove_flag(main_content, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(main_content, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_grow(main_content, 1);

    // If we have a list of saviors, show the list so the player can find one of them to get saved and re-join the battle
    if (page.battle_status->savior_handle_count > 0) {
        // Add a container so we can position an icon to the side of the label and list
        lv_obj_t *container = lv_obj_create(main_content);
        lv_obj_set_size(container, lv_pct(100), lv_pct(100));
        lv_obj_set_style_radius(container, 0, LV_PART_MAIN);
        lv_obj_set_style_border_width(container, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(container, 0, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_flex_grow(container, 1);

        lv_obj_t *label = lv_label_create(container);
        lv_label_set_text(label, "Get revived by:");
        lv_obj_set_width(label, lv_pct(100));
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_font(label, &bm_mini_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(label, lv_color_hex(GRAY_SHADE_3), LV_PART_MAIN);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_align(label, LV_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_flex_grow(label, 0);

        // List container for styling the border
        lv_obj_t *list_container = lv_obj_create(container);
        lv_obj_set_size(list_container, lv_pct(100), lv_pct(100));
        lv_obj_set_style_pad_all(list_container, 5, LV_PART_MAIN);
        lv_obj_set_style_radius(list_container, 1, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(list_container, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(list_container, 2, LV_PART_MAIN);
        lv_obj_set_style_border_color(list_container, lv_color_hex(GRAY_TINT_5), LV_PART_MAIN);
        lv_obj_set_flex_grow(list_container, 1);

        // List the saviors
        lv_obj_t *list = lv_list_create(list_container);
        lv_obj_set_size(list, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_border_width(list, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(list, 0, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_text_color(list, lv_color_hex(GRAY_SHADE_3), LV_PART_MAIN);

        // Add each savior to the list
        for (int i = 0; i < page.battle_status->savior_handle_count; i++) {
            lv_obj_t *item = lv_list_add_text(list, page.battle_status->savior_handle[i]);
            lv_obj_set_style_text_align(item, badge_config.wrist == BADGE_WRIST_LEFT ? LV_TEXT_ALIGN_LEFT : LV_TEXT_ALIGN_RIGHT,
                                        LV_PART_MAIN);
            lv_obj_set_style_pad_ver(item, 5, LV_PART_MAIN);
            lv_obj_set_style_border_side(item, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
            lv_obj_set_style_border_width(item, 2, LV_PART_MAIN);
            lv_obj_set_style_border_color(item, lv_color_hex(GRAY_TINT_5), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(item, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_set_style_text_font(item, &bm_mini_16, LV_PART_MAIN);
            lv_obj_set_style_text_color(item, lv_color_hex(GRAY_SHADE_3), LV_PART_MAIN);
        }

        lv_obj_t *modal_screen = lv_obj_get_screen(modal);
        lv_obj_t *icon         = lv_img_create(modal_screen);
        lv_img_set_src(icon, badge_config.wrist == BADGE_WRIST_LEFT ? &left_bump : &right_bump);
        lv_obj_set_size(icon, left_bump.header.w, left_bump.header.h);
        lv_obj_set_align(icon, badge_config.wrist == BADGE_WRIST_LEFT ? LV_ALIGN_RIGHT_MID : LV_ALIGN_LEFT_MID);
        lv_obj_set_pos(icon, 0, 0);

        // Animate the icon to move left and right
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, icon);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
        lv_anim_set_values(&a, 0, -(left_bump.header.w / 2));
        lv_anim_set_duration(&a, 700);
        lv_anim_set_playback_duration(&a, 700);
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        lv_anim_start(&a);

        // Make sure the icon is deleted when the modal is deleted
        lv_obj_add_event_cb(modal, bump_cleanup, LV_EVENT_DELETE, icon);
    }

    // Otherwise, show a screen with current battle status and a button to leave
    else {
        lv_obj_t *label = lv_label_create(main_content);
        lv_label_set_text(label, "Sorry, you were disconnected from the battle.");
        lv_obj_set_size(label, lv_pct(100), lv_pct(100));
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_font(label, &bm_mini_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(label, lv_color_hex(GRAY_SHADE_3), LV_PART_MAIN);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_align(label, LV_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_flex_grow(label, 0);
    }

    // Add a container for buttons at the bottom
    lv_obj_t *buttons = lv_obj_create(modal);
    lv_obj_set_size(buttons, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(buttons, 2, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(buttons, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_radius(buttons, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(buttons, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(buttons, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(buttons, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_flex_grow(buttons, 0);
    lv_obj_set_user_data(buttons, modal);

    // Add a button to leave the battle
    lv_obj_t *leave_btn = lv_button_create(buttons);
    lv_obj_add_style(leave_btn, &button_style, LV_PART_MAIN);
    lv_obj_add_style(leave_btn, &button_orange, LV_PART_MAIN);
    lv_obj_add_event_cb(leave_btn, battle_btn_event_cb, LV_EVENT_CLICKED, (void *)BTN_LEAVE_BATTLE);

    lv_obj_t *leave_label = lv_label_create(leave_btn);
    lv_label_set_text(leave_label, "Leave");
    lv_obj_center(leave_label);
}

static void savior_button_event_cb(lv_event_t *event) {
    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t *prompt     = lv_event_get_user_data(event);
    if (code == LV_EVENT_CLICKED) {
        // Close the current modal
        if (prompt != NULL && lv_obj_is_valid(prompt)) {
            lv_obj_delete_async(prompt);
            remove_modal(prompt);
        }

        // Show the loading screen while we start the revival mode
        lv_obj_t *modal = loading_modal("Loading revival mode...");
        esp_event_post_to(page.battle_async_events, TOWER_BATTLE_EVENT, BATTLE_EVENT_SAVIOR_START, &modal, sizeof(lv_obj_t **),
                          0);
    }
}

static void render_battle_savior_prompt() {
    lv_obj_t *modal = modal_create_base(NULL, false);
    add_modal(modal);

    // Add a title
    lv_obj_t *title = lv_label_create(modal);
    lv_obj_set_width(title, lv_pct(100));
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_text(title, "Revive Mode");
    lv_obj_set_style_text_font(title, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_hex(BLACK), LV_PART_MAIN);
    lv_obj_set_flex_grow(title, 0);

    // Add a container for the grid
    lv_obj_t *grid_container = lv_obj_create(modal);
    lv_obj_set_size(grid_container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(grid_container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(grid_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_radius(grid_container, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(grid_container, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(grid_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(grid_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_remove_flag(grid_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(grid_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_grow(grid_container, 1);

    // Create a flex grid with one row and two columns
    lv_obj_t *grid = lv_obj_create(grid_container);
    lv_obj_set_size(grid, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(grid, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_radius(grid, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(grid, 0, LV_PART_MAIN);
    lv_obj_set_layout(grid, LV_LAYOUT_GRID);

    // Set the column descriptors
    static lv_coord_t column_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[]    = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(grid, column_dsc, row_dsc);

    // Add a description label
    lv_obj_t *description = lv_label_create(grid);
    lv_label_set_text(description, "Revive others for 10 seconds. Press the button to start.");
    lv_label_set_long_mode(description, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(description, &bm_mini_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(description, lv_color_hex(GRAY_SHADE_3), LV_PART_MAIN);
    lv_obj_set_style_text_align(description, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_grid_cell(description, LV_GRID_ALIGN_STRETCH, badge_config.wrist == BADGE_WRIST_LEFT ? 0 : 1, 1,
                         LV_GRID_ALIGN_STRETCH, 0, 1);

    // Add a button to start the revival mode to the other side of the grid
    lv_obj_t *button = lv_button_create(grid);
    lv_obj_add_style(button, &button_style, LV_PART_MAIN);
    lv_obj_add_style(button, &button_grey, LV_PART_MAIN);
    lv_obj_set_style_pad_top(button, -15, LV_PART_MAIN);
    lv_obj_add_event_cb(button, savior_button_event_cb, LV_EVENT_CLICKED, (void *)modal);
    lv_obj_set_grid_cell(button, LV_GRID_ALIGN_CENTER, badge_config.wrist == BADGE_WRIST_LEFT ? 1 : 0, 1, LV_GRID_ALIGN_END, 0,
                         1);

    lv_obj_t *image      = lv_img_create(button);
    lv_image_dsc_t *icon = badge_config.wrist == BADGE_WRIST_LEFT ? &left_bump : &right_bump;
    lv_obj_set_size(image, icon->header.w, icon->header.h);
    lv_img_set_src(image, icon);
    lv_obj_set_style_align(image, LV_ALIGN_CENTER, LV_PART_MAIN);

    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, "Start");
    lv_obj_align_to(label, button, LV_ALIGN_BOTTOM_MID, 0, 15);
}

static void update_revival_time(lv_timer_t *timer) {
    lv_obj_t *label       = lv_timer_get_user_data(timer);
    int seconds_remaining = (page.revival_timer_ticks-- * REVIVAL_TX_UPDATE_MS) / 1000;
    if (seconds_remaining < 0) {
        seconds_remaining = 0;
    }

    if (label != NULL && lv_obj_is_valid(label)) {
        lv_label_set_text_fmt(label, "Revival mode running... \n%d seconds remaining", seconds_remaining);
    }

    if (page.revival_timer_ticks <= 0) {
        lv_timer_delete(timer);

        // They are still disconnected but maybe they can try to join the battle again
        set_battle_state(BATTLE_STATE_JOIN_BATTLE);
    }

    // Send the revival IR code
    ir_disable_rx();
    ir_transmit(page.revival_code.address, page.revival_code.command);
    ir_enable_rx();
}

static void render_battle_savior_run(bool code_received) {
    lv_obj_t *modal = loading_modal("Revival mode running...");
    add_modal(modal);
    lv_obj_t *modal_screen = lv_obj_get_screen(modal);
    lv_obj_t *icon         = lv_img_create(modal_screen);
    lv_img_set_src(icon, badge_config.wrist == BADGE_WRIST_LEFT ? &left_bump : &right_bump);
    lv_obj_set_size(icon, left_bump.header.w, left_bump.header.h);
    lv_obj_set_align(icon, badge_config.wrist == BADGE_WRIST_LEFT ? LV_ALIGN_RIGHT_MID : LV_ALIGN_LEFT_MID);
    lv_obj_set_pos(icon, 0, 0);

    page.revival_timer_ticks = REVIVAL_SECONDS * (1000 / REVIVAL_TX_UPDATE_MS);
    lv_obj_t *label          = lv_obj_get_child_by_type(modal, 0, &lv_label_class);
    lv_timer_t *timer        = lv_timer_create(update_revival_time, REVIVAL_TX_UPDATE_MS, label);
    lv_timer_set_repeat_count(timer, page.revival_timer_ticks);
    lv_timer_set_auto_delete(timer, false); // We will delete it manually in the callback
    lv_timer_ready(timer);

    // Animate the icon to move left and right
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, icon);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_values(&a, 0, -(left_bump.header.w / 2));
    lv_anim_set_duration(&a, 700);
    lv_anim_set_playback_duration(&a, 700);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);

    // Make sure the icon is deleted when the modal is deleted
    lv_obj_add_event_cb(modal, bump_cleanup, LV_EVENT_DELETE, icon);

    lv_refr_now(NULL);
}

static void render_battle_self_save() {
    // Figure out which power-up they used to get self-save
    int power_up_index = -1;
    for (int i = 0; i < MINIBADGE_SLOT_COUNT; i++) {
        if (page.battle_buffs[i].buff == POWER_UP_SAVING_THROW || page.battle_buffs[i].buff == POWER_UP_REVERSE_SHELL) {
            if (battle_powerup_info[page.battle_buffs[i].buff].uses_per_battle == -1 ||
                page.battle_buffs[i].use_count < battle_powerup_info[page.battle_buffs[i].buff].uses_per_battle) {
                power_up_index = i;
                break;
            }
        }
    }
    if (power_up_index == -1) {
        ESP_LOGE(TAG, "No power-up index found for self-save");
        set_battle_state(BATTLE_STATE_JOIN_BATTLE);
        return;
    }

    // Increment the use count for the power-up
    page.battle_buffs[power_up_index].use_count++;

    // Set the flag to indicate that the player is executing a saving throw
    if (page.battle_buffs[power_up_index].buff == POWER_UP_SAVING_THROW) {
        page.executing_saving_throw = true;
    }

    // Show the loading modal with the power-up name
    char power_up_name[64];
    if (page.battle_buffs[power_up_index].buff == POWER_UP_SAVING_THROW) {
        strncpy(power_up_name, "Activating saving throw...\ncomplete this challenge!", sizeof(power_up_name));
    } else {
        strncpy(power_up_name, "Activating\nreverse shell...", sizeof(power_up_name));
    }
    lv_obj_t *modal = loading_modal(power_up_name);
    add_modal(modal);

    // Post the self-save event
    esp_event_post_to(page.battle_async_events, TOWER_BATTLE_EVENT, BATTLE_EVENT_SELF_SAVE, &modal, sizeof(lv_obj_t **), 0);
}

static void post_revival_ok_cb(lv_event_t *event) {
    lv_obj_t *modal = lv_event_get_user_data(event);
    if (modal != NULL && lv_obj_is_valid(modal)) {
        lv_obj_delete_async(modal);
        remove_modal(modal);
    }
    set_battle_state(BATTLE_STATE_JOIN_BATTLE);
}

static void render_battle_post_revival() {
    lv_obj_t *modal = modal_create_base(NULL, false);
    add_modal(modal);

    // Add a title
    lv_obj_t *title = lv_label_create(modal);
    lv_label_set_text(title, "Revived!");
    lv_obj_set_style_text_font(title, &cyberphont3b_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_hex(BLACK), LV_PART_MAIN);
    lv_obj_set_flex_grow(title, 0);

    // Add a container for the main content
    lv_obj_t *main_content = lv_obj_create(modal);
    lv_obj_set_size(main_content, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(main_content, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(main_content, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_flex_flow(main_content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(main_content, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_flex_grow(main_content, 1);

    // Add a label to display the revival message
    lv_obj_t *revival_msg = lv_label_create(main_content);
    lv_label_set_text(revival_msg, page.revival_msg);
    lv_obj_set_size(revival_msg, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_border_width(revival_msg, 0, LV_PART_MAIN);
    lv_label_set_long_mode(revival_msg, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(revival_msg, &bm_mini_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(revival_msg, lv_color_hex(GRAY_SHADE_3), LV_PART_MAIN);
    lv_obj_set_style_text_align(revival_msg, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_align(revival_msg, LV_ALIGN_CENTER, LV_PART_MAIN);

    // Add a container for buttons at the bottom
    lv_obj_t *buttons = lv_obj_create(modal);
    lv_obj_set_size(buttons, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(buttons, 2, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(buttons, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_radius(buttons, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(buttons, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(buttons, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(buttons, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_flex_grow(buttons, 0);
    lv_obj_set_user_data(buttons, modal);

    // Add an OK button to close the modal
    lv_obj_t *ok_btn = lv_button_create(modal);
    lv_obj_add_style(ok_btn, &button_style, LV_PART_MAIN);
    lv_obj_add_style(ok_btn, &button_orange, LV_PART_MAIN);
    lv_obj_add_event_cb(ok_btn, post_revival_ok_cb, LV_EVENT_CLICKED, (void *)modal);

    lv_obj_t *ok_label = lv_label_create(ok_btn);
    lv_label_set_text(ok_label, "OK");
    lv_obj_center(ok_label);
}

static void render_battle_result() {
    ESP_LOGI(TAG, "TODO: Show battle result");
}

static void status_updated() {
    // TODO: Update the UI with the new status info based on what is currently being displayed
}

static bool sync_player_status() {
    bool player_updated = false;

    // Get the current player status from the API
    api_result_t *badge_result = api_get_badge_data();
    if (badge_result != NULL && badge_result->status == true) {
        if (page.player_status != NULL) {
            api_free_result_data(page.player_status, API_BADGE_DATA);
        }
        page.player_status = badge_result->data;
        player_updated     = true;
    }
    api_free_result(badge_result, !player_updated);

    return player_updated;
}

static bool sync_battle_status() {
    // If the battle ID is not set, return false
    if (page.battle_id == 0) {
        return false;
    }

    bool battle_updated = false;

    // Get the current battle status from the API
    api_result_t *battle_result = api_get_battle_status(page.battle_id);
    if (battle_result != NULL && battle_result->status == true) {
        if (page.battle_status != NULL) {
            api_free_result_data(page.battle_status, API_BATTLE_STATUS);
        }
        page.battle_status = battle_result->data;
        battle_updated     = true;
    }
    api_free_result(battle_result, !battle_updated);

    return battle_updated;
}

static bool sync_status() {
    // Call the status updated callback if the battle or player data has updated
    if (sync_battle_status() || sync_player_status()) {
        lv_async_call(status_updated, NULL);
        return true;
    }
    return false;
}

static void tower_refresh_timer_callback(void *arg) {
    lv_async_call(render_state, NULL);
    vTaskDelay(1); // Delay 1 tick to make sure it is actually run async
}

static void status_timer_callback(void *arg) {
    sync_status();

    if (page.battle_state == BATTLE_STATE_ATTACKING && page.attack_modal != NULL) {
        int16_t tower_health =
            (int16_t)roundf((float)page.battle_status->tower_health / page.battle_status->tower_max_health * 100);
        int16_t connection_strength =
            (int16_t)roundf((float)page.battle_status->player_level / page.battle_status->player_hp * 100);
        attack_update_status(page.attack_modal, tower_health, connection_strength, page.battle_status->players_in_battle,
                             page.battle_status->players_disconnected);
    }
}

static bool stratagem_callback(uint32_t battle_id, uint16_t stratagem_index, uint16_t stratagem_length, uint16_t score,
                               uint16_t failures, int32_t time_elapsed) {
    ESP_LOGI(TAG, "[Battle:%lu] Stratagem %d (length: %d) with %d failures in %ldms", battle_id, stratagem_index,
             stratagem_length, failures, time_elapsed);

    // Check the battle status and if the player is disconnected or the battle is over, return false
    if (!page.executing_saving_throw &&
        (page.battle_status->player_status == PLAYER_STATUS_DISCONNECTED || !page.battle_status->battle_active)) {
        return false;
    }

    // If they have the spellchecker power-up, they can have up to 1 failure before it is used up
    int spellcheck_index = -1;
    for (int i = 0; i < MINIBADGE_SLOT_COUNT; i++) {
        if (page.battle_buffs[i].buff == POWER_UP_SPELLCHECK) {
            spellcheck_index = i;
        }
    }
    if (failures > 0) {
        if (spellcheck_index != -1) {
            page.battle_buffs[spellcheck_index].use_count++;
            return true;
        }
    }

    return failures == 0;
}

static void attack_result_callback(uint32_t battle_id, bool success, uint16_t stratagem_length, uint16_t stratagem_count,
                                   uint16_t score, int32_t attack_duration) {
    ESP_LOGI(TAG, "[Battle:%lu] Attack ended with %s in %ldms", battle_id, success ? "success" : "failure", attack_duration);

    // Show a generic loading modal
    lv_obj_t *modal = loading_modal(success ? "Attack successful!" : "Attack failed!");
    add_modal(modal);
    lv_refr_now(NULL);

    // Handle the saving throw
    if (page.executing_saving_throw) {
        page.executing_saving_throw = false;
        if (success) {
            esp_event_post_to(page.battle_async_events, TOWER_BATTLE_EVENT, BATTLE_EVENT_SELF_SAVE, &modal, sizeof(lv_obj_t **),
                              0);
        } else {
            set_battle_state(BATTLE_STATE_JOIN_BATTLE);
        }
        return;
    }

    // Send the attack to the API otherwise
    if (success) {
        api_result_t *result = api_send_attack(battle_id, stratagem_length, stratagem_count, attack_duration);
        if (result != NULL) {
            if (result->status == true && result->data != NULL) {
                page.post_attack_percentage = (((api_send_attack_t *)result->data)->percent);
            }
            api_free_result(result, true);
        }
    } else {
        bool battle_updated  = false;
        api_result_t *result = api_send_attack_fail(battle_id);
        if (result != NULL && result->status == true) {
            // Save the updated battle status that is returned from the API
            if (page.battle_status != NULL) {
                api_free_result_data(page.battle_status, API_BATTLE_STATUS);
            }
            page.battle_status = result->data;
            battle_updated     = true;
        }
        api_free_result(result, !battle_updated);
    }

    set_battle_state(BATTLE_STATE_POST_ATTACK);
}

static void attack_exit_callback(uint32_t battle_id, bool completed) {
    ESP_LOGI(TAG, "[Battle:%lu] Attack UI exit - completed: %s", battle_id, completed ? "true" : "false");
    if (!completed) { // For now we only call this when the player leaves the battle with the back/exit button so it will always
                      // be an incomplete attack
        set_battle_state(BATTLE_STATE_JOIN_BATTLE);
    } else {
        // We shouldn't get here but if we do, set the battle state to none so the user can rejoin a tower
        set_battle_state(BATTLE_STATE_NONE);
    }
}

// Handler for IR revival codes - called by `route_high_priority_code(...)` in `ir.c`
void handle_savior_code(ir_code_t *ir_code, const char *msg) {
    // Ignore duplicate revival codes
    static ir_code_t last_ir_code = {0};
    if (ir_code->decoded == last_ir_code.decoded) {
        return;
    }
    last_ir_code = *ir_code;
    strncpy(page.revival_msg, msg, sizeof(page.revival_msg));

    // If the battle state is disconnected, set the battle state to post-revival
    if (page.battle_state == BATTLE_STATE_DISCONNECTED) {
        ESP_LOGD(TAG, "Revival code received: %08X (%08X) - %s", (unsigned int)ir_code->code, (unsigned int)ir_code->decoded,
                 msg);
        set_battle_state(BATTLE_STATE_POST_REVIVAL);
    } else {
        ESP_LOGD(TAG, "Revival code received but battle state is not disconnected: %08X (%08X)", (unsigned int)ir_code->code,
                 (unsigned int)ir_code->decoded);
    }
}

// Modal functions
static void add_modal(lv_obj_t *modal) {
    modal_node_t *node = (modal_node_t *)malloc(sizeof(modal_node_t));
    if (node != NULL) {
        node->modal = modal;
        node->next  = NULL;
        if (modal_list.head == NULL) {
            modal_list.head = node;
        } else {
            modal_list.tail->next = node;
        }
        modal_list.tail = node;
    }
}

static void remove_modal(lv_obj_t *modal) {
    modal_node_t *current  = modal_list.head;
    modal_node_t *previous = NULL;
    while (current != NULL) {
        if (current->modal == modal) {
            if (previous == NULL) {
                modal_list.head = current->next;
            } else {
                previous->next = current->next;
            }
            if (current == modal_list.tail) {
                modal_list.tail = previous;
            }
            free(current);
            break;
        }
        previous = current;
        current  = current->next;
    }
}

static void cleanup_modals() {
    modal_node_t *current = modal_list.head;
    while (current != NULL) {
        if (current->modal != NULL && lv_obj_is_valid(current->modal)) {
            lv_obj_delete_async(current->modal);
        }
        modal_node_t *next = current->next;
        free(current);
        current = next;
    }
    modal_list.head = NULL;
    modal_list.tail = NULL;
}
