#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

// Simplified battery level enum (used for display)
typedef enum {
    BATTERY_LEVEL_EMPTY, // <= 5%
    BATTERY_LEVEL_1,     // 5% - 25%
    BATTERY_LEVEL_2,     // 25% - 50%
    BATTERY_LEVEL_3,     // 50% - 75%
    BATTERY_LEVEL_FULL,  // 75% - 100%
} battery_level_t;

// Battery status struct with voltage, percentage, and level
typedef struct {
    int voltage;
    uint8_t percentage;
    battery_level_t level;
} battery_status_t;

// Event group bits
#define BATTERY_UPDATE BIT0

// Battery event group
extern EventGroupHandle_t battery_event_group;

/**
 * @brief Initialize the battery module
 *
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t battery_init();

/**
 * @brief Deinitialize the battery module
 *
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t battery_deinit();

/**
 * @brief Get the current battery status
 *
 * @return battery_status_t The current battery status
 */
battery_status_t battery_get_status();

#ifdef __cplusplus
}
#endif
