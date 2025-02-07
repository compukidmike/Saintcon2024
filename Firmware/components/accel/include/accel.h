#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "esp_err.h"

// Compmonent headers to expose externally
#include "../mc3419.h"

/**
 * @brief Initialize the accelerometer.
 *
 * @return esp_err_t
 */
esp_err_t accel_init();

/**
 * @brief Reset the accelerometer (POR).
 *
 * @return esp_err_t
 */
esp_err_t accel_reset();

/**
 * @brief Get the accelerometer Device ID.
 *
 * @param id Pointer to an mc3419_chip_id_t struct instance to store the Device ID.
 * @return esp_err_t
 */
esp_err_t accel_get_device_id(mc3419_chip_id_t *id);

/**
 * @brief Set the accelerometer mode state.
 *
 * @param state The mode state to set.
 * @return esp_err_t
 */
esp_err_t accel_set_mode(mc3419_mode_state_t state);

/**
 * @brief Get the accelerometer mode state.
 *
 * @param mode Pointer to store the mode state.
 * @return esp_err_t
 */
esp_err_t accel_get_mode(mc3419_mode_t *mode);

/**
 * @brief Set interrupt enable state
 *
 * @param intr_en A bitsmask of interrupts to enable from mc3419_interrupt_t
 * @param autoclear Whether to autoclear the interrupts
 */
esp_err_t accel_set_interrupt_config(uint8_t intr_en, bool autoclear);

/**
 * @brief Set the accelerometer interrupt configuration.
 *
 * @param intr_ctrl The interrupt configuration to set.
 * @return esp_err_t
 */
esp_err_t accel_get_interrupt_config(mc3419_intr_ctrl_t *intr_ctrl);

/**
 * @brief Set the accelerometer sample rate.
 *
 * @param rate The sample rate to set.
 * @return esp_err_t
 */
esp_err_t accel_set_sample_rate(mc3419_rate_t rate);

/**
 * @brief Get the accelerometer sample rate.
 *
 * @param rate Pointer to store the sample rate.
 * @return esp_err_t
 */
esp_err_t accel_get_sample_rate(mc3419_sr_t *sr);

/**
 * @brief Set feature enable state
 *
 * @param feature_en A bitmask of features to enable from mc3419_feature_t
 * @return esp_err_t
 */
esp_err_t accel_enable_features(uint8_t feature_en);

/**
 * @brief Set the accelerometer range.
 *
 * @param range The range to set.
 * @return esp_err_t
 */
esp_err_t accel_set_range(mc3419_range_res_t res);

/**
 * @brief Get the accelerometer range.
 *
 * @param range Pointer to store the range.
 * @return esp_err_t
 */
esp_err_t accel_get_range(mc3419_range_t *range);

/**
 * @brief Read accelerometer data.
 *
 * @param data Pointer to store the accelerometer data.
 * @return esp_err_t
 */
esp_err_t accel_read_data(mc3419_accel_data_t *data);

/**
 * @brief Get the accelerometer status.
 *
 * #param status Pointer to store the accelerometer status.
 * @return esp_err_t
 */
esp_err_t accel_get_status(mc3419_status_t *status);

/**
 * @brief Get the accelerometer interrupt pending status.
 *
 * @param status Pointer to store the interrupt status.
 * @return esp_err_t
 */
esp_err_t accel_get_interrupt_status(mc3419_intr_stat_t *status);

/**
 * @brief Reset the interrupt status.
 *
 * @return esp_err_t
 */
esp_err_t accel_reset_interrupt_status();

/**
 * @brief Set GPIO polarity.
 *
 * @param int1_active_high Whether the INT1 pin is active high
 * @param int2_active_high Whether the INT2 pin is active high
 * @return esp_err_t
 */
esp_err_t accel_set_gpio_polarity(bool int1_active_high, bool int2_active_high);

/**
 * @brief Set GPIO drive mode.
 *
 * @param int1_push_pull Whether the INT1 pin is push-pull
 * @param int2_push_pull Whether the INT2 pin is push-pull
 * @return esp_err_t
 */
esp_err_t accel_set_gpio_drive_mode(bool int1_push_pull, bool int2_push_pull);

#ifdef __cplusplus
}
#endif
