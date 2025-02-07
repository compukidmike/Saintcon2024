#include <string.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_dma_utils.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "lvgl_private.h"

#include "display.h"
#include "ui.h"
#if CONFIG_LCD_TOUCH_ENABLED
    #include "touch.h"
#endif

static const char *TAG = "display";

#ifdef CONFIG_LCD_I80_COLOR_IN_PSRAM
// #define LCD_PIXEL_CLOCK_HZ (2 * 1000 * 1000)
// #define LCD_PIXEL_CLOCK_HZ (5 * 1000 * 1000)
// #define LCD_PIXEL_CLOCK_HZ (7.5 * 1000 * 1000)
    #define LCD_PIXEL_CLOCK_HZ (10 * 1000 * 1000)
// #define LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)
#else
    #define LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)
#endif

// ST7789 specific options
#ifdef CONFIG_LCD_CONTROLLER_ST7789
    #define LCD_CMD_BITS   8
    #define LCD_PARAM_BITS 8
#endif

// LCD configuration for parallel I/O interface
#ifdef CONFIG_LCD_I80_BUS_WIDTH_8
    #define LCD_BUS_WIDTH 8
#elif CONFIG_LCD_I80_BUS_WIDTH_16
    #define LCD_BUS_WIDTH 16
#endif
#define LCD_PIN_DATA0 CONFIG_LCD_I80_DATA_0_GPIO
#define LCD_PIN_DATA1 CONFIG_LCD_I80_DATA_1_GPIO
#define LCD_PIN_DATA2 CONFIG_LCD_I80_DATA_2_GPIO
#define LCD_PIN_DATA3 CONFIG_LCD_I80_DATA_3_GPIO
#define LCD_PIN_DATA4 CONFIG_LCD_I80_DATA_4_GPIO
#define LCD_PIN_DATA5 CONFIG_LCD_I80_DATA_5_GPIO
#define LCD_PIN_DATA6 CONFIG_LCD_I80_DATA_6_GPIO
#define LCD_PIN_DATA7 CONFIG_LCD_I80_DATA_7_GPIO
#if LCD_BUS_WIDTH > 8
    #define LCD_PIN_DATA8  CONFIG_LCD_I80_DATA_8_GPIO
    #define LCD_PIN_DATA9  CONFIG_LCD_I80_DATA_9_GPIO
    #define LCD_PIN_DATA10 CONFIG_LCD_I80_DATA_10_GPIO
    #define LCD_PIN_DATA11 CONFIG_LCD_I80_DATA_11_GPIO
    #define LCD_PIN_DATA12 CONFIG_LCD_I80_DATA_12_GPIO
    #define LCD_PIN_DATA13 CONFIG_LCD_I80_DATA_13_GPIO
    #define LCD_PIN_DATA14 CONFIG_LCD_I80_DATA_14_GPIO
    #define LCD_PIN_DATA15 CONFIG_LCD_I80_DATA_15_GPIO
#endif
#if CONFIG_LCD_I80_RD_GPIO != -1
    #define LCD_PIN_RD CONFIG_LCD_I80_RD_GPIO
#endif
#define LCD_PIN_PCLK CONFIG_LCD_I80_WR_GPIO
#define LCD_PIN_CS   CONFIG_LCD_I80_CS_GPIO
#define LCD_PIN_DC   CONFIG_LCD_I80_RS_GPIO
#define LCD_PIN_RST  CONFIG_LCD_I80_RESET_GPIO

// Panel handle for the display
static esp_lcd_panel_handle_t panel_handle = NULL;

// Display buffer size. Tune this for the display size, color depth, DMA/PSRAM usage, etc.
#define LV_BUFFER_SIZE (CONFIG_LCD_WIDTH * CONFIG_LCD_HEIGHT * sizeof(lv_color16_t) / 1)

// LVGL configuration
#define LVGL_TICK_PERIOD_MS    2
#define LVGL_TASK_MAX_DELAY_MS 500
#define LVGL_TASK_MIN_DELAY_MS 1
#define LVGL_TASK_STACK_SIZE   (8 * 1024)
#define LVGL_TASK_PRIORITY     6

