#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "api.h"

#define TOTAL_TOWERS               11           // Total number of towers - currently we have 11 total towers
#define MAX_NEARBY_TOWERS          TOTAL_TOWERS // For now, we can just keep track of all towers if needed
#define REFRESH_INTERVAL           120 * 1000   // Refresh interval in milliseconds
#define REFRESH_ALL                -1           // Refresh all towers when calling tower_tracker_refresh()
#define SEEN_AUTO_REFRESH_THROTTLE 60 * 1000    // Throttle how often we refresh tower info after a tower is seen
#define TOWER_RECENCY_WINDOW       300 * 1000   // Check for towers seen within the last 5 minutes

// Very similar to api_tower_info_t, but with fixed size strings for simplicity
typedef struct {
    int id;
    char name[32];
    char location[32];
    int level;
    int health;
    int max_health;
    char boot_time[32];
    uint32_t ir_code;
    bool enabled;
    tower_status_t status;
    int players_in_range;
    int players_in_battle;
    int players_joined_tower;
    int players_disconnected;
} tower_info_t;

// Struct to keep track of tower and last seen time
typedef struct {
    tower_info_t *info;
    int64_t last_seen;
} tower_state_t;

// Current state of all towers - refreshed from API periodically or when IR code is received
extern tower_state_t tower_state[TOTAL_TOWERS];

/**
 * @brief Initialize the tower tracker
 */
void tower_tracker_init();

/**
 * @brief Check if the tower tracker has been initialized
 *
 * @return True if initialized, false otherwise
 */
bool tower_tracker_ready();

/**
 * @brief Refresh the state of a specific tower or all towers
 *
 * @param tower_id ID of the tower to refresh or REFRESH_ALL to refresh all towers
 */
void tower_info_refresh(int tower_id);

/**
 * @brief Convert an IR code to a tower ID
 *
 * @param ir_code IR code to convert
 * @return Tower ID or -1 if not found
 */
int tower_ir_to_id(uint32_t ir_code);

/**
 * @brief Get the index of a tower from its ID
 *
 * @param tower_id ID of the tower
 * @return Index of the tower or -1 if not found
 */
int tower_id_to_idx(int tower_id);

/**
 * @brief Mark a tower as seen
 *
 * @param tower_id ID of the tower to mark as seen
 */
void tower_seen(int tower_id);

/**
 * @brief Get the IDs of towers that have been seen within the given time window
 *
 * @param recent_towers Pointer to an array to store the tower IDs
 * @param time_window Time window in microseconds
 * @return Number of towers found
 */
int get_recent_towers(tower_state_t **recent_towers, int64_t time_window);

#ifdef __cplusplus
}
#endif
