#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "sdkconfig.h"
#include "driver/i2c_master.h"
#include "esp_check.h"
#include "esp_event.h"

#ifdef CONFIG_LCD_TOUCH_ENABLED
    #define I2C_BUS_TOUCH I2C_NUM_0
#endif
#ifdef CONFIG_I2C_PERIPHERAL_BUS_ENABLED
    #ifdef CONFIG_LCD_TOUCH_ENABLED
        #define I2C_BUS_OTHER I2C_NUM_1
    #else
        #define I2C_BUS_OTHER I2C_NUM_0
    #endif
#endif

typedef struct {
    uint8_t sda_pin;
    uint8_t scl_pin;
    bool enable_internal_pullups;
} i2c_manager_pin_config_t;

// I2C bus info structure
typedef struct {
    bool initialized;
    i2c_master_bus_config_t config;
    i2c_master_bus_handle_t handle;
    SemaphoreHandle_t lock;
} i2c_manager_bus_t;

// I2C device config tracking structure
typedef struct {
    uint8_t bus_index; // I2C bus index
#ifdef CONFIG_I2C_SWITCH_ENABLED
    uint8_t channel; // Channel on the I2C switch
#endif
    i2c_device_config_t config;
} i2c_manager_device_config_t;

typedef struct {
    int8_t bus_index; // I2C bus index. -1 for any bus
#ifdef CONFIG_I2C_SWITCH_ENABLED
    uint8_t channels; // Bit set of channels to search on the I2C switch... see I2C_SWITCH_CHANNELx
#endif
    uint8_t address; // 7-bit I2C address. 0 for any address
} i2c_manager_device_criteria_t;

// I2C master bus configurations
extern i2c_manager_bus_t i2c_bus[SOC_I2C_NUM];

// clang-format off

// Macro to handle locking a given I2C bus
#define I2C_BUS_LOCK(bus_index, err_code)                                   \
    if (xSemaphoreTake(i2c_bus[bus_index].lock, portMAX_DELAY) != pdTRUE) { \
        ESP_LOGE(TAG, "Failed to lock I2C bus %d", bus_index);              \
        return err_code;                                                    \
    }
// Macro to handle unlocking a given I2C bus
#define I2C_BUS_UNLOCK(bus_index) xSemaphoreGive(i2c_bus[bus_index].lock)

// clang-format on

// This is arbitrary, but just want to impose a cap on the number of devices to track
#define MAX_I2C_DEVICES 32

typedef struct {
    uint8_t bus_index; // I2C bus index
#ifdef CONFIG_I2C_SWITCH_ENABLED
    uint8_t channel; // Channel on the I2C switch
#endif
    uint16_t address; // 7-bit I2C address
    i2c_master_dev_handle_t handle;
} i2c_manager_known_device_t;

// Known I2C devices (added to the bus and tracked)
extern i2c_manager_known_device_t i2c_devices[MAX_I2C_DEVICES];
extern uint8_t i2c_device_count;

// Whether we have initialized the I2C manager or not
extern bool i2c_manager_initialized;

#ifdef CONFIG_I2C_SWITCH_ENABLED
// I2C interrupt event group to indicate which interrupts are active
extern EventGroupHandle_t i2c_switch_int;
extern const int I2C_SWITCH_INT0;
extern const int I2C_SWITCH_INT1;
extern const int I2C_SWITCH_INT2;
extern const int I2C_SWITCH_INT3;

    // Convenience macros for channel selection
    #define I2C_SWITCH_CHANNEL_0        0
    #define I2C_SWITCH_CHANNEL_1        1
    #define I2C_SWITCH_CHANNEL_2        2
    #define I2C_SWITCH_CHANNEL_3        3
    #define I2C_SWITCH_CHANNEL_BIT0     0b0001
    #define I2C_SWITCH_CHANNEL_BIT1     0b0010
    #define I2C_SWITCH_CHANNEL_BIT2     0b0100
    #define I2C_SWITCH_CHANNEL_BIT3     0b1000
    #define I2C_SWITCH_CHANNEL_BITS_ALL 0b1111
#endif

#ifdef CONFIG_I2C_SWITCH_ENABLED
/**
 * @brief Select an I2C switch channel.
 *
 * @param channel The channel to select (0-3)
 * @return esp_err_t
 *     - ESP_OK: I2C switch channel selected successfully
 *     - ESP_ERR_INVALID_ARG: Invalid I2C channel
 *     - ESP_FAIL: Failed to select I2C switch channel
 */
esp_err_t i2c_switch_select(uint8_t channel);

/**
 * @brief Get the current I2C switch status. This will read the control register into the status parameter.
 *
 * @param[out] status Pointer to store the status
 * @return esp_err_t
 */
esp_err_t i2c_switch_get_status(uint8_t *status);
#endif

/**
 * @brief Initialize I2C manager
 *
 * @param[in] i2c_pins I2C pin configurations
 * @param[in] i2c_bus_count I2C bus configurations to initialize (max: SOC_I2C_NUM)
 *
 * @return
 *     - ESP_OK: I2C manager initialized successfully
 *     - ESP_ERR_INVALID_ARG: Invalid I2C bus count
 *     - ESP_ERR_INVALID_STATE: I2C manager already initialized
 */
esp_err_t i2c_manager_init(i2c_manager_pin_config_t *i2c_pins, uint8_t i2c_bus_count);

