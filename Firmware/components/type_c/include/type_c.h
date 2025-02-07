#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_err.h"

extern EventGroupHandle_t type_c_event_group;
extern QueueHandle_t type_c_status_queue;

// Event group bits
#define TYPE_C_EVENT_ID  BIT0
#define TYPE_C_EVENT_INT BIT1

// Queue size
#define TYPE_C_QUEUE_SIZE 10

typedef struct {
    uint8_t chip_id;
    uint8_t version_id;
    uint8_t vendor_id;
} typec_device_id_t;

/**
 * Charging current status:
 *   00 = standby
 *   01 = default
 *   10 = medium (1.5A)
 *   11 = high (3A)
 */
typedef enum {
    TYPEC_CHARGING_STANDBY = 0b00,
    TYPEC_CHARGING_DEFAULT = 0b01,
    TYPEC_CHARGING_MEDIUM  = 0b10,
    TYPEC_CHARGING_HIGH    = 0b11,
} typec_charging_current_t;

/**
 * Attached port status:
 *   000 = standby
 *   001 = device
 *   010 = host
 *   011 = audio adapter accessory
 *   100 = debug accessory
 *   101 = device with active cable
 */
typedef enum {
    TYPEC_PORT_STANDBY = 0b000,
    TYPEC_PORT_DEVICE  = 0b001,
    TYPEC_PORT_HOST    = 0b010,
    TYPEC_PORT_AUDIO   = 0b011,
    TYPEC_PORT_DEBUG   = 0b100,
    TYPEC_PORT_ACTIVE  = 0b101,
} typec_port_status_t;

/**
 * Port role:
 *   00 = device
 *   01 = host
 *   10 = dual role (equal chance to connect as SRC or SNK)
 *   11 = dual role 2 (higher chance to connect as SNK with Try.SNK supported or SRC with Try.SRC supported)
 */
typedef enum {
    TYPEC_ROLE_DEVICE = 0b00,
    TYPEC_ROLE_HOST   = 0b01,
    TYPEC_ROLE_DUAL   = 0b10,
    TYPEC_ROLE_DUAL2  = 0b11,
} typec_port_role_t;

/**
 * Plug polarity:
 *   00 = standby
 *   01 = CC1 makes connection
 *   10 = CC2 makes connection
 *   11 = undetermined (e.g. audio accessory, debug accessory, etc.)
 */
typedef enum {
    TYPEC_PLUG_STANDBY = 0b00,
    TYPEC_PLUG_CC1     = 0b01,
    TYPEC_PLUG_CC2     = 0b10,
    TYPEC_PLUG_UNKNOWN = 0b11,
} typec_plug_polarity_t;

/**
 * Type-C status structure.
 */
typedef struct {
    uint8_t vbus_detected;
    typec_charging_current_t charging_current;
    typec_port_status_t port_status;
    typec_plug_polarity_t plug_polarity;
} typec_status_t;

/**
 * @brief Initialize the Type-C controller.
 *  This function will add the controller to the I2C manager device list bus as well as set up a task to handle
 *  interrupts from the controller.
 *
 * @return esp_err_t
 */
esp_err_t typec_init(void);

/**
 * @brief Read all 4 registers of the PI5USB30216C USB Type-C controller.
 *   In order to read data we have to read all 4 registers at once.
 *
 * @param data
 * @return esp_err_t
 */
esp_err_t typec_read_all(uint8_t *data);

/**
 * @brief Get the device ID of the Type-C controller.
 *   bits 7:5: Chip ID (0x01 = PI5USB30216C)
 *   bits 4:3: Version ID (0x00 = Product version)
 *   bits 2:0: Vendor ID (0x00 = Pericom)
 *
 * @param device_id
 * @return esp_err_t
 */
esp_err_t typec_read_device_id(typec_device_id_t *device_id);

/**
 * @brief Get status from the PI5USB30216C USB Type-C controller.
 *
 * @param status
 * @return esp_err_t
 */
esp_err_t typec_read_status(typec_status_t *status);

/**
 * @brief Return the current status of the Type-C controller.
 *
 * @return typec_status_t
 */
typec_status_t typec_get_status();

/**
 * @brief Enable or disable power saving mode.
 *
 * @param enable True to enable power saving mode, false to disable it.
 * @return esp_err_t
 */
esp_err_t typec_set_power_saving(bool enable);

/**
 * @brief Set the dual-role mode (Try.SRC or Try.SNK).
 *
 * @param try_snk True to enable Try.SNK, false to enable Try.SRC.
 * @return esp_err_t
 */
esp_err_t typec_set_dual_role(bool try_snk);

/**
 * @brief Enable or disable accessory detection in device mode.
 *
 * @param enable True to enable accessory detection, false to disable it.
 * @return esp_err_t
 */
esp_err_t typec_set_accessory_detection(bool enable);

/**
 * @brief Set the charging current mode.
 *
 * @param current The charging current mode.
 * @return esp_err_t
 */
esp_err_t typec_set_charging_current(typec_charging_current_t current);

/**
 * @brief Set the role of the port (device, host, dual role, etc.).
 *
 * @param role The role of the port.
 * @return esp_err_t
 */
esp_err_t typec_set_role(typec_port_role_t role);

/**
 * @brief Enable or disable interrupt masking.
 *
 * @param mask True to mask interrupts, false to enable interrupts.
 * @return esp_err_t
 */
esp_err_t typec_set_interrupt_mask(bool mask);

#ifdef __cplusplus
}
#endif