// LVGL display object
static lv_display_t *display = NULL;

// Display initialization flag
static bool display_initialized = false;

// Determine the default display orientation
static display_orientation_t
#ifdef CONFIG_LCD_ORIENTATION_LANDSCAPE
    current_orientation = DISPLAY_ORIENTATION_LANDSCAPE;
#elif defined(CONFIG_LCD_ORIENTATION_PORTRAIT)
    current_orientation = DISPLAY_ORIENTATION_PORTRAIT;
#elif defined(CONFIG_LCD_ORIENTATION_LANDSCAPE_FLIP)
    current_orientation = DISPLAY_ORIENTATION_LANDSCAPE_FLIP;
#elif defined(CONFIG_LCD_ORIENTATION_PORTRAIT_FLIP)
    current_orientation = DISPLAY_ORIENTATION_PORTRAIT_FLIP;
#endif

// Display orientation parameters for the current orientation
static display_orientation_params_t orientation_params = {0};

// Forward declarations
void lvgl_display_setup();

#ifdef CONFIG_LCD_BACKLIGHT_CONTROL_PWM
static int8_t pin_channels[SOC_GPIO_PIN_COUNT] = {0};
static int ledc_count                          = SOC_LEDC_CHANNEL_NUM;
/**
 * @brief Get a free LEDC channel for a given pin
 *
 * @param pin The GPIO pin number
 * @return int8_t
 */
int8_t get_channel(uint8_t pin) {
    int8_t channel = -1;
    if (pin < SOC_GPIO_PIN_COUNT) {
        if (pin_channels[pin] == 0) {
            if (!ledc_count) {
                ESP_LOGE(TAG, "No more LEDC channels available");
                return -1;
            }
            channel = --ledc_count;
        } else {
            channel = pin_channels[pin] - 1;
        }
    }
    return channel;
}

static uint8_t analog_resolution = 8;
static int analog_frequency      = 1000;
/**
 * @brief Set the backlight level using LEDC PWM
 *
 * @param level The backlight level (0-255)
 */
void set_backlight(uint8_t level) {
    // Get and validate LEDC channel
    int8_t chan = get_channel(LCD_BACKLIGHT_PIN);
    if (chan < 0 || chan >= SOC_LEDC_CHANNEL_NUM) {
        ESP_LOGE(TAG, "Invalid LEDC channel");
        return;
    }
    if (analog_resolution > SOC_LEDC_TIMER_BIT_WIDTH) {
        ESP_LOGE(TAG, "Invalid LEDC resolution");
        return;
    }

    // Configure LEDC timer and channel
    uint8_t channel                = chan % 8;
    uint8_t group                  = chan / 8;
    uint8_t timer                  = (chan / 2) % 4;
    ledc_timer_config_t ledc_timer = {
        .speed_mode      = group,
        .timer_num       = timer,
        .duty_resolution = analog_resolution,
        .freq_hz         = analog_frequency,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    if (ledc_timer_config(&ledc_timer) != ESP_OK) {
        ESP_LOGE(TAG, "LEDC timer config failed");
        return;
    }
    if (ledc_get_freq(group, timer) == 0) {
        ESP_LOGE(TAG, "LEDC frequency config failed");
        return;
    }
    uint32_t duty                      = ledc_get_duty(group, channel);
    uint32_t max_duty                  = (1 << analog_resolution) - 1;
    ledc_channel_config_t ledc_channel = {
        .speed_mode = group,
        .channel    = channel,
        .timer_sel  = timer,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = LCD_BACKLIGHT_PIN,
        .duty       = duty,
    };
    if (ledc_channel_config(&ledc_channel) != ESP_OK) {
        ESP_LOGE(TAG, "LEDC channel config failed");
        return;
    }
    pin_channels[LCD_BACKLIGHT_PIN] = channel + 1;

    // Set LEDC channel duty to write level
    ledc_set_duty(group, channel, level * max_duty / 255);
    ledc_update_duty(group, channel);
}
#elif defined(CONFIG_LCD_BACKLIGHT_CONTROL_SIMPLE)
/**
 * @brief Set the backlight level using simple GPIO high/low output
 *
 * @param level Turn the backlight on or off
 */
void set_backlight(bool level) {
    gpio_set_level(LCD_BACKLIGHT_PIN, level);
}
#endif

/**
 * @brief Pin reset for the backlight pin
 */
static void init_backlight() {
    gpio_reset_pin(LCD_BACKLIGHT_PIN);
    gpio_set_direction(LCD_BACKLIGHT_PIN, GPIO_MODE_OUTPUT);
}

/**
 * @brief Notify LVGL that flush is ready (ie. panel I/O has finished transfering color data)
 *
 * @param panel_io Panel IO handle
 * @param event Panel IO event data
 * @param user_ctx User data passed from the `esp_lcd_panel_io_XXX_config_t` struct
 * @return Whether a high priority task has been woken up by this function
 */
static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *event, void *user_ctx) {
    lv_display_flush_ready((lv_display_t *)user_ctx);
    return false;
}

