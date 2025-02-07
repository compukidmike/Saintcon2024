#include <string.h>
#include "esp_log.h"
#include "esp_timer.h"

#include "api.h"
#include "badge.h"
#include "towers.h"
#include "ui.h"

static const char *TAG = "badge/towers";

// Tower state is updated with the latest info from the API
tower_state_t tower_state[TOTAL_TOWERS] = {0};

// Tower info is refreshed from the API and cached here
static tower_info_t tower_info[TOTAL_TOWERS] = {0};

// Timer handle for refreshing tower info from the API
static esp_timer_handle_t tower_info_refresh_timer;

// Initialized state
static bool initialized       = false;
static bool all_towers_loaded = false;

// Callback function for the tower info refresh timer
static void tower_info_refresh_callback(void *arg) {
    tower_info_refresh((int)arg);
}

void tower_tracker_init() {
    if (initialized) {
        return;
    }

    // Initialize the tower state
    for (int i = 0; i < TOTAL_TOWERS; i++) {
        tower_state[i].info      = &tower_info[i];
        tower_state[i].last_seen = 0;
    }

    // Refresh the tower info from the API
    tower_info_refresh(REFRESH_ALL);

    // Set up a timer to refresh the tower info periodically
    esp_timer_create_args_t tower_info_refresh_args = {
        .callback = tower_info_refresh_callback,
        .arg      = (void *)REFRESH_ALL,
    };
    esp_timer_create(&tower_info_refresh_args, &tower_info_refresh_timer);
    esp_timer_start_periodic(tower_info_refresh_timer, REFRESH_INTERVAL * 1000);

    initialized = true;
}

bool tower_tracker_ready() {
    return initialized;
}

// Helper function to copy tower info from API struct to similar internal struct
static void copy_tower_info(api_tower_info_t *src, tower_info_t *dst) {
    // Copy non-string fields
    dst->id                   = src->id;
    dst->level                = src->level;
    dst->health               = src->health;
    dst->max_health           = src->max_health;
    dst->ir_code              = src->ir_code;
    dst->enabled              = src->enabled;
    dst->status               = src->status;
    dst->players_in_range     = src->players_in_range;
    dst->players_in_battle    = src->players_in_battle;
    dst->players_joined_tower = src->players_joined_tower;
    dst->players_disconnected = src->players_disconnected;

    // Copy string fields
    if (src->name) {
        strncpy(dst->name, src->name, sizeof(dst->name));
    } else {
        dst->name[0] = '\0';
    }

    if (src->location) {
        strncpy(dst->location, src->location, sizeof(dst->location));
    } else {
        dst->location[0] = '\0';
    }

    if (src->boot_time) {
        strncpy(dst->boot_time, src->boot_time, sizeof(dst->boot_time));
    } else {
        dst->boot_time[0] = '\0';
    }
}

void tower_info_refresh(int tower_id) {
    // If there are no recent towers, be sure toclear the alert count
    if (get_recent_towers(NULL, TOWER_RECENCY_WINDOW * 1000) == 0) {
        set_status_alert_count(0);
    }

    // Make sure we're connected to wifi
    if (badge_state.wifi_status != WIFI_STATUS_CONNECTED) {
        ESP_LOGW(TAG, "Not connected to wifi, skipping tower info refresh");
        return;
    }

    api_result_t *result = NULL;

    if (tower_id == REFRESH_ALL) {
        result = api_get_all_tower_status();
        if (result != NULL) {
            api_all_tower_info_t *all_tower_info = (api_all_tower_info_t *)result->data;
            for (int i = 0; i < all_tower_info->count; i++) {
                copy_tower_info(&all_tower_info->towers[i], &tower_info[i]);
                all_towers_loaded = true;
            }
        }
    } else {
        result = api_get_tower_status_by_id(tower_id);
        if (result != NULL) {
            int idx = tower_id_to_idx(tower_id);
            if (idx != -1) {
                copy_tower_info((api_tower_info_t *)result->data, &tower_info[idx]);
            }
        }
    }

    if (result != NULL) {
        api_free_result(result, true);
    }
}

int tower_ir_to_id(uint32_t ir_code) {
    ESP_LOGD(TAG, "tower_ir_to_id: %lu", ir_code);
    for (int i = 0; i < TOTAL_TOWERS; i++) {
        if (tower_info[i].ir_code == ir_code) {
            ESP_LOGD(TAG, "tower_ir_to_id: %lu -> %d", ir_code, tower_info[i].id);
            return tower_info[i].id;
        }
    }
    return -1;
}

int tower_id_to_idx(int tower_id) {
    for (int i = 0; i < TOTAL_TOWERS; i++) {
        if (tower_info[i].id == tower_id) {
            return i;
        }
    }
    return -1;
}

void tower_seen(int tower_id) {
    // If the tower info is not loaded, try to refresh it
    if (!all_towers_loaded) {
        tower_info_refresh(REFRESH_ALL);
    }

    // Try to look up the tower index
    int idx = tower_id_to_idx(tower_id);
    if (idx == -1) {
        ESP_LOGE(TAG, "Tower %d not found", tower_id);
        return;
    } else {
        ESP_LOGD(TAG, "Tower %d found at index %d", tower_id, idx);
    }

    tower_state[idx].last_seen = esp_timer_get_time();

    if (idx != -1) {
        int64_t now                = esp_timer_get_time();
        int64_t prev_seen          = tower_state[idx].last_seen;
        tower_state[idx].last_seen = now;

        // Refresh the tower info if it was previously seen more than 10 minutes ago
        if (now - prev_seen > 600 * 1000 * 1000) {
            tower_info_refresh(tower_id);
        }

        // Update the alert count
        if (get_content_page() != PAGE_TOWER_BATTLE) {
            set_status_alert_count(get_recent_towers(NULL, TOWER_RECENCY_WINDOW * 1000));
        }
    }
}

int get_recent_towers(tower_state_t **recent_towers, int64_t time_window) {
    int64_t now    = esp_timer_get_time();
    int64_t cutoff = now - time_window;
    if (cutoff < 0) {
        cutoff = 0;
    }
    ESP_LOGI(TAG, "Getting recent towers with cutoff %lld (now %lld)", cutoff, now);
    int count = 0;
    for (int i = 0; i < TOTAL_TOWERS; i++) {
        ESP_LOGI(TAG, "Tower %d: %s seen at %lld", tower_state[i].info->id, tower_state[i].info->name, tower_state[i].last_seen);
        if (tower_state[i].last_seen > cutoff) {
            if (recent_towers != NULL) {
                recent_towers[count++] = &tower_state[i];
            } else {
                count++;
            }
        }
    }
    return count;
}
