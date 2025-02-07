#include <stdint.h>
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "api.h"
#include "badge.h"
#include "ir.h"
#include "ui.h"

static const char *TAG = "badge/ir";

#define MAX_IR_CODES              16
#define HIGH_PRIORITY_DEBOUNCE_MS 1000
#define HIGH_PRIORITY_BIT         (1 << 0)
#define MAX_IR_CALLBACKS          10

// List of high-priority IR message types
const ir_message_type_id_t HIGH_PRIORITY_TYPES[] = {
    IR_MTI_LEVELUP, //
    IR_MTI_SAVIOR,  //
    IR_MTI_PVP,     //
    IR_MTI_VENDING, //
    IR_MTI_AUTH,    //
};

// Ring buffer for received IR codes
static ir_code_t ir_code_buffer[MAX_IR_CODES];
static uint8_t ir_code_head  = 0;
static uint8_t ir_code_tail  = 0;
static uint8_t ir_code_count = 0;
static SemaphoreHandle_t ir_code_mutex;

// Last high-priority code
static ir_code_t last_high_priority_code = {0};

// Event group for high-priority IR code handling
static EventGroupHandle_t ir_event_group;

// List of callbacks for additional IR code handling
static ir_rx_callback_t ir_callbacks[MAX_IR_CALLBACKS];

// Flag for enabling or disabling the IR Rx buffer
static bool ir_rx_buffer_enabled = true;

// Function prototypes
void ir_rx_callback(uint16_t address, uint16_t command);
void ir_code_task(void *_arg);
void ir_high_priority_task(void *_arg);
void route_high_priority_code(ir_code_t *ir_code, const char *msg);

