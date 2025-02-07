#include "esp_err.h"
#include "esp_log.h"

#include "config.h"
#include "migrate.h"
#include "nvs.h"

static const char *TAG = "badge/config";

#define BADGE_NVS_NAMESPACE "badge"

badge_config_t badge_config;

esp_err_t load_badge_config() {
    if (!nvs_ready()) {
        ESP_LOGE(TAG, "NVS not ready");
        return ESP_FAIL;
    }

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(BADGE_NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "No NVS data found, using defaults");
        badge_config = BADGE_DEFAULTS;
        return ESP_OK;
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle", esp_err_to_name(err));
        return err;
    }

    uint32_t stored_version = 0;
    err                     = nvs_get_u32(nvs_handle, "badge_version", &stored_version);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "No version found or version size mismatch, initializing defaults");
        badge_config = BADGE_DEFAULTS;
    } else {
        if (stored_version != BADGE_CONFIG_VERSION) {
            ESP_LOGI(TAG, "Configuration version mismatch, migrating if possible");
            migrate_badge_config(nvs_handle, stored_version);
        } else {
            size_t required_size = sizeof(badge_config_t);
            err                  = nvs_get_blob(nvs_handle, "badge_config", &badge_config, &required_size);
            if (err != ESP_OK) {
                if (required_size != sizeof(badge_config_t)) {
                    ESP_LOGW(TAG, "Size mismatch loading badge config, using defaults");
                    badge_config = BADGE_DEFAULTS;
                    save_badge_config();
                } else {
                    ESP_LOGW(TAG, "Error (%s) reading badge config, using defaults for now", esp_err_to_name(err));
                    badge_config = BADGE_DEFAULTS;
                }
            }
        }
    }

    nvs_close(nvs_handle);
    return err;
}

esp_err_t save_badge_config() {
    if (!nvs_ready()) {
        ESP_LOGE(TAG, "NVS not ready");
        return ESP_FAIL;
    }

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(BADGE_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle", esp_err_to_name(err));
        return err;
    }

    err = nvs_set_u32(nvs_handle, "badge_version", badge_config.version);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) saving badge version", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    err = nvs_set_blob(nvs_handle, "badge_config", &badge_config, sizeof(badge_config_t));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) saving badge config", esp_err_to_name(err));
    }

    nvs_close(nvs_handle);
    return err;
}