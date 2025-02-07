#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "badge.h"

lv_obj_t *create_update_screen();

// Update progress
void update_ota_state(ota_state_t state);

#ifdef __cplusplus
}
#endif
