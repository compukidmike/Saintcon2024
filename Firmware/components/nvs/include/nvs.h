#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "esp_err.h"
#include "nvs_flash.h"

/**
 * @brief Initialize NVS flash storage
 *
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t nvs_init();

/**
 * @brief Check if NVS flash storage is ready
 *
 * @return true if NVS flash storage is ready, false otherwise
 */
bool nvs_ready();

#ifdef __cplusplus
}
#endif
