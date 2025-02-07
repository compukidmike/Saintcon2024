#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"
#include "i2c_manager.h"

#define LOAD_SWITCH_INTERRUPT I2C_SWITCH_INT3

/**
 * @brief Initialize the AP22816B load switch module
 *
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t load_switch_init();

/**
 * @brief Enable the load switch power output
 *
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t load_switch_enable();

/**
 * @brief Disable the load switch power output
 *
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t load_switch_disable();

/**
 * @brief Get the load switch enabled state
 *
 * @return bool true if the load switch is enabled, false if disabled
 */
bool load_switch_enabled();

#ifdef __cplusplus
}
#endif
