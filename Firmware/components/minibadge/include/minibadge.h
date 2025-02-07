#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "esp_err.h"
#include "esp_event.h"

#define MINIBADGE_I2C_ADDR_EEPROM 0x50
#define MINIBADGE_I2C_ADDR_DPAD   0x38
#define MINIBADGE_I2C_ADDR_CUSTOM 0x50 // TODO: Update this if we change it later

#define MINIBADGE_EEPROM_ADDR_DATA   0x00 // EEPROM address for the data or name string
#define MINIBADGE_EEPROM_ADDR_SERIAL 0x80 // EEPROM address for the serial number

#define MINIBADGE_SLOT_COUNT 2

// Minibadge type enum
typedef enum {
    MINIBADGE_TYPE_NONE,
    MINIBADGE_TYPE_EEPROM_BASIC,
    MINIBADGE_TYPE_EEPROM_SERIALIZED,
    MINIBADGE_TYPE_CUSTOM,
} minibadge_type_t;

// Minibadge slot enum
typedef enum {
    MINIBADGE_SLOT_1,
    MINIBADGE_SLOT_2,
} minibadge_slot_t;

// Minibadge device struct
typedef struct {
    minibadge_type_t type;
    minibadge_slot_t slot;
    uint8_t address;
    uint8_t data[8];    // Most minibadges will have an 8-byte string of data for its "name"
    uint8_t serial[16]; // Serialized minibadges will have a 16-byte serial number
} minibadge_device_t;

extern minibadge_device_t minibadge_devices[MINIBADGE_SLOT_COUNT];

// Event types for minibadge events
typedef enum {
    MINIBADGE_EVENT_NONE,
    MINIBADGE_EVENT_INSERTED, // A minibadge was inserted
    MINIBADGE_EVENT_REMOVED,  // A minibadge was removed
    MINIBADGE_EVENT_REPLACED, // A minibadge was swapped for another since the last event
} minibadge_event_type_t;

// Event data for minibadge events
typedef struct {
    minibadge_event_type_t type;
    minibadge_slot_t slot;
} minibadge_event_t;

// Minibadge event callback
typedef void (*minibadge_event_cb_t)(minibadge_event_t e);

// Events for the D-pad directions on the D-pad minibadge
ESP_EVENT_DECLARE_BASE(MINIBADGE_DPAD_EVENT);
typedef enum {
    MINIBADGE_DPAD_EVENT_PRESS,
    MINIBADGE_DPAD_EVENT_RELEASE,
} minibadge_dpad_event_type_t;
typedef enum {
    MINIBADGE_DPAD_NONE  = 0xFF,
    MINIBADGE_DPAD_UP    = 0xFE,
    MINIBADGE_DPAD_DOWN  = 0xFB,
    MINIBADGE_DPAD_LEFT  = 0xF7,
    MINIBADGE_DPAD_RIGHT = 0xFD,
    MINIBADGE_DPAD_PRESS = 0xEF,
} minibadge_dpad_state_t;
typedef struct {
    minibadge_slot_t slot;
    minibadge_dpad_state_t state;
} minibadge_dpad_event_t;
extern esp_event_loop_handle_t minibadge_event_loop_handle;

/**
 * @brief Initialize the minibadge component
 *
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t minibadge_init();

/**
 * @brief Add a minibadge event callback
 *
 * @param callback The callback function to add
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t minibadge_add_event_callback(minibadge_event_cb_t callback);

/**
 * @brief Remove a minibadge event callback
 *
 * @param callback The callback function to remove
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t minibadge_remove_event_callback(minibadge_event_cb_t callback);

/**
 * @brief Get the number of minibadges
 *
 * @return The number of minibadges
 */
uint8_t minibadge_get_count();

/**
 * @brief Scan for minibadge devices and update the minibadge_devices array
 *
 * @return The number of minibadge devices found
 */
int8_t check_minibadges();

/**
 * @brief Start/stop polling the D-pad minibadge
 *
 * @param enable Whether to start polling the D-pad minibadge
 */
void minibadge_dpad_poll(bool enable, minibadge_slot_t slot);

#ifdef __cplusplus
}
#endif
