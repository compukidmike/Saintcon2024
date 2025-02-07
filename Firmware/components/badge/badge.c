#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include "esp_app_desc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_spiffs.h"
#include "esp_timer.h"

#include "api.h"
#include "badge.h"
#include "display.h"
#include "ir.h"
#include "minibadge.h"
#include "nvs.h"
#include "towers.h"
#include "ui.h"

static const char *TAG = "badge";

// Badge state struct
badge_state_t badge_state = {
    .ready       = false,
    .wifi_status = WIFI_STATUS_DISCONNECTED,
};

// Badge events to be handled by the main event badge task
#define BADGE_EVENT_QUEUE_SIZE      10
#define BADGE_EVENT_TASK_STACK_SIZE 8 * 1024

typedef enum {
    BADGE_EVENT_NONE,
    BADGE_EVENT_READY,
    // BADGE_EVENT_IR,
    BADGE_EVENT_WIFI_STATUS,
    BADGE_EVENT_SEND_MINIBADGE_STATUS,
    // BADGE_EVENT_API,
    // BADGE_EVENT_DISPLAY,
    BADGE_EVENT_OTA,
    BADGE_EVENT_OTA_CHECK,
} badge_event_type_t;

// Badge event data
typedef struct {
    badge_event_type_t type;
    union {
        // ir_event_t ir;
        wifi_status_t wifi_status;
        // api_event_t api;
        // display_event_t display;
        ota_state_t ota;
    } data;
} badge_event_t;

// Badge event queue
QueueHandle_t badge_event_queue = NULL;

// Screen timer
esp_timer_handle_t screen_timer = NULL;
static bool screen_off          = false;
static void screen_timeout_callback(void *arg);

// Badge event handling task
void badge_event_task(void *_args);

// Wifi status callback function
void wifi_status_callback(wifi_status_t wifi_status) {
    badge_event_t event = {.type = BADGE_EVENT_WIFI_STATUS, .data.wifi_status = wifi_status};
    xQueueSend(badge_event_queue, &event, 0);
}

void ota_state_callback(ota_state_t ota_state) {
    ESP_LOGD(TAG, "OTA state callback: %d", ota_state.status);
    set_ota_state(ota_state);
    badge_event_t event = {.type = BADGE_EVENT_OTA, .data.ota = ota_state};
    xQueueSend(badge_event_queue, &event, 0);
}

void minibadge_event_callback(minibadge_event_t event) {
    ESP_LOGD(TAG, "Minibadge %s - slot %d", event.type == MINIBADGE_EVENT_INSERTED ? "inserted" : "removed", event.slot + 1);

    // Make sure we are connected to WiFi before sending minibadge events
    if (badge_state.wifi_status != WIFI_STATUS_CONNECTED) {
        ESP_LOGW(TAG, "Not connected to WiFi, not sending minibadge event");
        return;
    }

    // Send the minibadge event
    badge_event_t badge_event = {.type = BADGE_EVENT_SEND_MINIBADGE_STATUS};
    xQueueSend(badge_event_queue, &badge_event, 0);
}

static void send_minibadge_status() {
    // Get the serial numbers of the minibadges
    char minibadge_slot_serial[MINIBADGE_SLOT_COUNT][32 + 1] = {0}; // 16 bytes but we'll be converting to hex
    for (int i = 0; i < MINIBADGE_SLOT_COUNT; i++) {
        minibadge_device_t *device = &minibadge_devices[i];
        if (device->type == MINIBADGE_TYPE_EEPROM_SERIALIZED) {
            for (int j = 0; j < 16; j++) {
                snprintf(&minibadge_slot_serial[i][j * 2], 3, "%02X", device->serial[j]);
            }
        }
    }

    ESP_LOGD(TAG, "Minibadge slot 1 serial: %s", minibadge_slot_serial[0]);
    ESP_LOGD(TAG, "Minibadge slot 2 serial: %s", minibadge_slot_serial[1]);

    // Send the minibadge event to the API
    api_result_t *result = api_equip_minibadge(&minibadge_slot_serial[0], &minibadge_slot_serial[1]);
    if (result == NULL || result->status == false) {
        ESP_LOGE(TAG, "Failed to equip minibadge");
        api_free_result(result, true);
    } else {
        // Free the old minibadge data and set the new data
        if (badge_state.minibadges != NULL) {
            api_free_result_data(badge_state.minibadges, API_EQUIP_MINIBADGE);
        }
        badge_state.minibadges = (api_equip_minibadge_t *)result->data;

        // Free the result but keep the data since we're pointing to it now
        api_free_result(result, false);

        // Update the UI with the new minibadge status
        if (ui_ready()) {
            set_status_minibadge_update();
        }
    }
}

void screen_reset_timeout() {
    if (screen_off) {
        set_backlight(badge_config.brightness);
        screen_off = false;
    }
    if (esp_timer_is_active(screen_timer)) {
        ESP_ERROR_CHECK(esp_timer_stop(screen_timer));
    }
    ESP_ERROR_CHECK(esp_timer_start_once(screen_timer, badge_config.screen_timeout * 1000000));
}

