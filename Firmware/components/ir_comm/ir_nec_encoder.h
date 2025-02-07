#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "driver/rmt_encoder.h"

// NEC scan code structure
typedef struct {
    uint16_t address;
    uint16_t command;
} ir_nec_scan_code_t;

// NEC encoder configuration
typedef struct {
    uint32_t resolution; // Encoder resolution in Hz
} ir_nec_encoder_config_t;

esp_err_t rmt_new_ir_nec_encoder(const ir_nec_encoder_config_t *config, rmt_encoder_handle_t *ret_encoder);

#ifdef __cplusplus
}
#endif
