#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#include "display.h"

/*******************************************************************************
 *               TYPES + CONSTANTS USED IN BADGE CONFIGURATION                 *
 * --------------------------------------------------------------------------- *
 * NOTE: If any of these change, the badge configuration version should be     *
 *       incremented and the old value(s) added to the relevant config_vX.h    *
 *       files.                                                                *
 *******************************************************************************/

#define BADGE_HANDLE_LENGTH 64

// Wrist selection
typedef enum {
    BADGE_WRIST_LEFT,
    BADGE_WRIST_RIGHT,
} badge_wrist_t;

/*******************************************************************************
 *                          CONFIGURATION VERSIONING                           *
 * --------------------------------------------------------------------------- *
 * When the badge configuration changes, a version header file should be       *
 * created in the config directory with the new configuration struct and then  *
 * included here. This allows for easy migration between different versions.   *
 *******************************************************************************/

// Include all config versions here
#include "config/config_v1.h"
#include "config/config_v2.h"
#include "config/config_v3.h"
#include "config/config_v4.h"
#include "config/config_v5.h"

// Define the current config version
#define BADGE_CONFIG_VERSION 5

/*******************************************************************************
 *                            BADGE CONFIGURATION                              *
 *******************************************************************************/

// Define the current config struct
#define BCV(v) badge_config_v##v##_t
#define BCT(v) BCV(v)
typedef BCT(BADGE_CONFIG_VERSION) badge_config_t;

// Define the default config struct
#define BCDV(v)        BADGE_DEFAULTS_V##v
#define BCD(v)         BCDV(v)
#define BADGE_DEFAULTS BCD(BADGE_CONFIG_VERSION)

// Define the default config struct
extern badge_config_t badge_config;
extern badge_config_t badge_defaults;

/**
 * @brief Load the badge configuration
 *
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t load_badge_config();

/**
 * @brief Save the badge configuration
 *
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t save_badge_config();

#ifdef __cplusplus
}
#endif