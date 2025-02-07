#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

// Configuration for loading dots animation
typedef struct {
    lv_obj_t *parent;
    lv_obj_t *align_to;
    lv_coord_t x_offset;
    lv_coord_t y_offset;
    lv_color_t dot_color;
    lv_coord_t dot_size;
    uint32_t fade_in_duration;
    uint32_t fade_out_duration;
    uint32_t sequence_delay;
    uint32_t repeat_delay;
} loading_dots_anim_config_t;

/**
 * @brief Create a loading dots animation
 *
 * @param config Configuration struct for the loading dots animation
 * @return lv_obj_t* Pointer to the loading dots animation container
 */
lv_obj_t *loading_dots_anim(const loading_dots_anim_config_t *config);

#ifdef __cplusplus
}
#endif
