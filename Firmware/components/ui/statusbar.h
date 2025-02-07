#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "ui.h"

void create_status_bar(lv_obj_t *parent);
void status_bar_shutdown(); // Clean up timers

/**
 * @brief Update the status bar with the current state
 *       This function should be called within the LVGL context
 *
 * @param state The current UI state
 */
void render_status(ui_state_t *state);

#ifdef __cplusplus
}
#endif