esp_err_t badge_ir_init() {
    // Initialize mutex
    ir_code_mutex = xSemaphoreCreateMutex();
    if (ir_code_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create IR code mutex");
        return ESP_FAIL;
    }

    // Initialize event group
    ir_event_group = xEventGroupCreate();
    if (ir_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create IR event group");
        return ESP_FAIL;
    }

    // Create tasks
    xTaskCreate(ir_code_task, "ir_code_task", 5 * 1024, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(ir_high_priority_task, "ir_high_priority_task", 4096, NULL, tskIDLE_PRIORITY + 2, NULL);

    // Initialize IR communication module with the callback
    ir_init(ir_rx_callback);
    ir_enable_rx();
    ir_enable_tx();

    return ESP_OK;
}

ir_code_t badge_ir_get_code(uint32_t code) {
    ir_code_t ir_code = {
        .code      = code,
        .timestamp = esp_timer_get_time(),
    };
    ir_code.decoded      = ir_code.code ^ IR_KEY;
    ir_code.message_type = (ir_message_type_id_t)(ir_code.decoded >> 28); // MTI is bits 31-28 of the full IR code
    ir_code.priority     = IR_CODE_PRIORITY_NORMAL;

    // Determine if this is a high-priority code
    for (size_t i = 0; i < sizeof(HIGH_PRIORITY_TYPES) / sizeof(HIGH_PRIORITY_TYPES[0]); i++) {
        // MTI is bits 31-28 of the full IR code or bits 15-12 of the address portion of the NEC code
        if (ir_code.message_type == HIGH_PRIORITY_TYPES[i]) {
            ir_code.priority = IR_CODE_PRIORITY_HIGH;
            break;
        }
    }

    return ir_code;
}

void ir_rx_callback(uint16_t address, uint16_t command) {
    if (ir_rx_buffer_enabled) {
        // Initialize the IR code structure
        ir_code_t ir_code = badge_ir_get_code(((uint32_t)address << 16) | command);

        // For debugging, log the IR code
        ESP_LOGD(TAG, "Received IR code: 0x%08X [0x%04X 0x%04X] (decoded: 0x%08X) type: [%d] %s", (unsigned int)ir_code.code,
                 (unsigned int)ir_code.address, (unsigned int)ir_code.command, (unsigned int)ir_code.decoded,
                 ir_code.message_type,
                 ir_code.message_type == IR_MTI_TOWER     ? "TOWER"
                 : ir_code.message_type == IR_MTI_LEVELUP ? "LEVELUP"
                 : ir_code.message_type == IR_MTI_SAVIOR  ? "SAVIOR"
                 : ir_code.message_type == IR_MTI_PVP     ? "PVP"
                 : ir_code.message_type == IR_MTI_VENDING ? "VENDING"
                 : ir_code.message_type == IR_MTI_AUTH    ? "AUTH"
                                                          : "UNKNOWN");

        // Lock the mutex to protect the buffer
        xSemaphoreTake(ir_code_mutex, portMAX_DELAY);

        // Handle high-priority codes with debounce
        if (ir_code.priority == IR_CODE_PRIORITY_HIGH) {
            int64_t current_time = esp_timer_get_time();
            if (ir_code.code != last_high_priority_code.code ||
                current_time - last_high_priority_code.timestamp > HIGH_PRIORITY_DEBOUNCE_MS * 1000) {
                last_high_priority_code = ir_code;

                // Notify the event group
                xEventGroupSetBits(ir_event_group, HIGH_PRIORITY_BIT);
            }
        }

        // Normal priority codes can go in the buffer for later processing
        else {
            // Check if the code is already in the buffer
            bool already_in_buffer = false;
            for (int i = 0; i < ir_code_count; i++) {
                int index = (ir_code_tail + i) % MAX_IR_CODES;
                if (ir_code_buffer[index].address == address && ir_code_buffer[index].command == command) {
                    already_in_buffer = true;
                    break;
                }
            }

            if (!already_in_buffer) {
                // Add code to buffer
                ir_code_buffer[ir_code_head] = ir_code;

                // Advance head
                ir_code_head = (ir_code_head + 1) % MAX_IR_CODES;

                if (ir_code_count < MAX_IR_CODES) {
                    ir_code_count++;
                } else {
                    // Buffer is full, advance tail
                    ir_code_tail = (ir_code_tail + 1) % MAX_IR_CODES;
                }

                // If the tower tracker is ready, we can use it to check for tower codes for a faster UI response
                if (tower_tracker_ready()) {
                    for (int i = 0; i < ir_code_count; i++) {
                        if (ir_code_buffer[i].message_type == IR_MTI_TOWER) {
                            int tower_id = tower_ir_to_id(ir_code_buffer[i].code);
                            if (tower_id != -1) {
                                tower_seen(tower_id);
                            }
                        }
                    }
                }
            }
        }

        xSemaphoreGive(ir_code_mutex);
    }

    // Call additional callbacks
    for (int i = 0; i < MAX_IR_CALLBACKS; i++) {
        if (ir_callbacks[i] != NULL) {
            ir_callbacks[i](address, command);
        }
    }
}

void ir_code_task(void *_arg) {
    (void)_arg;
    for (;;) {
        // Wait for 10 seconds
        vTaskDelay(pdMS_TO_TICKS(10000));

        // Lock the mutex
        xSemaphoreTake(ir_code_mutex, portMAX_DELAY);

        if (ir_code_count > 0) {
            // Collect the codes
            uint32_t ir_codes[MAX_IR_CODES];
            size_t num_codes = 0;
            for (int i = 0; i < ir_code_count; i++) {
                int index             = (ir_code_tail + i) % MAX_IR_CODES;
                ir_codes[num_codes++] = ((uint32_t)ir_code_buffer[index].address << 16) | ir_code_buffer[index].command;
            }

            // Call API
            api_result_t *result = api_check_ir_codes(ir_codes, num_codes);
            if (result != NULL) {
                if (tower_tracker_ready()) {
                    api_ir_code_result_t *ir_code_result = (api_ir_code_result_t *)result->data;
                    for (int i = 0; i < ir_code_result->count; i++) {
                        if (ir_code_result->ir_codes[i].is_valid && ir_code_result->ir_codes[i].type == IR_CODE_TYPE_TOWER) {
                            int tower_id = tower_ir_to_id(ir_code_result->ir_codes[i].code);
                            if (tower_id != -1) {
                                tower_seen(tower_id);
                            } else {
                                ESP_LOGW(TAG, "Received IR code for unknown tower: 0x%08X",
                                         (unsigned int)ir_code_result->ir_codes[i].code);
                            }
                        }
                    }
                }
                api_free_result(result, true);

                // Clear the buffer
                ir_code_head  = 0;
                ir_code_tail  = 0;
                ir_code_count = 0;
            } else {
                ESP_LOGE(TAG, "Failed periodic IR code check");
            }
        }

        xSemaphoreGive(ir_code_mutex);
    }
}

void ir_high_priority_task(void *_arg) {
    (void)_arg;
    for (;;) {
        // Wait indefinitely for the high-priority bit to be set
        xEventGroupWaitBits(ir_event_group, HIGH_PRIORITY_BIT, pdTRUE, pdFALSE, portMAX_DELAY);

        // Handle the high-priority code
        xSemaphoreTake(ir_code_mutex, portMAX_DELAY);

        // Assuming the last high-priority code is the one to process
        const uint32_t ir_codes[] = {last_high_priority_code.code};
        api_result_t *result      = api_check_ir_codes(ir_codes, 1);
        if (result != NULL) {
            api_ir_code_result_t *ir_code_result = (api_ir_code_result_t *)result->data;
            if (ir_code_result != NULL) {
                for (int i = 0; i < ir_code_result->count; i++) {
                    if (ir_code_result->ir_codes[i].is_valid) {
                        route_high_priority_code(&last_high_priority_code, ir_code_result->ir_codes[i].response);
                    }
                }
            }
            api_free_result(result, true);
        } else {
            ESP_LOGE(TAG, "Failed to check high-priority IR codes");
        }

        xSemaphoreGive(ir_code_mutex);
    }
}

esp_err_t badge_ir_add_rx_callback(ir_rx_callback_t callback) {
    for (int i = 0; i < MAX_IR_CALLBACKS; i++) {
        if (ir_callbacks[i] == NULL) {
            ir_callbacks[i] = callback;
            return ESP_OK;
        }
    }
    return ESP_FAIL;
}

esp_err_t badge_ir_remove_rx_callback(ir_rx_callback_t callback) {
    for (int i = 0; i < MAX_IR_CALLBACKS; i++) {
        if (ir_callbacks[i] == callback) {
            ir_callbacks[i] = NULL;
            return ESP_OK;
        }
    }
    return ESP_FAIL;
}

esp_err_t badge_ir_enable_rx_buffer(bool enable) {
    ir_rx_buffer_enabled = enable;
    return ESP_OK;
}

void route_high_priority_code(ir_code_t *ir_code, const char *msg) {
    switch (ir_code->message_type) {
        case IR_MTI_LEVELUP: // Level up code from staff badge
            handle_levelup_code(ir_code);
            break;
        case IR_MTI_SAVIOR: // Revival code from player with Hydra power-up
            handle_savior_code(ir_code, msg);
            break;
        case IR_MTI_PVP: // PvP code when players are doing PvP battles
            // Not yet implemented
            // handle_pvp_code(ir_code);
            break;
        case IR_MTI_VENDING: // Vending machine code
            // For now we only care that the code was sent to the API
            // handle_vending_code(ir_code);
            break;
        case IR_MTI_AUTH: // Authentication code for community vending
            // For now we only care that the code was sent to the API
            // handle_auth_code(ir_code);
            break;
        default: //
            ESP_LOGW(TAG, "Unhandled high-priority IR code type: %d", ir_code->message_type);
            break;
    }
}
