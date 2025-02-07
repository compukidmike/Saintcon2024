#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

// Callback types
typedef void (*input_prompt_cb_t)(lv_event_code_t event, const char *input, void *user_data);

// Component function prototypes
void input_prompt(const char *initial_text, const char *placeholder, bool password_mode, input_prompt_cb_t callback,
                  void *user_data);

#ifdef __cplusplus
}
#endif
