#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"

#include "nvs.h"
#include "wifi_manager.h"

// Compmonent headers to expose externally
#include "../config.h"
#include "../ir.h"
#include "../ota.h"
#include "../towers.h"

typedef struct {
    bool ready;
    wifi_status_t wifi_status;
    api_equip_minibadge_t *minibadges;
} badge_state_t;
extern badge_state_t badge_state;

/**
 * @brief Initialize the badge configuration
 *
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t badge_init();

/**
 * @brief Set the badge handle
 *
 * @param handle The handle to set
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t set_badge_handle(const char *handle);

/**
 * @brief Set the badge wrist
 *
 * @param wrist The wrist to set
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t set_badge_wrist(badge_wrist_t wrist);

/**
 * @brief Set screen timeout
 *
 * @param timeout The timeout in seconds
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t set_screen_timeout(uint32_t timeout);

/**
 * @brief Screen keepalive
 *     Wake the screen if it's off and reset the screen timeout
 */
void screen_reset_timeout();

#ifdef __cplusplus
}
#endif