/**
 * @brief LVGL flush callback
 *
 * @param display LVGL display object
 * @param area Area to flush
 * @param px_map Pixel map
 */
static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
#if (CONFIG_LCD_SWAP_COLOR_LVGL && LCD_BUS_WIDTH == 8)
    // For 8-bit interfaces we need to swap the color bytes. Use LVGL (slower) to do it if we aren't doing it in DMA
    lv_draw_sw_rgb565_swap(px_map, (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1));
#endif
    // Copy the buffer content to the display at the specified area
    esp_lcd_panel_draw_bitmap((esp_lcd_panel_handle_t)lv_display_get_user_data(disp), area->x1, area->y1, area->x2 + 1,
                              area->y2 + 1, px_map);
}

/**
 * @brief LVGL tick task
 */
static void increase_lvgl_tick(void *_arg) {
    (void)_arg;
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

static int32_t lvgl_tick_cb() {
    return esp_timer_get_time() / 1000;
}

static bool log_ignore_file(const char *file) {
    const char *F              = strrchr(__FILE__, '/');
    const char *ignore_files[] = {
        F ? F + 1 : __FILE__,
    };
    for (int i = 0; i < sizeof(ignore_files) / sizeof(ignore_files[0]); i++) {
        size_t flen = strlen(file);
        size_t ilen = strlen(ignore_files[i]);
        if (flen >= ilen && strncmp(file + flen - ilen, ignore_files[i], ilen) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * @brief LVGL mutex lock
 */
bool lvgl_lock(TickType_t timeout_ticks, const char *file, int line) {
    lv_result_t res       = lv_mutex_lock(&LV_GLOBAL_DEFAULT()->lv_general_mutex);
    bool ret              = res == LV_RESULT_OK;
    int core_id           = esp_cpu_get_core_id();
    const char *task_name = pcTaskGetName(NULL);
    if (!ret) {
        ESP_LOGE(TAG, "LVGL: lock failed from %s:%d [%s:%d]", file, line, task_name, core_id);
    } else {
        if (!log_ignore_file(file)) {
            ESP_LOGD(TAG, "LVGL: locked from %s:%d [%s:%d]", file, line, task_name, core_id);
        }
    }
    return ret;
}

/**
 * @brief LVGL mutex unlock
 */
bool lvgl_unlock(const char *file, int line) {
    lv_result_t res       = lv_mutex_unlock(&LV_GLOBAL_DEFAULT()->lv_general_mutex);
    bool ret              = res == LV_RESULT_OK;
    int core_id           = esp_cpu_get_core_id();
    const char *task_name = pcTaskGetName(NULL);
    if (!ret) {
        ESP_LOGE(TAG, "LVGL: unlock failed from %s:%d [%s:%d]", file, line, task_name, core_id);
    } else {
        if (!log_ignore_file(file)) {
            ESP_LOGD(TAG, "LVGL: unlocked from %s:%d [%s:%d]", file, line, task_name, core_id);
        }
    }
    return ret;
}

/**
 * @brief LVGL main task
 */
static void lvgl_task(void *_arg) {
    (void)_arg;

    if (!lv_is_initialized()) {
        lvgl_display_setup();
        esp_task_wdt_config_t wdt_config = {
            .timeout_ms    = 10 * 1000, // Default is 5 but I'm adding more time because everything sucks
            .trigger_panic = false,
        };
        esp_task_wdt_reconfigure(&wdt_config);
    }

    ESP_LOGI(TAG, "Starting LVGL task");
    uint32_t task_delay_ms = LVGL_TASK_MAX_DELAY_MS;
    while (1) {
        task_delay_ms = lv_timer_handler();
        if (task_delay_ms > LVGL_TASK_MAX_DELAY_MS) {
            task_delay_ms = LVGL_TASK_MAX_DELAY_MS;
        } else if (task_delay_ms < LVGL_TASK_MIN_DELAY_MS) {
            task_delay_ms = LVGL_TASK_MIN_DELAY_MS;
        }
        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }
}

/**
 * @brief Initialize parallel I/O interface for LCD
 *
 * @param[out] io_handle Pointer to `esp_lcd_panel_io_handle_t`
 * @param[in] user_ctx User context to store... in this case it will be the display object
 */
static void init_lcd_bus(esp_lcd_panel_io_handle_t *io_handle, void *user_ctx) {
    esp_lcd_i80_bus_handle_t i80_bus    = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        // .clk_src = LCD_CLK_SRC_DEFAULT,
        .clk_src     = LCD_CLK_SRC_PLL240M,
        .dc_gpio_num = LCD_PIN_DC,
        .wr_gpio_num = LCD_PIN_PCLK,
        // clang-format off
        .data_gpio_nums =
            {
                LCD_PIN_DATA0,
                LCD_PIN_DATA1,
                LCD_PIN_DATA2,
                LCD_PIN_DATA3,
                LCD_PIN_DATA4,
                LCD_PIN_DATA5,
                LCD_PIN_DATA6,
                LCD_PIN_DATA7,
                #if LCD_BUS_WIDTH > 8
                LCD_PIN_DATA8,
                LCD_PIN_DATA9,
                LCD_PIN_DATA10,
                LCD_PIN_DATA11,
                LCD_PIN_DATA12,
                LCD_PIN_DATA13,
                LCD_PIN_DATA14,
                LCD_PIN_DATA15,
                #endif
            },
        // clang-format on
        .bus_width          = LCD_BUS_WIDTH,
        .max_transfer_bytes = CONFIG_LCD_WIDTH * CONFIG_LCD_HEIGHT * sizeof(uint16_t),
        .dma_burst_size     = 64,
    };
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num       = LCD_PIN_CS,
        .pclk_hz           = LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 10,
        .dc_levels =
            {
                .dc_idle_level  = 0,
                .dc_cmd_level   = 0,
                .dc_dummy_level = 0,
                .dc_data_level  = 1,
            },
        .flags =
            {
                .swap_color_bytes = CONFIG_LCD_SWAP_COLOR_DMA,
            },
        .on_color_trans_done = notify_lvgl_flush_ready,
        .user_ctx            = user_ctx,
        .lcd_cmd_bits        = LCD_CMD_BITS,
        .lcd_param_bits      = LCD_PARAM_BITS,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, io_handle));
}

/**
 * @brief Get orientation parameters for a given display orientation
 *
 * @param orientation Display orientation
 * @return display_orientation_params_t
 */
display_orientation_params_t get_params_for_display_orientation(display_orientation_t orientation) {
    display_orientation_params_t params;
    switch (orientation) {
        case DISPLAY_ORIENTATION_LANDSCAPE:
            params.h_res    = CONFIG_LCD_HEIGHT;
            params.v_res    = CONFIG_LCD_WIDTH;
            params.swap_xy  = true;
            params.mirror_x = false;
            params.mirror_y = true;
            break;
        case DISPLAY_ORIENTATION_LANDSCAPE_FLIP:
            params.h_res    = CONFIG_LCD_HEIGHT;
            params.v_res    = CONFIG_LCD_WIDTH;
            params.swap_xy  = true;
            params.mirror_x = true;
            params.mirror_y = false;
            break;
        case DISPLAY_ORIENTATION_PORTRAIT:
            params.h_res    = CONFIG_LCD_WIDTH;
            params.v_res    = CONFIG_LCD_HEIGHT;
            params.swap_xy  = false;
            params.mirror_x = false;
            params.mirror_y = false;
            break;
        case DISPLAY_ORIENTATION_PORTRAIT_FLIP:
            params.h_res    = CONFIG_LCD_WIDTH;
            params.v_res    = CONFIG_LCD_HEIGHT;
            params.swap_xy  = false;
            params.mirror_x = true;
            params.mirror_y = true;
            break;
        default: ESP_LOGE(TAG, "Invalid display orientation"); break;
    }
    ESP_LOGD(TAG, "Display orientation parameters: h_res=%d, v_res=%d, swap_xy=%d, mirror_x=%d, mirror_y=%d", params.h_res,
             params.v_res, params.swap_xy, params.mirror_x, params.mirror_y);

    return params;
}

/**
 * @brief Set the display orientation
 *
 * @param orientation Display orientation
 */
void set_display_orientation(display_orientation_t orientation) {
    if (orientation == current_orientation) {
        ESP_LOGW(TAG, "Display orientation is already set to %d", orientation);
        return;
    }

    orientation_params  = get_params_for_display_orientation(orientation);
    current_orientation = orientation;

    if (display_initialized) {
        ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, orientation_params.swap_xy));
        ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, orientation_params.mirror_x, orientation_params.mirror_y));

        // For some reason I have to keep it fixed to landscape even when flipped in order to avoid the touch being inverted
        ESP_ERROR_CHECK(set_touch_orientation(DISPLAY_ORIENTATION_LANDSCAPE));

        // Update LVGL
        lv_display_set_rotation(display, (lv_display_rotation_t)orientation);
    }
}

