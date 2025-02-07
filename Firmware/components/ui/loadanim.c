#include "esp_log.h"

#include "loadanim.h"

static const char *TAG = "loading_dots";

typedef struct {
    lv_obj_t *dot1;
    lv_obj_t *dot2;
    lv_obj_t *dot3;
    lv_anim_timeline_t *anim_timeline;
} loading_dots_internal_t;

void loading_delete_event_cb(lv_event_t *event);

/**
 * @brief Custom animation function to set the opacity of an object in and out.
 *
 *   This is needed because the built-in LVGL animation timeline implementation is a bit broken. The built-in
 *   playback feature of lv_anim is not implemented in lv_anim_timeline, so we have to manually set the opacity. We
 *   also need to handle the case where it's reached the end of the animation and the opacity is not set to the
 *   path value but rather the end value directly. **BUGS**
 *
 * @param anim The animation object passed in
 * @param value The value to set the opacity to
 */
static void anim_set_opacity_in_out(lv_anim_t *anim, int32_t value) {
    if (value != anim->start_value && value != anim->end_value) {
        int32_t midpoint = anim->duration / 2;
        if (anim->act_time <= midpoint) {
            lv_obj_set_style_opa(anim->var, (lv_opa_t)lv_map(anim->act_time, 0, midpoint, anim->start_value, anim->end_value),
                                 LV_PART_MAIN);
        } else {
            lv_obj_set_style_opa(anim->var,
                                 (lv_opa_t)lv_map(anim->act_time, midpoint, anim->duration, anim->end_value, anim->start_value),
                                 LV_PART_MAIN);
        }
    } else {
        lv_obj_set_style_opa(anim->var, (lv_opa_t)anim->start_value, LV_PART_MAIN);
    }
}

lv_obj_t *loading_dots_anim(const loading_dots_anim_config_t *config) {
    if (config == NULL || config->parent == NULL) {
        ESP_LOGE(TAG, "Invalid configuration");
        return NULL;
    }

    // Create a container to hold the dots
    lv_obj_t *dots_container = lv_obj_create(config->parent);
    lv_obj_set_size(dots_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_layout(dots_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(dots_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dots_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(dots_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(dots_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(dots_container, 0, LV_PART_MAIN);

    // Allocate memory for internal data
    loading_dots_internal_t *internal = malloc(sizeof(loading_dots_internal_t));
    if (internal == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for internal data");
        lv_obj_delete(dots_container);
        return NULL;
    }
    lv_obj_add_event_cb(dots_container, loading_delete_event_cb, LV_EVENT_DELETE, internal);

    // Create the dots
    lv_obj_t *dot1 = lv_obj_create(dots_container);
    lv_obj_set_size(dot1, config->dot_size, config->dot_size);
    lv_obj_set_style_bg_color(dot1, config->dot_color, LV_PART_MAIN);
    lv_obj_set_style_border_width(dot1, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(dot1, 0, LV_PART_MAIN);

    lv_obj_t *dot2 = lv_obj_create(dots_container);
    lv_obj_set_size(dot2, config->dot_size, config->dot_size);
    lv_obj_set_style_bg_color(dot2, config->dot_color, LV_PART_MAIN);
    lv_obj_set_style_border_width(dot2, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(dot2, 0, LV_PART_MAIN);

    lv_obj_t *dot3 = lv_obj_create(dots_container);
    lv_obj_set_size(dot3, config->dot_size, config->dot_size);
    lv_obj_set_style_bg_color(dot3, config->dot_color, LV_PART_MAIN);
    lv_obj_set_style_border_width(dot3, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(dot3, 0, LV_PART_MAIN);

    // Align the dots within the container
    lv_obj_align(dot1, LV_ALIGN_CENTER, -config->dot_size - (config->dot_size * 2 / 3), 0);
    lv_obj_align(dot2, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(dot3, LV_ALIGN_CENTER, config->dot_size + (config->dot_size * 2 / 3), 0);

    // Position the container relative to align_to object
    if (config->align_to != NULL) {
        lv_obj_align_to(dots_container, config->align_to, LV_ALIGN_OUT_BOTTOM_MID, config->x_offset, config->y_offset);
    } else {
        lv_obj_align(dots_container, LV_ALIGN_CENTER, config->x_offset, config->y_offset);
    }

    // Create an animation timeline
    lv_anim_timeline_t *anim_timeline = lv_anim_timeline_create();

    // Calculate the total duration of each dot animation
    uint32_t total_duration = config->fade_in_duration + config->fade_out_duration;

    // Animations for the dots
    lv_anim_t a1;
    lv_anim_t a2;
    lv_anim_t a3;

    // Dot 1 animation
    lv_anim_init(&a1);
    lv_anim_set_var(&a1, dot1);
    lv_anim_set_values(&a1, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_duration(&a1, total_duration);
    lv_anim_set_custom_exec_cb(&a1, anim_set_opacity_in_out);

    // Dot 2 animation
    lv_anim_init(&a2);
    lv_anim_set_var(&a2, dot2);
    lv_anim_set_values(&a2, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_duration(&a2, total_duration);
    lv_anim_set_custom_exec_cb(&a2, anim_set_opacity_in_out);

    // Dot 3 animation
    lv_anim_init(&a3);
    lv_anim_set_var(&a3, dot3);
    lv_anim_set_values(&a3, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_duration(&a3, total_duration);
    lv_anim_set_custom_exec_cb(&a3, anim_set_opacity_in_out);

    // Add animations to the timeline with appropriate delays
    lv_anim_timeline_add(anim_timeline, 0, &a1);
    lv_anim_timeline_add(anim_timeline, config->sequence_delay, &a2);
    lv_anim_timeline_add(anim_timeline, 2 * config->sequence_delay, &a3);

    // Start the animation timeline
    lv_anim_timeline_set_repeat_count(anim_timeline, LV_ANIM_REPEAT_INFINITE);
    lv_anim_timeline_set_repeat_delay(anim_timeline, config->repeat_delay);
    lv_anim_timeline_start(anim_timeline);

    // Save internal data
    internal->dot1          = dot1;
    internal->dot2          = dot2;
    internal->dot3          = dot3;
    internal->anim_timeline = anim_timeline;

    return dots_container;
}

// Handler for if the dots container is deleted
void loading_delete_event_cb(lv_event_t *event) {
    lv_event_code_t code              = lv_event_get_code(event);
    lv_obj_t *obj                     = lv_event_get_target(event);
    loading_dots_internal_t *internal = lv_event_get_user_data(event);

    if (code == LV_EVENT_DELETE) {
        if (internal != NULL) {
            // Clean up all the animations. They don't get cleaned up automatically because they
            // aren't directly associated with the object due to things like using a custom exec
            // callback.
            if (internal->anim_timeline != NULL) {
                uint32_t dot_count = lv_obj_get_child_count(obj);
                for (int i = 0; i < dot_count; i++) {
                    lv_obj_t *child = lv_obj_get_child(obj, 0);
                    lv_anim_delete(child, NULL);
                }
                lv_anim_timeline_delete(internal->anim_timeline);
                internal->anim_timeline = NULL;
            } else {
                ESP_LOGE(TAG, "Animation timeline is NULL... skipping delete");
            }
            free(internal);
        } else {
            ESP_LOGE(TAG, "Internal data is NULL");
        }
    }
}