static void screen_timeout_callback(void *arg) {
    set_backlight(20);
    screen_off = true;
}

esp_err_t badge_init() {
    esp_err_t err = load_badge_config();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Loading default badge config due to error");
        badge_config = BADGE_DEFAULTS;
    }
    ESP_LOGI(
        TAG,
        "Badge config: version=%lu, hw_pass=%d, registered=%d, wrist=%d, brightness=%d, screen_timeout=%lu, id=%d, handle=%s, "
        "xp=%d, level=%d, enabled=%d, badge_team=%d, can_level=%d, custom_wifi=%d, staff=%d, blackbadge=%d, community=%s, "
        "community_levels=%d, coins=%d",
        badge_config.version, badge_config.hw_pass, badge_config.registered, badge_config.wrist, badge_config.brightness,
        badge_config.screen_timeout, badge_config.id, badge_config.handle, badge_config.xp, badge_config.level,
        badge_config.enabled, badge_config.badge_team, badge_config.can_level, badge_config.custom_wifi, badge_config.staff,
        badge_config.blackbadge, badge_config.community, badge_config.community_levels, badge_config.coins);

    // Create the badge event queue
    badge_event_queue = xQueueCreate(BADGE_EVENT_QUEUE_SIZE, sizeof(badge_event_t));

    // Print the firmware version
    const esp_app_desc_t *app_desc = esp_app_get_description();
    ESP_LOGI(TAG,
             "Badge firmware information:\n"
             "  - Version: %s\n"
             "  - Project name: %s\n"
             "  - Compile time: %s\n"
             "  - Compile date: %s\n"
             "  - IDF version: %s\n"
             "  - SHA256: %s",
             app_desc->version, app_desc->project_name, app_desc->time, app_desc->date, app_desc->idf_ver,
             esp_app_get_elf_sha256_str());

    // Configure SPIFFS on the storage partition
    esp_vfs_spiffs_conf_t spiffs_conf = {
        .base_path              = "/spiffs",
        .partition_label        = NULL,
        .max_files              = 5,
        .format_if_mount_failed = true,
    };
    err = esp_vfs_spiffs_register(&spiffs_conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(err));
    } else {
        // Print SPIFFS info
        size_t total_bytes, used_bytes;
        err = esp_spiffs_info(NULL, &total_bytes, &used_bytes);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to get SPIFFS info (%s)", esp_err_to_name(err));
        } else {
            ESP_LOGI(TAG, "SPIFFS: %d/%d bytes used", used_bytes, total_bytes);
            if (esp_log_level_get(TAG) >= ESP_LOG_DEBUG) {
                // List files and their sizes in SPIFFS
                DIR *dir = opendir("/spiffs");
                if (dir == NULL) {
                    ESP_LOGE(TAG, "Failed to open SPIFFS directory");
                } else {
                    struct dirent *entry;
                    while ((entry = readdir(dir)) != NULL) {
                        struct stat st;
                        stat(entry->d_name, &st);
                        ESP_LOGI(TAG, "  - File: %s, Size: %ld", entry->d_name, st.st_size);
                    }
                    closedir(dir);
                }
            }
        }
    }

    // Add wifi status callback
    add_wifi_status_callback(wifi_status_callback);

    // Initialize OTA and add a state callback
    ota_init();
    ota_add_state_callback(ota_state_callback);

    // Initialize IR handler
    err = badge_ir_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize IR handler: %s", esp_err_to_name(err));
    }
    err = badge_ir_enable_rx_buffer(true);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable IR RX buffer: %s", esp_err_to_name(err));
    }

    // Initialize the minibadge monitor
    err = minibadge_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize minibadge monitor: %s", esp_err_to_name(err));
    }
    minibadge_add_event_callback(minibadge_event_callback);

    // Create the screen timer
    const esp_timer_create_args_t screen_timer_args = {
        .callback = &screen_timeout_callback,
        .name     = "screen_timer",
    };
    err = esp_timer_create(&screen_timer_args, &screen_timer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create screen timer: %s", esp_err_to_name(err));
    }
    screen_reset_timeout();

    // Initialize the tower tracker
    tower_tracker_init();

    // Create the badge event handling task
    xTaskCreate(badge_event_task, "badge_event_task", BADGE_EVENT_TASK_STACK_SIZE, NULL, 5, NULL);

    // Send a ready event
    badge_event_t event = {.type = BADGE_EVENT_READY};
    xQueueSend(badge_event_queue, &event, 0);

    return err;
}

esp_err_t set_badge_handle(const char *handle) {
    if (strlen(handle) >= BADGE_HANDLE_LENGTH) {
        ESP_LOGE(TAG, "Handle too long");
        return ESP_ERR_INVALID_ARG;
    }
    snprintf(badge_config.handle, sizeof(badge_config.handle), "%s", handle);
    return save_badge_config();
}

esp_err_t set_badge_wrist(badge_wrist_t wrist) {
    ESP_LOGD(TAG, "Setting wrist: %d (%s)", wrist, wrist == BADGE_WRIST_LEFT ? "left" : "right");
    badge_config.wrist = wrist;
    esp_err_t ret      = save_badge_config();
    set_display_orientation(badge_config.wrist == BADGE_WRIST_LEFT ? DISPLAY_ORIENTATION_LANDSCAPE_FLIP
                                                                   : DISPLAY_ORIENTATION_LANDSCAPE);
    return ret;
}

