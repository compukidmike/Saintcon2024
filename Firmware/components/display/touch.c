#include "esp_err.h"
#include "esp_log.h"
#include "esp_lcd_touch.h"

#include "badge.h"
#include "config.h"
#include "display.h"
#include "touch.h"
#include "i2c_manager.h"

#ifdef CONFIG_LCD_TOUCH_ENABLED

static const char *TAG = "touch";

// Touch handle
static esp_lcd_touch_handle_t tp = NULL;

// A buffer to track touch data between reads
static touch_data_t data_buffer;

    #if CONFIG_LCD_TOUCH_USE_INTERRUPT
static SemaphoreHandle_t data_avail;
static SemaphoreHandle_t data_mutex;

/**
 * @brief Task to read touch data from the touch controller
 * @param arg LCD peripheral touch handle
 */
void touch_read_task(void *_arg) {
    (void)_arg; // Unused
    TickType_t last_touch_time = 0;
    while (1) {
        if (xSemaphoreTake(data_avail, portMAX_DELAY) == pdTRUE) {
            // Debounce a bit to avoid false positives
            TickType_t now = xTaskGetTickCount();
            if (now - last_touch_time < pdMS_TO_TICKS(CONFIG_LCD_TOUCH_DEBOUNCE_MS)) {
                continue;
            }

            // Read the touch data
            esp_err_t ret = esp_lcd_touch_read_data(tp);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to read touch data: %s", esp_err_to_name(ret));
                continue;
            }
            assert(tp);

            // Get coordinates and state
            touch_data_t data;
            data.is_pressed = esp_lcd_touch_get_coordinates(tp, &data.x, &data.y, NULL, &data.touch_count, 1);
            last_touch_time = now;

            // Update the buffered data
            if (xSemaphoreTake(data_mutex, portMAX_DELAY) == pdTRUE) {
                if (data.x != data_buffer.x || data.y != data_buffer.y || data.is_pressed != data_buffer.is_pressed) {
                    data_buffer = data;
                }
                xSemaphoreGive(data_mutex);
            }
        }
    }
}

/**
 * @brief Touch interrupt callback to set for `esp_lcd_touch_config_t::interrupt_callback`
 * @param tp LCD peripheral touch handle
 */
static void IRAM_ATTR touch_intr_cb(esp_lcd_touch_handle_t _tp) {
    (void)_tp; // Unused
    BaseType_t task_woken = pdFALSE;
    if (data_avail != NULL) {
        xSemaphoreGiveFromISR(data_avail, &task_woken);
        portYIELD_FROM_ISR(task_woken);
    }
}

/**
 * @brief Callback to read buffered touch data and in turn update the LVGL touch data
 *
 * @param indev Pointer to the input device
 * @param data Pointer to the data structure to store the read data
 */
void lvgl_touch_cb(lv_indev_t *_indev, lv_indev_data_t *data) {
    (void)_indev; // Unused
    if (xSemaphoreTake(data_mutex, portMAX_DELAY) == pdTRUE) {
        if (data_buffer.is_pressed && data_buffer.touch_count > 0) {
            data->state   = LV_INDEV_STATE_PRESSED;
            data->point.x = data_buffer.x;
            data->point.y = data_buffer.y;

            // Reset the screen timeout
            screen_reset_timeout();
        } else {
            data->state = LV_INDEV_STATE_RELEASED;
        }
        xSemaphoreGive(data_mutex);
    }
}
    #else
void lvgl_touch_cb(lv_indev_t *indev, lv_indev_data_t *data) {
    touch_data_t td;

    // Read touch controller data
    esp_err_t err = esp_lcd_touch_read_data(tp);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read touch data: %s", esp_err_to_name(err));
        return;
    }
    assert(tp);

    // Get coordinates and state
    td.is_pressed = esp_lcd_touch_get_coordinates(tp, &td.x, &td.y, NULL, &td.touch_count, 1);
    if (!td.is_pressed) { // Clean up noise when the display is not being touched
        td.x = data_buffer.x;
        td.y = data_buffer.y;
    }
    if (td.x != data_buffer.x || td.y != data_buffer.y || td.is_pressed != data_buffer.is_pressed) {
        data_buffer = td;
        ESP_LOGD(TAG, "New touch data: %d, %d, %d", td.x, td.y, td.is_pressed);
    }
    if (td.is_pressed && td.touch_count > 0) {
        data->state   = LV_INDEV_STATE_PRESSED;
        data->point.x = td.x;
        data->point.y = td.y;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}
    #endif

