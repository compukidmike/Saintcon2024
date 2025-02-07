#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"
#include "ir_comm.h"

// The internal representation of codes is XORed with this key before being sent
#define IR_KEY 0xDEADBEEF

// IR message type IDs (MTI)
typedef enum {
    IR_MTI_TOWER   = 0x0,
    IR_MTI_LEVELUP = 0x1,
    IR_MTI_SAVIOR  = 0x2,
    IR_MTI_PVP     = 0x3,
    IR_MTI_VENDING = 0x4,
    IR_MTI_AUTH    = 0x5,
    // ... other future types here
    IR_MTI_MULTIFRAME = 0xF,
} ir_message_type_id_t;
typedef enum {
    IR_CODE_PRIORITY_NORMAL,
    IR_CODE_PRIORITY_HIGH,
} ir_code_priority_t;
extern const ir_message_type_id_t HIGH_PRIORITY_TYPES[];

// Structured high-level representation of an IR code
typedef struct {
    union {
        uint32_t code;
        struct {
            uint16_t command : 16; // 16-bit command - bits 15-0 of the full IR code
            uint16_t address : 16; // 16-bit address - bits 31-16 of the full IR code
        };
    };
    int64_t timestamp;

    // Additional fields for convenience
    uint32_t decoded;
    ir_message_type_id_t message_type;
    ir_code_priority_t priority;
} ir_code_t;

/**
 * @brief Initialize the IR code handling
 *
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t badge_ir_init();

/**
 * @brief Get an ir_code_t struct from a raw IR code in uint32_t format
 *
 * @param code The raw IR code in uint32_t format
 * @return An ir_code_t struct
 */
ir_code_t badge_ir_get_code(uint32_t code);

/**
 * @brief Add callback for received IR codes
 *
 * @param callback The callback function to add
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t badge_ir_add_rx_callback(ir_rx_callback_t callback);

/**
 * @brief Remove callback for received IR codes
 *
 * @param callback The callback function to remove
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t badge_ir_remove_rx_callback(ir_rx_callback_t callback);

/**
 * @brief Enable or disable IR Rx buffer
 *
 * @param enable Enable or disable the IR Rx buffer
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t badge_ir_enable_rx_buffer(bool enable);

#ifdef __cplusplus
}
#endif