esp_err_t set_screen_timeout(uint32_t timeout) {
    ESP_LOGD(TAG, "Setting screen timeout: %ld", timeout);
    badge_config.screen_timeout = timeout;
    return save_badge_config();
}

void badge_event_task(void *_args) {
    badge_event_t event;
    while (1) {
        if (xQueueReceive(badge_event_queue, &event, portMAX_DELAY) == pdTRUE) {
            switch (event.type) {
                case BADGE_EVENT_READY: //
                    badge_state.ready = true;
                    break;
                case BADGE_EVENT_WIFI_STATUS:
                    // Update the wifi status in our state
                    badge_state.wifi_status = event.data.wifi_status;

                    // Update the UI with the current wifi status
                    if (ui_ready()) {
                        set_status_wifi_state(badge_state.wifi_status);
                    }

                    // Things to do when wifi is connected
                    if (badge_state.wifi_status == WIFI_STATUS_CONNECTED) {
                        // Check for updates if the badge is in a ready state
                        if (badge_state.ready) {
                            // Sync badge data from the server
                            if (badge_config.registered) {
                                api_result_t *result = api_get_badge_data();
                                if (result == NULL) {
                                    ESP_LOGE(TAG, "Failed to get badge data");
                                } else {
                                    api_badge_data_t *badge_data = (api_badge_data_t *)result->data;
                                    ESP_LOGD(TAG,
                                             "Badge data: \n"
                                             "  - ID: %d\n"
                                             "  - Badge ID: %s\n"
                                             "  - Handle: %s\n"
                                             "  - XP: %d\n"
                                             "  - Level: %d\n"
                                             "  - Enabled: %d\n"
                                             "  - Badge Team: %d\n"
                                             "  - Staff: %d\n"
                                             "  - Black Badge: %d\n"
                                             "  - Can Level: %d\n"
                                             "  - Community: %s\n"
                                             "  - Community Levels: %d\n"
                                             "  - Is Savior: %d\n"
                                             "  - Coins: %d\n",
                                             badge_data->id, badge_data->badge_id, badge_data->handle, badge_data->xp,
                                             badge_data->level, badge_data->enabled, badge_data->badge_team, badge_data->staff,
                                             badge_data->blackbadge, badge_data->can_level,
                                             badge_data->community != NULL ? badge_data->community : "",
                                             badge_data->community_levels, badge_data->is_savior, badge_data->coins);
                                    if (badge_data->handle != NULL) {
                                        strncpy(badge_config.handle, badge_data->handle, sizeof(badge_config.handle));
                                        badge_config.id         = badge_data->id;
                                        badge_config.xp         = badge_data->xp;
                                        badge_config.level      = badge_data->level;
                                        badge_config.enabled    = badge_data->enabled;
                                        badge_config.badge_team = badge_data->badge_team;
                                        badge_config.staff      = badge_data->staff;
                                        badge_config.blackbadge = badge_data->blackbadge;
                                        badge_config.can_level  = badge_data->can_level;
                                        if (badge_data->community != NULL) {
                                            strncpy(badge_config.community, badge_data->community,
                                                    sizeof(badge_config.community));
                                        }
                                        badge_config.community_levels = badge_data->community_levels;
                                        badge_config.coins            = badge_data->coins;
                                        save_badge_config();
                                    }
                                    api_free_result(result, true);
                                }
                            }

                            // Try to register the badge
                            else {
                                if (strlen(badge_config.handle) > 0) {
                                    api_result_t *result = api_register(badge_config.handle);
                                    if (result == NULL) {
                                        ESP_LOGE(TAG, "Failed to register badge");
                                    } else {
                                        api_free_result(result, true);
                                        ESP_LOGI(TAG, "Badge registered successfully!");
                                        badge_config.registered = true;
                                        save_badge_config();
                                    }
                                }
                            }

                            // Do things that require the badge to be registered, on the network, and at the main screen
                            if (get_screen() == SCREEN_MAIN && badge_config.registered) {
                                ESP_LOGD(TAG, "WiFi connected - running ota_check()");
                                ota_check();
                                ESP_LOGD(TAG, "WiFi connected - sending minibadge status");
                                send_minibadge_status();
                                ESP_LOGD(TAG, "WiFi connected - refreshing tower info");
                                tower_info_refresh(REFRESH_ALL);
                            }
                        } else {
                            ESP_LOGW(TAG, "Badge not ready, skipping badge registration and update check");
                        }
                    }
                    break;
                case BADGE_EVENT_SEND_MINIBADGE_STATUS: //
                    send_minibadge_status();
                    break;
                case BADGE_EVENT_OTA: //
                    ESP_LOGD(TAG, "OTA event message: %s", event.data.ota.message);
                    break;
                default: //
                    ESP_LOGW(TAG, "Unknown event type: %d", event.type);
                    break;
            }
        }
    }
}
