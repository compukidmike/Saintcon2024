#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#ifdef CONFIG_LCD_TOUCH_CONTROLLER_GT911
    #include "esp_lcd_touch_gt911.h"
#endif
#include "lvgl.h"

// Struct to store touch data
typedef struct {
    bool is_pressed;
    uint16_t x;
    uint16_t y;
    uint8_t touch_count;
} touch_data_t;

/**
 * @brief Callback to read buffered touch data and in turn update the LVGL touch data
 *
 * @param indev Pointer to the input device
 * @param data Pointer to the data structure to store the read data
 */
void lvgl_touch_cb(lv_indev_t *indev, lv_indev_data_t *data);

/**
 * @brief Initialize LCD touch interface
 */
void init_lcd_touch();

/**
 * @brief Set touch orientation
 *
 * @param orientation Display orientation
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t set_touch_orientation(display_orientation_t orientation);

/**
 * @brief Read the current touch orientation parameters
 *
 * @return display_orientation_params_t
 */
display_orientation_params_t get_touch_orientation_params();

#ifdef __cplusplus
}
#endif
