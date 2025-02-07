#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

lv_obj_t *create_splash_screen();

// Stop and remove splash screen animations
void splash_screen_shutdown();

#ifdef __cplusplus
}
#endif