/**
 * @brief Initialize I2C manager using I2C bus configurations from KConfig.
 *      This is the simplest way to initialize the I2C manager because it determines the I2C bus
 *      configurations from the selected I2C related configuration in KConfig.
 *
 * @return
 *     - ESP_OK: I2C manager initialized successfully
 *     - ESP_ERR_INVALID_ARG: Invalid I2C bus count
 *     - ESP_ERR_INVALID_STATE: I2C manager already initialized
 */
esp_err_t i2c_manager_init_auto();

/**
 * @brief Get I2C bus by index
 *
 * @param[in] bus_index The bus number to get
 * @return i2c_manager_bus_t*
 */
i2c_manager_bus_t *i2c_manager_get_bus(uint8_t bus_index);

/**
 * @brief Return whether the given I2C bus is locked
 *
 * @param[in] bus_index The bus number to check
 * @return bool True if the bus is locked, false otherwise
 */
bool i2c_manager_bus_locked(uint8_t bus_index);

/**
 * @brief See if a given device config represents a device that actually (still?) exists on the bus
 *
 * @param[in] device Device configuration
 * @return esp_err_t
 */
esp_err_t i2c_manager_ping(i2c_manager_device_config_t *device);

/**
 * @brief Scan for I2C devices on the bus
 *
 * @param[in] bus_index The bus number to scan
 * @param[out] found 7-bit addresses of found devices
 * @param[out] count Number of found devices
 *
 * @return esp_err_t
 */
esp_err_t i2c_manager_scan(uint8_t bus_index, uint8_t *found, uint8_t *count);

/**
 * @brief Discover I2C devices on the bus
 *
 * @param[in] criteria Criteria to match devices
 * @param[out] results Array of device configurations
 *
 * @return Number of devices found
 */
uint8_t i2c_manager_discover(i2c_manager_device_criteria_t criteria, i2c_manager_device_config_t **results);

/**
 * @brief Add a known device to the list of devices to keep track of
 *    This function will add the device to the list of known devices and add it to the I2C bus.
 *
 * @param[in] device Device configuration
 * @param[out] handle Device handle
 *
 * @return esp_err_t
 */
esp_err_t i2c_manager_add_device(i2c_manager_device_config_t *device, i2c_master_dev_handle_t *handle);

/**
 * @brief Remove a known device from the list of devices to keep track of
 *    This function will remove the device from the list of known devices and remove it from the I2C bus.
 *
 * @param[in] device Partial or full known device structure
 *
 * @return esp_err_t
 */
esp_err_t i2c_manager_remove_device(i2c_manager_known_device_t *device);

/**
 * @brief Find a known device
 *   This function will search the list of known devices for a device matching the given criteria.
 *
 * @param[in] device Partial or full known device structure
 * @param[out] handle Device handle
 *
 * @return esp_err_t
 */
esp_err_t i2c_manager_find_device(i2c_manager_known_device_t *device, i2c_master_dev_handle_t *handle);

/**
 * @brief Helper to transmit data to an I2C device using an i2c_manager_device_config_t structure
 *     This function will lock the I2C bus, switch the mux if necessary, transmit the data, and unlock the bus.
 *
 * @param[in] device Device configuration
 * @param[in] data Data buffer to transmit
 * @param[in] size Number of bytes to transmit
 * @param[in] timeout_ms Timeout in milliseconds
 *
 * @return esp_err_t
 */
esp_err_t i2c_manager_transmit(i2c_manager_device_config_t *device, uint8_t *data, size_t size, TickType_t timeout_ms);

/**
 * @brief Helper to receive data from an I2C device using an i2c_manager_device_config_t structure
 *     This function will lock the I2C bus, switch the mux if necessary, receive the data, and unlock the bus.
 *
 * @param[in] device Device configuration
 * @param[out] data Data buffer to receive into
 * @param[in] size Number of bytes to receive
 * @param[in] timeout_ms Timeout in milliseconds
 *
 * @return esp_err_t
 */
esp_err_t i2c_manager_receive(i2c_manager_device_config_t *device, uint8_t *data, size_t size, TickType_t timeout_ms);

/**
 * @brief Helper to transmit and receive data from an I2C device using an i2c_manager_device_config_t structure
 *    This function will lock the I2C bus, switch the mux if necessary, transmit/receive the data, and unlock the bus.
 *
 * @param[in] device Device configuration
 * @param[in] tx_data Data buffer to transmit
 * @param[in] tx_size Number of bytes to transmit
 * @param[out] rx_data Data buffer to receive into
 * @param[in] rx_size Number of bytes to receive
 * @param[in] timeout_ms Timeout in milliseconds
 *
 * @return esp_err_t
 */
esp_err_t i2c_manager_transmit_receive(i2c_manager_device_config_t *device, uint8_t *tx_data, size_t tx_size, uint8_t *rx_data,
                                       size_t rx_size, TickType_t timeout_ms);

/**
 * @brief Read data from a given EEPROM device
 *
 * @param[in] device Device configuration
 * @param[in] address EEPROM address to read from
 * @param[out] data Data buffer to read into
 * @param[in] size Number of bytes to read
 *
 * @return esp_err_t
 *     - ESP_OK: Data read successfully
 *     - ESP_ERR_INVALID_ARG: Invalid device configuration
 *     - ESP_ERR_INVALID_SIZE: Invalid data length
 *     - ESP_FAIL: Failed to read data
 */
esp_err_t i2c_manager_read_eeprom(i2c_manager_device_config_t *device, uint32_t address, uint8_t *data, size_t size);

#ifdef __cplusplus
}
#endif
