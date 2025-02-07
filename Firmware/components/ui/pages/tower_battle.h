#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "api.h"

// Battle event types
ESP_EVENT_DECLARE_BASE(TOWER_BATTLE_EVENT);
typedef enum {
    BATTLE_EVENT_JOIN_TOWER,
    BATTLE_EVENT_LEAVE_TOWER,
    BATTLE_EVENT_JOIN_BATTLE,
    BATTLE_EVENT_START_ATTACK,
    BATTLE_EVENT_SAVIOR_START,
    BATTLE_EVENT_SELF_SAVE,
    // BATTLE_EVENT_RESULT,
} battle_event_t;
ESP_EVENT_DECLARE_BASE(TOWER_BATTLE_API_EVENT);
typedef enum {
    API_EVENT_BATTLE_JOIN_TOWER_SUCCESS,
    API_EVENT_BATTLE_JOIN_TOWER_FAILED,
    API_EVENT_BATTLE_LEAVE_TOWER_DONE, // Doesn't return anything
    API_EVENT_BATTLE_JOIN_BATTLE_SUCCESS,
    API_EVENT_BATTLE_JOIN_BATTLE_FAILED,
    API_EVENT_BATTLE_START_ATTACK_SUCCESS,
    API_EVENT_BATTLE_START_ATTACK_FAILED,
    API_EVENT_BATTLE_SAVIOR_START_SUCCESS,
    API_EVENT_BATTLE_SAVIOR_START_FAILED,
    API_EVENT_BATTLE_SELF_SAVE_SUCCESS,
    API_EVENT_BATTLE_SELF_SAVE_FAILED,
} battle_api_event_t;

// Create the tower battle page
void tower_battle_page_create(lv_obj_t *parent);

// Handler for IR revival codes - called by `route_high_priority_code(...)` in `ir.c`
void handle_savior_code(ir_code_t *ir_code, const char *msg);

#ifdef __cplusplus
}
#endif
