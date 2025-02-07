#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "sdkconfig.h"
#include "esp_err.h"
#include "esp_wifi_types.h"

typedef struct {
    char ssid[32];
    char password[64];
} wifi_credentials_t;

typedef enum {
    WIFI_STATUS_DISCONNECTED,
    WIFI_STATUS_CONNECTING,
    WIFI_STATUS_CONNECTED,
} wifi_status_t;

typedef void (*wifi_status_callback_t)(wifi_status_t status);

#define WIFI_STATUS_CALLBACK_MAX 5

/**
 * @brief Get a list of the saved WiFi credentials
 *
 * @param[out] creds WiFi credentials
 * @param[out] count Number of saved WiFi networks
 *
 * @return
 *     - ESP_OK: WiFi credentials retrieved successfully
 */
esp_err_t get_saved_wifi_networks(wifi_credentials_t *creds, uint8_t *count);

/**
 * @brief Save or update a WiFi network's credentials
 *
 * @param[in] creds WiFi credentials
 *
 * @return
 *     - ESP_OK: WiFi credentials saved successfully
 */
esp_err_t save_wifi_network(wifi_credentials_t *creds);

/**
 * @brief Delete a saved WiFi network's credentials
 *
 * @param[in] ssid SSID of the network to delete
 *
 * @return
 *     - ESP_OK: WiFi credentials deleted successfully
 */
esp_err_t delete_wifi_network(char *ssid);

/**
 * @brief Get a list of available WiFi networks
 *
 * @param[out] networks Array of available networks
 * @param[out] count Number of available networks
 *
 * @return
 *     - ESP_OK: WiFi networks retrieved successfully
 *     - ESP_FAIL: Failed to retrieve WiFi networks
 */
esp_err_t get_wifi_network_list(wifi_ap_record_t **networks, uint16_t *count);

/**
 * @brief Add an external callback to be called for WiFi status changes
 *
 * The callback will be called with the current WiFi connection status. Note that
 * the callback will be called from the WiFi event task, so it should not block and
 * you may need to use a semaphore to synchronize with other tasks.
 *
 * @param[in] cb Callback function
 */
void add_wifi_status_callback(wifi_status_callback_t cb);

/**
 * @brief Get the current WiFi connection status
 *
 * @return Current WiFi connection status
 */
wifi_status_t get_wifi_status();

/**
 * @brief Initialize the WiFi manager
 *
 * @return
 *     - ESP_OK: WiFi manager initialized successfully
 */
esp_err_t wifi_manager_init();

#ifdef __cplusplus
}
#endif