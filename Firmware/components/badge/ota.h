#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define OTA_EVENT_QUEUE_SIZE 10

typedef enum {
    OTA_STATUS_IDLE,
    OTA_STATUS_CHECKING,
    OTA_STATUS_CHECK_FAILED,
    OTA_STATUS_DOWNLOADING,
    OTA_STATUS_INSTALLING,
    OTA_STATUS_FAILED,
    OTA_STATUS_SUCCESS,
} ota_status_t;

typedef struct {
    int bytes_received;
    int bytes_total;
} ota_progress_t;

typedef struct {
    uint32_t last_check_time;
    ota_status_t status;
    ota_progress_t progress;
    const char *message;
} ota_state_t;

typedef void (*ota_state_callback_t)(ota_state_t state);

#define OTA_STATE_CALLBACK_MAX 5

// Event queue for OTA events
extern QueueHandle_t ota_event_queue;

void ota_init();
void ota_check();

void ota_add_state_callback(ota_state_callback_t callback);

void set_ota_status(ota_status_t status);
void set_ota_progress(ota_progress_t progress);
void set_ota_message(const char *message);

ota_status_t get_ota_status();
ota_progress_t get_ota_progress();
const char *get_ota_message();

#ifdef __cplusplus
}
#endif
