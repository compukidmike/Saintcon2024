#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "freertos/FreeRTOS.h"

// Compmonent headers to expose externally
#include "../config.h"

// Backlight control options and levels
#define LCD_BACKLIGHT_PIN CONFIG_LCD_BACKLIGHT_GPIO
#if defined(CONFIG_LCD_BACKLIGHT_CONTROL_PWM)
    #define LCD_BACKLIGHT_ON  128
    #define LCD_BACKLIGHT_OFF 0
// Set the backlight level (0-255).
void set_backlight(uint8_t level);
#elif defined(CONFIG_LCD_BACKLIGHT_CONTROL_SIMPLE)
    #define LCD_BACKLIGHT_ON  1
    #define LCD_BACKLIGHT_OFF !LCD_BACKLIGHT_ON
// Turn the backlight on or off.
void set_backlight(bool on);
#endif

// LVGL mutex lock
bool lvgl_lock(TickType_t timeout_ticks, const char *file, int line);

// LVGL mutex unlock
bool lvgl_unlock(const char *file, int line);

// Initializes the display, backlight,touch interface, and LVGL.
void display_init();

// Return whether or not the display is initialized.
bool display_ready();

// Get the display orientation parameters.
display_orientation_params_t get_params_for_display_orientation(display_orientation_t orientation);

// Get the display orientation.
display_orientation_t get_display_orientation();

// Get the current display orientation parameters.
display_orientation_params_t get_display_orientation_params();

// Set the display orientation.
void set_display_orientation(display_orientation_t orientation);

#ifdef __cplusplus
}
#endif
