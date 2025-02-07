#include <cstddef>
#include <cstring>
#include "types.h"

// Map of tower statuses to their names
const tower_status_map_t tower_status_map[] = {
#define X(val, name) {TOWER_STATUS_##name, #name},
    TOWER_STATUS_LIST
#undef X
};

// Function to get the tower_status_t from a string
extern "C" tower_status_t get_tower_status(const char *status_str) {
    for (size_t i = 0; i < sizeof(tower_status_map) / sizeof(tower_status_map[0]); i++) {
        if (strcasecmp(tower_status_map[i].name, status_str) == 0) {
            return tower_status_map[i].status;
        }
    }
    return TOWER_STATUS_UNKNOWN;
}

// Map of IR code types to their names
const ir_code_type_map_t ir_code_type_map[] = {
#define X(val, name) {IR_CODE_TYPE_##name, #name},
    IR_CODE_TYPE_LIST
#undef X
};

// Function to get the ir_code_type_t from a string
extern "C" ir_code_type_t get_ir_code_type(const char *type_str) {
    for (size_t i = 0; i < sizeof(ir_code_type_map) / sizeof(ir_code_type_map[0]); i++) {
        if (strcasecmp(ir_code_type_map[i].name, type_str) == 0) {
            return ir_code_type_map[i].type;
        }
    }
    return IR_CODE_TYPE_UNKNOWN;
}

// Map of minibadge buff types to their names
const minibadge_buff_type_map_t minibadge_buff_type_map[] = {
#define X(val, name) {MINIBADGE_BUFF_TYPE_##name, #name},
    MINIBADGE_BUFF_TYPE_LIST
#undef X
};

// Function to get the minibadge_buff_type_t from a string
extern "C" minibadge_buff_type_t get_minibadge_buff_type(const char *type_str) {
    for (size_t i = 0; i < sizeof(minibadge_buff_type_map) / sizeof(minibadge_buff_type_map[0]); i++) {
        if (strcasecmp(minibadge_buff_type_map[i].name, type_str) == 0) {
            return minibadge_buff_type_map[i].type;
        }
    }
    return MINIBADGE_BUFF_TYPE_NONE;
}

// Map of player statuses to their names
const player_status_map_t player_status_map[] = {
#define X(val, name) {PLAYER_STATUS_##name, #name},
    PLAYER_STATUS_LIST
#undef X
};

// Function to get the player_status_t from a string
extern "C" player_status_t get_player_status(const char *status_str) {
    for (size_t i = 0; i < sizeof(player_status_map) / sizeof(player_status_map[0]); i++) {
        if (strcasecmp(player_status_map[i].name, status_str) == 0) {
            return player_status_map[i].status;
        }
    }
    return PLAYER_STATUS_UNKNOWN;
}