/**
 * @brief Get the display orientation
 *
 * @return display_orientation_t
 */
display_orientation_t get_display_orientation() {
    return current_orientation;
}

/**
 * @brief Get the display orientation parameters for the current orientation
 *
 * @return display_orientation_params_t
 */
display_orientation_params_t get_display_orientation_params() {
    return orientation_params;
}

/**
 * @brief Initialize LCD panel
 *
 * @param[in] io_handle Pointer to `esp_lcd_panel_io_handle_t`
 * @param[out] panel_handle Pointer to `esp_lcd_panel_handle_t`
 */
static void init_lcd_panel(esp_lcd_panel_io_handle_t *io_handle, esp_lcd_panel_handle_t *panel) {
    esp_lcd_panel_handle_t panel_handle = NULL;
    // clang-format off
    #ifdef CONFIG_LCD_CONTROLLER_ST7789
    ESP_LOGI(TAG, "Initialize ST7789 panel");
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_PIN_RST,
        .rgb_ele_order  = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(*io_handle, &panel_config, &panel_handle));

    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);

    // Set inversion, X/Y order, X/Y mirror, panel gap, etc. based on device specs and selected config
    esp_lcd_panel_invert_color(panel_handle, true);
    esp_lcd_panel_set_gap(panel_handle, 0, 0);
    esp_lcd_panel_swap_xy(panel_handle, orientation_params.swap_xy);
    esp_lcd_panel_mirror(panel_handle, orientation_params.mirror_x, orientation_params.mirror_y);
    #endif
    // clang-format on
    *panel = panel_handle;
}

