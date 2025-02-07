#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "wifi_manager.h"
#include "nvs.h"
#include "esp_check.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif_sntp.h"
#include "esp_sntp.h"
#include "esp_task_wdt.h"
#include "esp_wifi.h"

static const char *TAG = "wifi_manager";

#define STRINGIFY(x) #x
#define STR(x)       STRINGIFY(x)

// Setting a hard cap on the number of stored networks for use in some of the logic around storing them in NVS
#define WIFI_STORED_NETWORKS_MAX 10

#ifndef CONFIG_ALLOW_EXTERNAL_WIFI_NETWORKS
    #define CONFIG_EXTERNAL_WIFI_MAX_NETWORKS 0
#else
    #if CONFIG_EXTERNAL_WIFI_MAX_NETWORKS > WIFI_STORED_NETWORKS_MAX
        #pragma message("Warning: CONFIG_EXTERNAL_WIFI_MAX_NETWORKS must be no more than " STR( \
            WIFI_STORED_NETWORKS_MAX) "... setting to " STR(WIFI_STORED_NETWORKS_MAX))
        #define CONFIG_EXTERNAL_WIFI_MAX_NETWORKS WIFI_STORED_NETWORKS_MAX
    #elif CONFIG_EXTERNAL_WIFI_MAX_NETWORKS < 1
        #pragma message("Warning: CONFIG_EXTERNAL_WIFI_MAX_NETWORKS must be at least 1... setting to 1")
        #define CONFIG_EXTERNAL_WIFI_MAX_NETWORKS 1
    #endif
#endif

wifi_status_t wifi_status = WIFI_STATUS_DISCONNECTED;

#ifdef CONFIG_ALLOW_EXTERNAL_WIFI_NETWORKS
static wifi_credentials_t wifi_credentials[CONFIG_EXTERNAL_WIFI_MAX_NETWORKS] = {0};
#endif
static wifi_status_callback_t wifi_status_callbacks[WIFI_STATUS_CALLBACK_MAX] = {0};

// Wifi event group
static EventGroupHandle_t wifi_event_group = NULL;
const int WIFI_CONNECTED_BIT               = BIT0;
const int WIFI_DISCONNECT_BIT              = BIT1;
const int WIFI_NOCONNECT_BIT               = BIT2;

/**
 * @brief Serialize WiFi credentials to NVS
 *
 * @return esp_err_t
 *     - ESP_OK: Credentials serialized successfully. Other errors passed through from NVS
 */
esp_err_t serialize_creds() {
    if (!nvs_ready()) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret;
    nvs_handle_t nvs;
    size_t count = 0;

    ESP_RETURN_ON_ERROR(nvs_open("wifi", NVS_READWRITE, &nvs), TAG, "Failed to open NVS");

    // First try to clear any existing saved credentials
    for (size_t i = 0; i < WIFI_STORED_NETWORKS_MAX; i++) {
        char key[16];
        snprintf(key, sizeof(key), "cred%d", i);
        if ((ret = nvs_erase_key(nvs, key)) == ESP_ERR_NVS_NOT_FOUND) {
            break; // No more credentials to delete
        } else if (ret != ESP_OK) {
            nvs_close(nvs);
            return ret;
        }
    }

#ifdef CONFIG_ALLOW_EXTERNAL_WIFI_NETWORKS
    // Now save the new credentials
    for (size_t i = 0; i < CONFIG_EXTERNAL_WIFI_MAX_NETWORKS; i++) {
        if (strlen(wifi_credentials[i].ssid) == 0) {
            continue;
        }
        char key[16];
        snprintf(key, sizeof(key), "cred%d", i);
        if ((ret = nvs_set_blob(nvs, key, &wifi_credentials[i], sizeof(wifi_credentials_t))) != ESP_OK) {
            nvs_close(nvs);
            return ret;
        }
        if ((ret = nvs_commit(nvs)) != ESP_OK) {
            nvs_close(nvs);
            return ret;
        }
        ++count;
    }
#endif

    if ((ret = nvs_set_u8(nvs, "count", (uint8_t)count)) != ESP_OK) {
        nvs_close(nvs);
        return ret;
    }
    if ((ret = nvs_commit(nvs)) != ESP_OK) {
        nvs_close(nvs);
        return ret;
    }
    nvs_close(nvs);

    return ESP_OK;
}

/**
 * @brief Deserialize WiFi credentials from NVS
 *
 * @return esp_err_t
 *     - ESP_OK: Credentials deserialized successfully. Other errors passed through from NVS
 */