void init_lcd_touch() {
    esp_lcd_panel_io_handle_t io_handle = NULL;
    i2c_manager_bus_t *bus              = i2c_manager_get_bus(I2C_BUS_TOUCH);
    display_orientation_params_t params = get_display_orientation_params();
    assert(bus);

    // Build touch config struct
    esp_lcd_touch_config_t tp_config = {
        .x_max = params.v_res,
        .y_max = params.h_res,
    #ifdef CONFIG_LCD_TOUCH_RST_GPIO
        .rst_gpio_num = CONFIG_LCD_TOUCH_RST_GPIO,
    #else
        .rst_gpio_num = GPIO_NUM_NC,
    #endif
    #ifdef CONFIG_LCD_TOUCH_INT_GPIO
        .int_gpio_num = CONFIG_LCD_TOUCH_INT_GPIO,
    #else
        .int_gpio_num = GPIO_NUM_NC,
    #endif
    #if CONFIG_LCD_TOUCH_USE_INTERRUPT
        .interrupt_callback = touch_intr_cb,
    #endif
        .flags =
            {
                .swap_xy  = params.swap_xy,
                .mirror_x = params.mirror_x,
                .mirror_y = params.mirror_y,
            },
    };

    #if CONFIG_LCD_TOUCH_CONTROLLER_GT911
    // Get the default GT911 config
    esp_lcd_panel_io_i2c_config_t io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();

    // Set the bus speed to 400kHz
    io_config.scl_speed_hz = 400000;

    // Configure the GT911-specific controller driver data
    esp_lcd_touch_io_gt911_config_t gt911_config = {
        .dev_addr = (uint8_t)io_config.dev_addr,
    };
    tp_config.driver_data = &gt911_config;
    #else
    ESP_LOGE(TAG, "No supported touch controller specified... cannot initialize touch interface");
    return;
    #endif // CONFIG_LCD_TOUCH_CONTROLLER_GT911

    // Initialize the I2C device and I/O interface
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(bus->handle, &io_config, &io_handle));

    // Initialize the touch controller driver
    #if CONFIG_LCD_TOUCH_CONTROLLER_GT911
    ESP_LOGI(TAG, "Initializing GT911 touch controller");
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(io_handle, &tp_config, &tp));
    #endif

    #if CONFIG_LCD_TOUCH_USE_INTERRUPT
    // Create semaphores
    data_avail = xSemaphoreCreateBinary();
    assert(data_avail);
    data_mutex = xSemaphoreCreateMutex();
    assert(data_mutex);

    // Create task to read touch data
    xTaskCreate(touch_read_task, "touch_read_task", 4 * 1024, NULL, 10, NULL);
    #endif
}

/**
 * @brief Set touch orientation
 *
 * @param orientation Display orientation
 * @return esp_err_t ESP_OK on success or an error code on failure
 */
esp_err_t set_touch_orientation(display_orientation_t orientation) {
    display_orientation_params_t params = get_params_for_display_orientation(orientation);
    ESP_RETURN_ON_ERROR(esp_lcd_touch_set_swap_xy(tp, params.swap_xy), TAG, "Failed to set swap_xy");
    ESP_RETURN_ON_ERROR(esp_lcd_touch_set_mirror_x(tp, params.mirror_x), TAG, "Failed to set mirror_x");
    ESP_RETURN_ON_ERROR(esp_lcd_touch_set_mirror_y(tp, params.mirror_y), TAG, "Failed to set mirror_y");
    return ESP_OK;
}

/**
 * @brief Read the current touch orientation parameters
 *
 * @return display_orientation_params_t
 */
display_orientation_params_t get_touch_orientation_params() {
    display_orientation_params_t params = {0};
    if (tp == NULL) {
        ESP_LOGE(TAG, "Touch controller not initialized");
        return params;
    }
    esp_lcd_touch_get_swap_xy(tp, &params.swap_xy);
    esp_lcd_touch_get_mirror_x(tp, &params.mirror_x);
    esp_lcd_touch_get_mirror_y(tp, &params.mirror_y);
    return params;
}

#endif // CONFIG_LCD_TOUCH_ENABLED