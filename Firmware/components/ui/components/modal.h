#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

#define MODAL_BUTTON_COUNT_MAX 3

// Some basic button types we can use
typedef enum {
    MODAL_BUTTON_CANCEL,
    MODAL_BUTTON_OK,
    MODAL_BUTTON_OTHER,
} modal_button_type_t;

typedef struct {
    modal_button_type_t type;
    lv_obj_t *button;
    lv_obj_t *label;
    void *data;
    void (*callback)(void *data);
} modal_button_t;

typedef struct {
    lv_obj_t *modal;
    lv_obj_t *title;
    lv_obj_t *message;
    modal_button_t buttons[MODAL_BUTTON_COUNT_MAX];
} modal_t;

// Create a blank modal styled with our colors, and an optional close button
lv_obj_t *modal_create_base(lv_obj_t *screen, bool closeable);

// Base modal exit callback - can be reused for your own close buttons on a base modal
void base_modal_exit_cb(lv_event_t *event);

// Create a loading modal
lv_obj_t *loading_modal(const char *label_text);

// Create a modal styled with our colors, fonts, etc.
// lv_obj_t modal_create(modal_t *modal, const char *title, const char *message, modal_button_t *buttons, uint8_t button_count);

#ifdef __cplusplus
}
#endif