esp_err_t deserialize_creds() {
    if (!nvs_ready()) {
        return ESP_ERR_INVALID_STATE;
    }

#ifdef CONFIG_ALLOW_EXTERNAL_WIFI_NETWORKS
    esp_err_t ret;
    nvs_handle_t nvs;
    uint8_t count = 0;

    ESP_RETURN_ON_ERROR(nvs_open("wifi", NVS_READONLY, &nvs), TAG, "Failed to open NVS");
    if ((ret = nvs_get_u8(nvs, "count", &count)) != ESP_OK) {
        nvs_close(nvs);
        return ret;
    }
    for (uint8_t i = 0; i < count; i++) {
        char key[16];
        snprintf(key, sizeof(key), "cred%d", i);
        size_t len = sizeof(wifi_credentials_t);
        if ((ret = nvs_get_blob(nvs, key, &wifi_credentials[i], &len)) != ESP_OK) {
            nvs_close(nvs);
            return ret;
        }
    }
    nvs_close(nvs);
#endif

    return ESP_OK;
}

esp_err_t get_saved_wifi_networks(wifi_credentials_t *creds, uint8_t *count) {
    if (!nvs_ready()) {
        return ESP_ERR_INVALID_STATE;
    }
    wifi_credentials_t _creds[CONFIG_EXTERNAL_WIFI_MAX_NETWORKS];
    uint8_t _count = 0;
#ifdef CONFIG_ALLOW_EXTERNAL_WIFI_NETWORKS
    for (uint8_t i = 0; i < CONFIG_EXTERNAL_WIFI_MAX_NETWORKS; i++) {
        if (strlen(wifi_credentials[i].ssid) > 0) {
            memcpy(&_creds[_count], &wifi_credentials[i], sizeof(wifi_credentials_t));
            ++_count;
        }
    }
#endif
    memcpy(creds, _creds, sizeof(wifi_credentials_t) * _count);
    *count = _count;
    return ESP_OK;
}

esp_err_t save_wifi_network(wifi_credentials_t *creds) {
    if (!nvs_ready()) {
        return ESP_ERR_INVALID_STATE;
    }

#ifdef CONFIG_ALLOW_EXTERNAL_WIFI_NETWORKS
    // Update the network if it already exists, otherwise add it
    for (size_t i = 0; i < CONFIG_EXTERNAL_WIFI_MAX_NETWORKS; i++) {
        if (strcmp(wifi_credentials[i].ssid, creds->ssid) == 0 || strlen(wifi_credentials[i].ssid) == 0) {
            memcpy(&wifi_credentials[i], creds, sizeof(wifi_credentials_t));
            serialize_creds();
            return ESP_OK;
        }
    }
#endif

    ESP_LOGE(TAG, "No slots remaining to save WiFi network");
    return ESP_ERR_NO_MEM;
}

esp_err_t delete_wifi_network(char *ssid) {
    if (!nvs_ready()) {
        return ESP_ERR_INVALID_STATE;
    }

#ifdef CONFIG_ALLOW_EXTERNAL_WIFI_NETWORKS
    for (size_t i = 0; i < CONFIG_EXTERNAL_WIFI_MAX_NETWORKS; i++) {
        if (strcmp(wifi_credentials[i].ssid, ssid) == 0) {
            memset(&wifi_credentials[i], 0, sizeof(wifi_credentials_t));
            serialize_creds();
            return ESP_OK;
        }
    }
#endif

    ESP_LOGE(TAG, "No saved network with SSID %s", ssid);
    return ESP_ERR_NOT_FOUND;
}

/**
 * @brief Initialize the WiFi station interface
 *
 * @return esp_err_t
 *     - ESP_OK: WiFi station interface initialized and started successfully
 *     - Other errors passed through from WiFi functions
 */
esp_err_t init_wifi_sta() {
    const esp_netif_t *const netif = esp_netif_create_default_wifi_sta();
    assert(netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_RETURN_ON_ERROR(esp_wifi_init(&cfg), TAG, "Failed to initialize WiFi");
    ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_STA), TAG, "Failed to set WiFi mode");
    ESP_RETURN_ON_ERROR(esp_wifi_start(), TAG, "Failed to start WiFi");

    return ESP_OK;
}

void time_sync_notification_cb(struct timeval *tv) {
    ESP_LOGI(TAG, "SNTP time sync complete");

    // Print the current time
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    ESP_LOGI(TAG, "Current date/time: %s", asctime(&timeinfo));
}

/**
 * @brief Initialize the SNTP service and set the timezone
 *
 * @return esp_err_t
 */
