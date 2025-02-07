#include <string.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_timer.h"

#include "api.h"
#include "ota.h"
#include "version.h"

static const char *TAG = "badge/ota";

#define OTA_CHECK_INTERVAL_MS 2 * 60 * 60 * 1000 // 2 hours

// OTA state tracking
ota_state_t ota_state = {
    .last_check_time = 0,               //
    .status          = OTA_STATUS_IDLE, //
    .progress        = {0, 0},          //
    .message         = ""               //
};

ota_state_callback_t ota_state_callbacks[OTA_STATE_CALLBACK_MAX] = {0};

// Event queue for OTA events
QueueHandle_t ota_event_queue = NULL;
TaskHandle_t ota_task_handle  = NULL;
TimerHandle_t ota_timer       = NULL;

static void ota_state_updated() {
    if (ota_event_queue == NULL) {
        ESP_LOGW(TAG, "OTA event queue not initialized");
        return;
    }

    xQueueSend(ota_event_queue, &ota_state, 0);

    for (int i = 0; i < OTA_STATE_CALLBACK_MAX; i++) {
        if (ota_state_callbacks[i] != NULL) {
            ota_state_callbacks[i](ota_state);
        }
    }
}

static void ota_task(void *arg) {
    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // Block until notified

        ESP_LOGI(TAG, "Checking for updates...");
        uint32_t now = esp_timer_get_time() / 1000;
        if (ota_state.last_check_time != 0 && now - ota_state.last_check_time < OTA_CHECK_INTERVAL_MS) {
            ESP_LOGI(TAG, "Skipping update check (last check was %lu milliseconds ago)", now - ota_state.last_check_time);
            continue;
        }

        // Reset the OTA state
        ota_state.status   = OTA_STATUS_CHECKING;
        ota_state.progress = (ota_progress_t){0, 0};
        ota_state.message  = "Checking for updates...";

        // Send the updated OTA state to the event queue
        ota_state_updated();

        // Check the latest firmware version from the server
        char latest_version[32]       = {0};
        api_result_t *firmware_result = api_get_firmware_version();
        if (firmware_result == NULL) {
            ota_state.status  = OTA_STATUS_CHECK_FAILED;
            ota_state.message = "Failed to check for updates";
            ota_state_updated();
            continue;
        } else {
            api_firmware_data_t *data = (api_firmware_data_t *)firmware_result->data;
            strncpy(latest_version, data->value, sizeof(latest_version));
            api_free_result(firmware_result, true);
            firmware_result = NULL;
        }

        // Compare the latest version with the current version
        if (firmware_version_compare(latest_version) >= 0) {
            ota_state.last_check_time = now;
            ota_state.status          = OTA_STATUS_SUCCESS;
            ota_state.message         = "No updates available";
            ota_state_updated();
            continue;
        }

        // Download the firmware update
        api_err_t err = api_do_firmware_update();
        if (err != API_OK) {
            ota_state.status  = OTA_STATUS_FAILED;
            ota_state.message = "Failed to download firmware update";
            ota_state_updated();
        }
    }
}

static void ota_timer_callback(TimerHandle_t xTimer) {
    xTaskNotifyGive(ota_task_handle);
}

void ota_init() {
    if (ota_event_queue == NULL && (ota_event_queue = xQueueCreate(OTA_EVENT_QUEUE_SIZE, sizeof(ota_state_t))) == NULL) {
        ESP_LOGE(TAG, "Failed to create OTA event queue");
        return;
    }

    if (ota_timer == NULL) {
        ota_timer = xTimerCreate("OTA Timer", pdMS_TO_TICKS(OTA_CHECK_INTERVAL_MS), pdTRUE, NULL, ota_timer_callback);
        if (ota_timer == NULL) {
            ESP_LOGE(TAG, "Failed to create OTA timer");
            return;
        }
    }

    if (ota_task_handle == NULL) {
        xTaskCreate(ota_task, "OTA Task", 8 * 1024, NULL, 5, &ota_task_handle);
        if (ota_task_handle == NULL) {
            ESP_LOGE(TAG, "Failed to create OTA task");
            return;
        }
    }

    // Start the OTA timer
    xTimerStart(ota_timer, 0);
}

void ota_check() {
    if (ota_task_handle == NULL) {
        ESP_LOGE(TAG, "OTA task not initialized");
        return;
    }
    xTaskNotifyGive(ota_task_handle);
}

void ota_add_state_callback(ota_state_callback_t callback) {
    for (int i = 0; i < OTA_STATE_CALLBACK_MAX; i++) {
        if (ota_state_callbacks[i] == NULL) {
            ota_state_callbacks[i] = callback;
            break;
        }
    }
}

void set_ota_status(ota_status_t status) {
    ota_state.status = status;
    ota_state_updated();
}

void set_ota_progress(ota_progress_t progress) {
    ota_state.progress = progress;
    if (progress.bytes_total > 0) {
        ota_state.status  = OTA_STATUS_DOWNLOADING;
        ota_state.message = "Downloading firmware update...";
    }
    ota_state_updated();
}

void set_ota_message(const char *message) {
    ota_state.message = message;
    ota_state_updated();
}

ota_status_t get_ota_status() {
    return ota_state.status;
}

ota_progress_t get_ota_progress() {
    return ota_state.progress;
}

const char *get_ota_message() {
    return ota_state.message;
}