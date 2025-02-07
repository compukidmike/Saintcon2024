#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "esp_err.h"
#include "driver/rmt_types.h"

// Callback type for received data
typedef void (*ir_rx_callback_t)(uint16_t address, uint16_t command);

/**
 * @brief Initialize the IR communication module
 *
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t ir_init(ir_rx_callback_t rx_callback);

/**
 * @brief Enable IR receiving
 *
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t ir_enable_rx();

/**
 * @brief Disable IR receiving
 *
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t ir_disable_rx();

/**
 * @brief Enable IR transmitting
 *
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t ir_enable_tx();

/**
 * @brief Disable IR transmitting
 *
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t ir_disable_tx();

/**
 * @brief Transmit an IR NEC code
 *
 * @param address The address part of the NEC code
 * @param command The command part of the NEC code
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t ir_transmit(uint16_t address, uint16_t command);

#ifdef __cplusplus
}
#endif