esp_err_t init_sntp() {
    // Set the timezone to Mountain Time
    setenv("TZ", "MST7MDT,M3.2.0,M11.1.0", 1);
    tzset();

    // Initialize SNTP with multiple servers
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG_MULTIPLE(2, ESP_SNTP_SERVER_LIST("time.aws.com", "pool.ntp.org"));
    config.sync_cb           = time_sync_notification_cb;
    return esp_netif_sntp_init(&config);
}

/**
 * @brief Attempt to connect to the given WiFi network
 *
 * @param ssid SSID of the network to connect to
 * @param password Password of the network to connect to
 *
 * @return esp_err_t
 *     - ESP_OK: Successfully connected to the network
 *     - ESP_FAIL: Failed to connect to the network
 *     - Other errors passed through from WiFi functions
 */
esp_err_t try_connect(const char *ssid, const char *password) {
    // Check if we are already connected to the requested network
    wifi_ap_record_t current_ap_info;
    if (esp_wifi_sta_get_ap_info(&current_ap_info) == ESP_OK && strcmp((char *)current_ap_info.ssid, ssid) != 0) {
        ESP_LOGI(TAG, "Already connected to %s", current_ap_info.ssid);
        return ESP_OK;
    }

    // Set the WiFi configuration
    wifi_config_t wifi_config = {0};
    memcpy(wifi_config.sta.ssid, ssid, strlen(ssid) + 1);
    memcpy(wifi_config.sta.password, password, strlen(password) + 1);

    // Make sure we're disconnected, set configuration and mode, and then connect
    ESP_RETURN_ON_ERROR(esp_wifi_disconnect(), TAG, "Failed to disconnect from current network");
    ESP_RETURN_ON_ERROR(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config), TAG, "Failed to set WiFi configuration");
    ESP_RETURN_ON_ERROR(esp_wifi_connect(), TAG, "Connection failure while trying to connect to %s", ssid);

    // Notify callbacks that we are connecting
    wifi_status = WIFI_STATUS_CONNECTING;
    for (size_t i = 0; i < WIFI_STATUS_CALLBACK_MAX; i++) {
        if (wifi_status_callbacks[i] != NULL) {
            wifi_status_callbacks[i](WIFI_STATUS_CONNECTING);
        }
    }

    // Wait for connection
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_DISCONNECT_BIT, // Bits to wait for
                                           pdTRUE,                                   // Clear bits on exit
                                           pdFALSE,                                  // Wait for any bit
                                           pdMS_TO_TICKS(10000));                    // 10 second timeout
    if (bits & WIFI_CONNECTED_BIT) {
        return ESP_OK;
    } else {
        xEventGroupSetBits(wifi_event_group, WIFI_NOCONNECT_BIT);
        return ESP_FAIL;
    }
}

/**
 * @brief WiFi event handler for station mode
 *
 * @param arg
 * @param event_base
 * @param event_id
 * @param event_data
 */
void wifi_event_handler(void *_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    (void)_arg; // Unused
    // WiFi events
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START: ESP_LOGI(TAG, "WiFi station started"); break;
            case WIFI_EVENT_STA_CONNECTED:
                wifi_event_sta_connected_t *event = (wifi_event_sta_connected_t *)event_data;
                ESP_LOGI(TAG, "WiFi connected to %s", event->ssid);
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(TAG, "WiFi disconnected");
                xEventGroupSetBits(wifi_event_group, WIFI_DISCONNECT_BIT);
                wifi_status = WIFI_STATUS_DISCONNECTED;
                for (size_t i = 0; i < WIFI_STATUS_CALLBACK_MAX; i++) {
                    if (wifi_status_callbacks[i] != NULL) {
                        wifi_status_callbacks[i](WIFI_STATUS_DISCONNECTED);
                    }
                }
                break;
            default: break;
        }
    }

    // IP events
    if (event_base == IP_EVENT) {
        switch (event_id) {
            case IP_EVENT_STA_GOT_IP:
                ESP_LOGI(TAG, "WiFi station got an IP address");
                xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
                wifi_status = WIFI_STATUS_CONNECTED;
                for (size_t i = 0; i < WIFI_STATUS_CALLBACK_MAX; i++) {
                    if (wifi_status_callbacks[i] != NULL) {
                        wifi_status_callbacks[i](WIFI_STATUS_CONNECTED);
                    }
                }
                break;
            default: break;
        }
    }
}

/**
 * @brief Register event handlers for WiFi and IP events
 *
 * @return
 *     - ESP_OK: Event handlers registered successfully
 */
esp_err_t wifi_register_events() {
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));
    return ESP_OK;
}

/**
 * @brief Task for managing WiFi connections
 */
