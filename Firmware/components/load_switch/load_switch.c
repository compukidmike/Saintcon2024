#include "load_switch.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "load_switch";

static bool load_switch_en = false;

esp_err_t load_switch_init() {
    ESP_LOGD(TAG, "Initializing load switch");

    gpio_config_t io_conf = {
        .intr_type    = GPIO_INTR_DISABLE,
        .mode         = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << CONFIG_LOAD_SWITCH_EN_GPIO),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en   = GPIO_PULLUP_ENABLE, // Enable pull-up since the load switch EN pin is active low on the 'B' variant
    };
    return gpio_config(&io_conf);
}

esp_err_t load_switch_enable() {
    ESP_LOGI(TAG, "Enabling load switch");
    esp_err_t ret = gpio_set_level(CONFIG_LOAD_SWITCH_EN_GPIO, 0);
    if (ret == ESP_OK) {
        load_switch_en = true;
    }
    return ret;
}

esp_err_t load_switch_disable() {
    ESP_LOGI(TAG, "Disabling load switch");
    esp_err_t ret = gpio_set_level(CONFIG_LOAD_SWITCH_EN_GPIO, 1);
    if (ret == ESP_OK) {
        load_switch_en = false;
    }
    return ret;
}

bool load_switch_enabled() {
    return load_switch_en;
}