void display_init() {
    // Initialize and turn off LCD backlight
    init_backlight();
    ESP_LOGI(TAG, "Turn off LCD backlight until display is ready");
    set_backlight(LCD_BACKLIGHT_OFF);

    // Get orientation parameters for the current/default orientation
    orientation_params = get_params_for_display_orientation(current_orientation);

    // Create LVGL task
    //   NOTE: LVGL should be pinned to a core due to known issues with LVGL and multi-core systems - see:
    //     - https://forum.lvgl.io/t/esp32-crashes-when-calling-lv-obj-clean/16864/8
    //     - https://forum.lvgl.io/t/use-differents-cores-with-lv-tasks/4105
    xTaskCreatePinnedToCore(lvgl_task, "LVGL", LVGL_TASK_STACK_SIZE, NULL, LVGL_TASK_PRIORITY, NULL, 1);
}

void lvgl_display_setup() {
    // Do basic LVGL initialization
    ESP_LOGI(TAG, "Initialize LVGL");
    lv_init();

    // Create LVGL display object
    ESP_LOGI(TAG, "Initialize LVGL display object");
    display = lv_display_create(orientation_params.h_res, orientation_params.v_res);

    // Initialize parallel I/O interface for LCD
    ESP_LOGI(TAG, "Initialize LCD panel I/O via Intel 8080 bus");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    init_lcd_bus(&io_handle, display);

    // Initialize LCD panel
    ESP_LOGI(TAG, "Initialize LCD panel");
    init_lcd_panel(&io_handle, &panel_handle);

#if CONFIG_LCD_TOUCH_ENABLED
    // Initialize and configure touch interface
    init_lcd_touch();

    // Set up LVGL indev
    static lv_indev_t *indev = NULL;
    indev                    = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_display(indev, display);
    lv_indev_set_read_cb(indev, lvgl_touch_cb);
    lv_indev_set_mode(indev, LV_INDEV_MODE_TIMER);
#endif // CONFIG_LCD_TOUCH_ENABLED

    // Turn on the display
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    // Turn on the backlight
    ESP_LOGI(TAG, "Turn on LCD backlight");
    set_backlight(LCD_BACKLIGHT_ON);

    // Register the display driver to LVGL
    lv_display_set_user_data(display, panel_handle);

    // Allocate the draw buffers for LVGL (2 for double buffering)
    lv_color_t *buf1 = NULL;
    lv_color_t *buf2 = NULL;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 0)
    #if CONFIG_LCD_I80_COLOR_IN_PSRAM
    int heap_caps = MALLOC_CAP_SPIRAM;
    #else
    int heap_caps = MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL;
    #endif
    esp_dma_mem_info_t dma_mem_info = {
        .extra_heap_caps     = heap_caps,
        .dma_alignment_bytes = 4,
    };
    ESP_ERROR_CHECK(esp_dma_capable_malloc(LV_BUFFER_SIZE, &dma_mem_info, (void **)&buf1, NULL));
    ESP_ERROR_CHECK(esp_dma_capable_malloc(LV_BUFFER_SIZE, &dma_mem_info, (void **)&buf2, NULL));
