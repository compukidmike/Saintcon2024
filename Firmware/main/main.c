#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "accel.h"
#include "badge.h"
#include "display.h"
#include "i2c_manager.h"
#include "nvs.h"
#include "power_manager.h"
#include "ui.h"
#include "wifi_manager.h"

static const char *TAG = "main";

void app_main(void) {
    esp_err_t err;
    int64_t start_time = esp_timer_get_time();

    // Global log level override
    // esp_log_set_level_master(ESP_LOG_DEBUG);

    // Initialize the NVS storage component
    err = nvs_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(err));
        return;
    }

    vTaskDelay(pdMS_TO_TICKS(1000));

    // Initialize the badge configuration
    err = badge_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize badge configuration: %s", esp_err_to_name(err));
        return;
    }

    // Install the ISR service if needed
    err = gpio_install_isr_service(ESP_INTR_FLAG_LOWMED);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Failed to install ISR service: %s", esp_err_to_name(err));
    }

    // Initialize the I2C manager
    err = i2c_manager_init_auto();
    if (err != ESP_OK) {
        printf("Failed to initialize I2C manager: %s", esp_err_to_name(err));
        return;
    }

    // Initialize power management
    ESP_LOGD(TAG, "Initializing power management");
    err = power_manager_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize power management: %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGD(TAG, "Power management initialized");

    // Initialize the display - includes touch init and LVGL init
    display_init();

    // Wait for the display to be ready - maybe this should be a callback instead but display features and the UI won't work
    // without at minimum the lv_display object being created
    ESP_LOGI(TAG, "Waiting for display to be ready...");
    while (!display_ready()) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    ESP_LOGI(TAG, "Display ready, continuing...");

    // Apply saved brightness level (with lower limit make sure it's not off)
    if (badge_config.brightness <= 50) {
        badge_config.brightness = LCD_BACKLIGHT_ON;
        save_badge_config();
    }
    set_backlight(badge_config.brightness);

    // Apply orientation from badge config (with sanity check for corrupted config)
    if ((uint8_t)badge_config.wrist > 1) {
        badge_config.wrist = BADGE_WRIST_LEFT;
        save_badge_config();
    }
    set_display_orientation(badge_config.wrist == BADGE_WRIST_LEFT ? DISPLAY_ORIENTATION_LANDSCAPE_FLIP
                                                                   : DISPLAY_ORIENTATION_LANDSCAPE);

    // Initialize the UI
    ui_init();

    // Initialize the WiFi manager
    err = wifi_manager_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize WiFi manager: %s", esp_err_to_name(err));
        return;
    }
    set_status_wifi_state(WIFI_STATUS_CONNECTING);

    //[TESTING / DEBUGGING] Save a WiFi network
    // wifi_credentials_t creds = {
    //     .ssid     = "MyNetwork",
    //     .password = "password",
    // };
    // err = save_wifi_network(&creds);
    // if (err != ESP_OK) {
    //     ESP_LOGE(TAG, "Failed to save WiFi network: %s", esp_err_to_name(err));
    //     return;
    // }

    // Initialize the accelerometer component
    ESP_ERROR_CHECK(accel_init());

    // Switch to the home screen after it's been a few seconds since boot
    int64_t elapsed_time     = esp_timer_get_time() - start_time;
    const int64_t delay_time = 5000 - (elapsed_time / 1000);
    if (delay_time > 0) {
        vTaskDelay(pdMS_TO_TICKS(delay_time));
    }

    // Switch to the hardware test screen if it hasn't been passed yet
    if (!badge_config.hw_pass) {
        set_screen(SCREEN_HWTEST);
    } else {
        set_screen(SCREEN_MAIN);
    }
}