void wifi_manager_task(void *_arg) {
    (void)_arg; // Unused
    const TickType_t initialDelay = pdMS_TO_TICKS(1000);
    const TickType_t maxDelay     = pdMS_TO_TICKS(60000);
    TickType_t delay              = initialDelay;

    for (;;) {
        // Try to connect to the conference network first and then any saved networks
        if (try_connect(CONFIG_CONFERENCE_WIFI_SSID, CONFIG_CONFERENCE_WIFI_PASSWORD) != ESP_OK) {
#ifdef CONFIG_ALLOW_EXTERNAL_WIFI_NETWORKS
            ESP_LOGW(TAG, "Failed to connect to conference network... trying any extra saved networks");
            for (size_t i = 0; i < CONFIG_EXTERNAL_WIFI_MAX_NETWORKS; i++) {
                if (strlen(wifi_credentials[i].ssid) == 0)
                    continue;
                if (try_connect(wifi_credentials[i].ssid, wifi_credentials[i].password) == ESP_OK) {
                    delay = initialDelay;
                    break;
                }
            }
#else
            ESP_LOGW(TAG, "Failed to connect to conference network");
#endif
        } else {
            delay = initialDelay;
        }

        xEventGroupWaitBits(wifi_event_group,
                            WIFI_DISCONNECT_BIT | WIFI_NOCONNECT_BIT, // Wait for disconnect or unable to connect
                            pdTRUE,                                   // Clear bits on exit
                            pdFALSE,                                  // Wait for any bit
                            portMAX_DELAY);                           // Wait indefinitely
        vTaskDelay(delay);
        delay = delay < maxDelay ? delay * 2 : maxDelay; // Exponential backoff up to max delay
    }
}

void add_wifi_status_callback(wifi_status_callback_t cb) {
    for (size_t i = 0; i < WIFI_STATUS_CALLBACK_MAX; i++) {
        if (wifi_status_callbacks[i] == NULL) {
            wifi_status_callbacks[i] = cb;
            return;
        }
    }
    ESP_LOGE(TAG, "No slots remaining for WiFi status callbacks");
}

wifi_status_t get_wifi_status() {
    return wifi_status;
}

esp_err_t get_wifi_network_list(wifi_ap_record_t **networks, uint16_t *count) {
    wifi_scan_config_t scan_config = {0};
    scan_config.show_hidden        = true;
    scan_config.scan_type          = WIFI_SCAN_TYPE_ACTIVE;
    scan_config.scan_time.active   = (wifi_active_scan_time_t){.min = 100, .max = 200};

    // Scan for available networks
    ESP_RETURN_ON_ERROR(esp_wifi_scan_start(&scan_config, true), TAG, "Failed to start WiFi scan");
    ESP_RETURN_ON_ERROR(esp_wifi_scan_get_ap_num(count), TAG, "Failed to get AP count");

    if (*count == 0) {
        *networks = NULL;
        return ESP_OK;
    }

    // Allocate memory for the network list
    *networks = malloc(sizeof(wifi_ap_record_t) * *count);
    if (*networks == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for network list");
        esp_wifi_clear_ap_list(); // Free the memory allocated by the scan
        return ESP_ERR_NO_MEM;
    }

    // Get the AP records
    if (esp_wifi_scan_get_ap_records(count, *networks) != ESP_OK) {
        free(*networks);
        *networks = NULL;
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t wifi_manager_init() {
    ESP_LOGI(TAG, "Initializing WiFi manager");

    // Intialize NVS
    ESP_RETURN_ON_ERROR(nvs_init(), TAG, "Failed to initialize NVS");
    // Initialize the network stack
    ESP_RETURN_ON_ERROR(esp_netif_init(), TAG, "Failed to initialize network stack");
    // Initialize the event loop
    ESP_RETURN_ON_ERROR(esp_event_loop_create_default(), TAG, "Failed to create event loop");
    // Initialize SNTP
    ESP_RETURN_ON_ERROR(init_sntp(), TAG, "Failed to initialize SNTP");
    // Initialize WiFi with default configuration (conference network)
    ESP_RETURN_ON_ERROR(init_wifi_sta(), TAG, "Failed to initialize WiFi");
    // Register event handlers
    ESP_RETURN_ON_ERROR(wifi_register_events(), TAG, "Failed to register WiFi events");

    // Load any saved WiFi credentials
    deserialize_creds();
    // Create the WiFi event group
    wifi_event_group = xEventGroupCreate();

    // Start the WiFi manager task
    xTaskCreate(wifi_manager_task, "wifi_manager_task", 4096, NULL, 5, NULL);

    return ESP_OK;
}