#else
    uint32_t malloc_flags = 0;
    #if CONFIG_LCD_I80_COLOR_IN_PSRAM
    malloc_flags |= ESP_DMA_MALLOC_FLAG_PSRAM;
    #endif // CONFIG_LCD_I80_COLOR_IN_PSRAM
    ESP_ERROR_CHECK(esp_dma_malloc(LV_BUFFER_SIZE, malloc_flags, (void **)&buf1, NULL));
    ESP_ERROR_CHECK(esp_dma_malloc(LV_BUFFER_SIZE, malloc_flags, (void **)&buf2, NULL));
#endif
    assert(buf1 && buf2);
    ESP_LOGI(TAG, "LVGL draw buffers allocated at %p and %p", buf1, buf2);

    // Initialize LVGL draw buffers
    lv_display_set_flush_cb(display, lvgl_flush_cb);
    lv_display_set_buffers(display,        // Display handle
                           buf1,           // Buffer 1
                           buf2,           // Buffer 2
                           LV_BUFFER_SIZE, // Buffer size in bytes
                           //    LV_DISPLAY_RENDER_MODE_PARTIAL // Render mode: partial allows buffers to be smaller than display
                           //    size
                           LV_DISPLAY_RENDER_MODE_FULL
                           // to save RAM
    );

    // Install LVGL tick timer
    // ESP_LOGI(TAG, "Install LVGL tick timer");
    // const esp_timer_create_args_t lvgl_tick_timer_args = {
    //     .callback = increase_lvgl_tick,
    //     .name     = "lvgl_tick",
    // };
    // esp_timer_handle_t lvgl_tick_timer = NULL;
    // ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    // ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, LVGL_TICK_PERIOD_MS * 1000));

    // Install LVGL tick callback
    ESP_LOGI(TAG, "Install LVGL tick callback");
    lv_tick_set_cb(lvgl_tick_cb);

    // Set display initialized
    display_initialized = true;
}

bool display_ready() {
    return display_initialized;
}
