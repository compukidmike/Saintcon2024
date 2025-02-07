#include "nvs.h"
#include "esp_log.h"

static const char *TAG = "nvs";

static bool nvs_initialized = false;

esp_err_t nvs_init() {
    if (nvs_initialized) {
        return ESP_OK;
    }
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGI(TAG, "Erasing NVS flash");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    if (err == ESP_OK) {
        nvs_initialized = true;
    }
    return err;
}

bool nvs_ready() {
    return nvs_initialized;
}
