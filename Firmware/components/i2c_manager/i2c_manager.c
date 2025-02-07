#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "i2c_manager.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_log.h"

static const char *TAG = "i2c_manager";

bool i2c_manager_initialized           = false;
i2c_manager_bus_t i2c_bus[SOC_I2C_NUM] = {0};
i2c_manager_known_device_t i2c_devices[MAX_I2C_DEVICES];
uint8_t i2c_device_count = 0;

// Use 100 kHz as the default I2C bus speed
#define I2C_DEFAULT_BUS_SPEED 100000

#ifdef CONFIG_I2C_SWITCH_ENABLED
    // I2C switch configuration for the TCA9544A
    #define TCA9544A_FIXED_ADDR            0b1110000  // Fixed 7-bit address of the TCA9544A I2C switch
    #define TCA9544A_SELECT_ADDR           0b000      // Hardware selectable address portion (A2, A1, A0 pins)
    #define TCA9544A_CTRL_REG_ENABLE       0b00000100 // Enable bit in the control register
    #define TCA9544A_CTRL_REG_CHANNEL_BITS 0b00000011 // Bits for selecting the channel

    #define I2C_SWITCH_ADDR     (TCA9544A_FIXED_ADDR | TCA9544A_SELECT_ADDR) // Full 7-bit address of the I2C switch
    #define I2C_SWITCH_CHANNELS 4                                            // The TCA9548A has 4 channels
    #define I2C_SWITCH_FREQ     100000                                       // I2C frequency to use for the switch

static i2c_master_dev_handle_t i2c_switch     = NULL; // Keep a handle to the I2C switch
static SemaphoreHandle_t i2c_switch_interrupt = NULL; // Semaphore for the I2C switch task
static SemaphoreHandle_t i2c_switch_lock      = NULL; // Mutex for channel selection
static uint8_t i2c_switch_channel             = 0;    // Track the currently selected channel
EventGroupHandle_t i2c_switch_int             = NULL; // Event group for I2C switch interrupts
const int I2C_SWITCH_INT0                     = BIT0;
const int I2C_SWITCH_INT1                     = BIT1;
const int I2C_SWITCH_INT2                     = BIT2;
const int I2C_SWITCH_INT3                     = BIT3;

/**
 * @brief I2C switch interrupt service routine.
 */
static void IRAM_ATTR i2c_switch_isr_handler(void *_arg) {
    (void)_arg;
    BaseType_t task_woken = pdFALSE;
    xSemaphoreGiveFromISR(i2c_switch_interrupt, &task_woken);
    portYIELD_FROM_ISR(task_woken);
}

/**
 * @brief I2C switch task to handle switch interrupts.
 */
void i2c_switch_int_task(void *_arg) {
    (void)_arg;
    while (1) {
        ESP_LOGD(TAG, "Waiting for I2C switch interrupt...");
        if (xSemaphoreTake(i2c_switch_interrupt, portMAX_DELAY) == pdTRUE) {
            // Read the I2C switch control register
            uint8_t control = 0;
            I2C_BUS_LOCK(I2C_BUS_OTHER, )
            if (i2c_master_receive(i2c_switch, &control, 1, portMAX_DELAY) != ESP_OK) {
                ESP_LOGE(TAG, "Failed to read I2C switch control register");
                I2C_BUS_UNLOCK(I2C_BUS_OTHER);
                continue;
            }
            I2C_BUS_UNLOCK(I2C_BUS_OTHER);

            ESP_LOGD(TAG, "I2C switch interrupt: 0x%02X", control >> 4);

            // Set corresponding event group bits for active interrupts
            xEventGroupClearBits(i2c_switch_int, 0xF);
            xEventGroupSetBits(i2c_switch_int,
                               control >> 4 // INT3, INT2, INT1, INT0 bits are in the upper nibble so shift them down to set the
                                            // corresponding event group bits
            );
        }
    }
}

/**
 * @brief Register the I2C switch interrupt service routine.
 */
esp_err_t i2c_switch_isr_init() {
    // Only register the ISR if the I2C switch interrupt GPIO is not -1
    if (CONFIG_I2C_SWITCH_INT_GPIO == -1) {
        ESP_LOGI(TAG, "I2C switch interrupt GPIO not defined... skipping ISR init");
        return ESP_OK;
    }

    // Configure the GPIO pin for the I2C switch interrupt
    esp_err_t err         = ESP_OK;
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << CONFIG_I2C_SWITCH_INT_GPIO,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .intr_type    = GPIO_INTR_NEGEDGE,
    };
    ESP_LOGI(TAG, "Configuring I2C switch interrupt GPIO %d", CONFIG_I2C_SWITCH_INT_GPIO);
    ESP_RETURN_ON_ERROR(gpio_config(&io_conf), TAG, "Failed to configure I2C switch interrupt GPIO");

    // Create the I2C switch interrupt semaphore
    if (i2c_switch_interrupt == NULL) {
        if ((i2c_switch_interrupt = xSemaphoreCreateBinary()) == NULL) {
            ESP_LOGE(TAG, "Failed to create I2C switch interrupt semaphore");
            return ESP_FAIL;
        }
    }

    // Register the ISR handler
    ESP_RETURN_ON_ERROR(gpio_isr_handler_add(CONFIG_I2C_SWITCH_INT_GPIO, i2c_switch_isr_handler, NULL), TAG,
                        "Failed to add I2C switch ISR handler: %s", esp_err_to_name(err));

    // Start the I2C switch task to handle switch interrupts
    xTaskCreate(i2c_switch_int_task, "i2c_switch_int_task", 4096, NULL, 10, NULL);

    return err;
}

/**
 * @brief Initialize the I2C switch device.
 *
 * @note We assume the switch is connected to I2C_BUS_OTHER.
 * @return esp_err_t
 *     - ESP_OK: I2C switch initialized successfully
 *     - ESP_ERR_INVALID_ARG: I2C bus not initialized
 *     - ESP_FAIL: Failed to add I2C switch device to peripheral bus
 */
esp_err_t i2c_switch_init() {
    if (!i2c_bus[I2C_BUS_OTHER].initialized) {
        ESP_LOGE(TAG, "I2C bus %d not initialized", I2C_BUS_OTHER);
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = ESP_OK;

    // Add the I2C switch device to the bus
    I2C_BUS_LOCK(I2C_BUS_OTHER, ESP_FAIL)
    if (i2c_switch == NULL) {
        i2c_device_config_t dev_config = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address  = I2C_SWITCH_ADDR,
            .scl_speed_hz    = I2C_SWITCH_FREQ,
        };
        ESP_GOTO_ON_ERROR(i2c_master_bus_add_device(i2c_bus[I2C_BUS_OTHER].handle, &dev_config, &i2c_switch), cleanup, TAG,
                          "Failed to add I2C switch device 0x%X to bus %d", I2C_SWITCH_ADDR, I2C_BUS_OTHER);
    }

    // Create the I2C switch lock
    i2c_switch_lock = xSemaphoreCreateMutex();

    // Read the control register to get the current interrupt and channel status
    if (xSemaphoreTake(i2c_switch_lock, portMAX_DELAY) != pdTRUE) {
        ret = ESP_FAIL;
        goto cleanup;
    }
    uint8_t control = 0;
    ESP_GOTO_ON_ERROR(i2c_master_receive(i2c_switch, &control, 1, portMAX_DELAY), cleanup, TAG,
                      "Failed to read I2C switch control register");
    i2c_switch_channel = control & TCA9544A_CTRL_REG_CHANNEL_BITS;
    ESP_LOGD(TAG, "I2C switch control register: 0x%02X", control);

cleanup:
    if (i2c_switch_lock != NULL) {
        xSemaphoreGive(i2c_switch_lock);
    }
    I2C_BUS_UNLOCK(I2C_BUS_OTHER);
    if (ret != ESP_OK) {
        return ret;
    }

    // Initialize the I2C switch interrupt
    return i2c_switch_isr_init();
}

esp_err_t i2c_switch_select(uint8_t channel) {
    if (!i2c_bus[I2C_BUS_OTHER].initialized) {
        ESP_LOGE(TAG, "I2C bus %d not initialized", I2C_BUS_OTHER);
        return ESP_ERR_INVALID_ARG;
    }
    if (channel >= I2C_SWITCH_CHANNELS) {
        ESP_LOGE(TAG, "Invalid I2C channel %d", channel);
        return ESP_ERR_INVALID_ARG;
    }
    if (!i2c_manager_bus_locked(I2C_BUS_OTHER)) {
        ESP_LOGW(TAG, "I2C bus %d should be locked before selecting I2C switch channel", I2C_BUS_OTHER);
    }

    // Obtain the I2C switch lock
    if (xSemaphoreTake(i2c_switch_lock, portMAX_DELAY) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to take I2C switch lock");
        return ESP_FAIL;
    }

    // Only switch channels if the current channel is different
    if (i2c_switch_channel == channel) {
        xSemaphoreGive(i2c_switch_lock);
        return ESP_OK;
    }

    // The TCA9544A control register format is as follows:
    //   | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
    //   | I | I | I | I | - | E | C | C |
    //
    //   I: Interrupt bits (INT3, INT2, INT1, INT0)
    //   E: Enable bit
    //   C: Channel select bits
    uint8_t command = TCA9544A_CTRL_REG_ENABLE | (channel & TCA9544A_CTRL_REG_CHANNEL_BITS);
    if (i2c_master_transmit(i2c_switch, &command, 1, portMAX_DELAY) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to select I2C switch channel %d", channel);
        return ESP_FAIL;
    }

    // Now verify that the channel actually changed
    uint8_t control = 0;
    if (i2c_master_receive(i2c_switch, &control, 1, portMAX_DELAY) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read I2C switch control register");
        xSemaphoreGive(i2c_switch_lock);
        return ESP_FAIL;
    }
    if ((control & TCA9544A_CTRL_REG_CHANNEL_BITS) != channel) {
        ESP_LOGE(TAG, "I2C switch channel failed to set to %d ... it is set to %d", channel,
                 control & TCA9544A_CTRL_REG_CHANNEL_BITS);
        xSemaphoreGive(i2c_switch_lock);
        return ESP_FAIL;
    }

    // Update the selected channel
    i2c_switch_channel = channel;
    xSemaphoreGive(i2c_switch_lock);

    return ESP_OK;
}

esp_err_t i2c_switch_get_status(uint8_t *status) {
    esp_err_t ret = ESP_OK;

    if (!i2c_bus[I2C_BUS_OTHER].initialized) {
        ESP_LOGE(TAG, "I2C bus %d not initialized", I2C_BUS_OTHER);
        return ESP_ERR_INVALID_ARG;
    }

    // Read the I2C switch control register
    I2C_BUS_LOCK(I2C_BUS_OTHER, ESP_FAIL)
    ret = i2c_master_receive(i2c_switch, status, 1, portMAX_DELAY);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read I2C switch control register");
    }
    I2C_BUS_UNLOCK(I2C_BUS_OTHER);

    return ret;
}
#endif // CONFIG_I2C_SWITCH_ENABLED

/**
 * @brief Check the given I2C bus for validity.
 *
 * @param bus_index The I2C bus index to check
 * @return esp_err_t
 *     - ESP_OK: I2C bus is valid
 *     - ESP_ERR_INVALID_ARG: I2C bus is invalid
 *     - ESP_ERR_INVALID_STATE: I2C manager not initialized
 */
esp_err_t check_bus(uint8_t bus_index) {
    if (!i2c_manager_initialized) {
        ESP_LOGE(TAG, "I2C manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    if (bus_index >= SOC_I2C_NUM) {
        ESP_LOGE(TAG, "Invalid I2C bus index %d", bus_index);
        return ESP_ERR_INVALID_ARG;
    }
    if (!i2c_bus[bus_index].initialized) {
        ESP_LOGE(TAG, "I2C bus %d not initialized", bus_index);
        return ESP_ERR_INVALID_ARG;
    }
    return ESP_OK;
}

/**
 * @brief Check the I2C device configuration for validity.
 *
 * @param device The I2C device configuration to check
 * @return esp_err_t
 *     - ESP_OK: Device configuration is valid
 *     - ESP_ERR_INVALID_ARG: Device configuration is invalid
 *     - ESP_FAIL: Device configuration is invalid
 */
esp_err_t check_device_config(i2c_manager_device_config_t *device) {
    if (device == NULL) {
        ESP_LOGE(TAG, "Invalid I2C device");
        return ESP_ERR_INVALID_ARG;
    }
    ESP_RETURN_ON_ERROR(check_bus(device->bus_index), TAG, "I2C bus validation error for bus %d", device->bus_index);
#ifdef CONFIG_I2C_SWITCH_ENABLED
    if (device->channel >= I2C_SWITCH_CHANNELS) {
        ESP_LOGE(TAG, "Invalid I2C switch channel %d", device->channel);
        return ESP_ERR_INVALID_ARG;
    }
#endif
    if (device->config.device_address == 0) {
        ESP_LOGE(TAG, "Invalid I2C device address 0x%X", device->config.device_address);
        return ESP_ERR_INVALID_ARG;
    }
    return ESP_OK;
}

esp_err_t i2c_manager_init(i2c_manager_pin_config_t *i2c_pins, uint8_t i2c_bus_count) {
    if (i2c_bus_count > SOC_I2C_NUM) {
        ESP_LOGE(TAG, "Invalid I2C bus count %d", i2c_bus_count);
        return ESP_ERR_INVALID_ARG;
    }
    if (i2c_manager_initialized) {
        ESP_LOGE(TAG, "I2C buses already initialized");
        return ESP_ERR_INVALID_STATE;
    }
    for (uint8_t i = 0; i < i2c_bus_count; i++) {
        i2c_master_bus_config_t config = {
            .i2c_port                     = i,
            .sda_io_num                   = i2c_pins[i].sda_pin,
            .scl_io_num                   = i2c_pins[i].scl_pin,
            .clk_source                   = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt            = 10,
            .flags.enable_internal_pullup = i2c_pins[i].enable_internal_pullups,
        };
        i2c_master_bus_handle_t handle;
        ESP_ERROR_CHECK(i2c_new_master_bus(&config, &handle));
        if (i2c_bus[i].lock == NULL) {
            i2c_bus[i].lock = xSemaphoreCreateMutex();
            if (i2c_bus[i].lock == NULL) {
                ESP_LOGE(TAG, "Failed to create I2C bus %d lock", i);
                return ESP_FAIL;
            }
        }
        i2c_bus[i].initialized = true;
        i2c_bus[i].config      = config;
        i2c_bus[i].handle      = handle;
    }
#ifdef CONFIG_I2C_SWITCH_ENABLED
    i2c_switch_int = xEventGroupCreate();
    ESP_ERROR_CHECK(i2c_switch_init());
#endif
    i2c_manager_initialized = true;
    return ESP_OK;
}

esp_err_t i2c_manager_init_auto() {
    if (i2c_manager_initialized) {
        ESP_LOGE(TAG, "I2C buses already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Make an array of I2C pin configurations
    i2c_manager_pin_config_t i2c_configs[SOC_I2C_NUM];
    uint8_t config_count = 0;
    // clang-format off
    #ifdef CONFIG_LCD_TOUCH_ENABLED
    i2c_configs[config_count++] = (i2c_manager_pin_config_t){
        .sda_pin                 = CONFIG_LCD_TOUCH_SDA_GPIO,
        .scl_pin                 = CONFIG_LCD_TOUCH_SCL_GPIO,
        .enable_internal_pullups = true
    };
    #endif
    #ifdef CONFIG_I2C_PERIPHERAL_BUS_ENABLED
    i2c_configs[config_count++] = (i2c_manager_pin_config_t){
        .sda_pin                 = CONFIG_I2C_PERIPHERAL_SDA_GPIO,
        .scl_pin                 = CONFIG_I2C_PERIPHERAL_SCL_GPIO,
        .enable_internal_pullups = true
    };
    #endif
    // clang-format on

    return i2c_manager_init(&i2c_configs[0], config_count);
}

i2c_manager_bus_t *i2c_manager_get_bus(uint8_t bus_index) {
    if (check_bus(bus_index) != ESP_OK) {
        return NULL;
    }
    return &i2c_bus[bus_index];
}

bool i2c_manager_bus_locked(uint8_t bus_index) {
    if (check_bus(bus_index) != ESP_OK) {
        return false;
    }
    if (xSemaphoreTake(i2c_bus[bus_index].lock, 0) == pdTRUE) {
        xSemaphoreGive(i2c_bus[bus_index].lock);
        return false;
    } else {
        return true;
    }
}

esp_err_t i2c_manager_ping(i2c_manager_device_config_t *device) {
    ESP_RETURN_ON_ERROR(check_device_config(device), TAG, "Invalid I2C device configuration");

    // Ping the device to see if it's there
    I2C_BUS_LOCK(device->bus_index, ESP_FAIL)
#ifdef CONFIG_I2C_SWITCH_ENABLED
    i2c_switch_select(device->channel);
#endif
    esp_err_t ret = i2c_master_probe(i2c_bus[device->bus_index].handle, device->config.device_address, 20);
    I2C_BUS_UNLOCK(device->bus_index);

    return ret;
}

esp_err_t i2c_manager_scan(uint8_t bus_index, uint8_t *found, uint8_t *count) {
    ESP_RETURN_ON_ERROR(check_bus(bus_index), TAG, "I2C bus validation error for bus %d", bus_index);
    if (!i2c_manager_bus_locked(bus_index)) {
        ESP_LOGW(TAG, "I2C bus %d should be locked before scanning", bus_index);
    }

    ESP_LOGD(TAG, "Scanning I2C bus %d", bus_index);
    for (uint8_t addr = 1; addr < 127; addr++) {
        if (i2c_master_probe(i2c_bus[bus_index].handle, addr, 20) == ESP_OK) {
            ESP_LOGD(TAG, "I2C device found at address 0x%X", addr);
            found[(*count)++] = addr;
        }
    }

    return ESP_OK;
}

uint8_t i2c_manager_discover(i2c_manager_device_criteria_t criteria, i2c_manager_device_config_t **results) {
    uint8_t device_count = 0;

    // Allocate memory for the results array
    if (*results == NULL) {
        *results = malloc(MAX_I2C_DEVICES * sizeof(i2c_manager_device_config_t));
    }

    if (criteria.bus_index == -1) {
        // Scan all buses
        for (uint8_t i = 0; i < SOC_I2C_NUM; i++) {
            if (i2c_bus[i].initialized) {
                device_count += i2c_manager_discover(
                    (i2c_manager_device_criteria_t){
                        .bus_index = i,
#ifdef CONFIG_I2C_SWITCH_ENABLED
                        .channels = criteria.channels,
#endif
                        .address = criteria.address,
                    },
                    results);
            }
        }
    } else {
        // Scan the specified bus
        if (i2c_bus[criteria.bus_index].initialized) {
            I2C_BUS_LOCK(criteria.bus_index, 0)

// Select the I2C switch channel if needed
#ifdef CONFIG_I2C_SWITCH_ENABLED
            uint8_t channels = criteria.channels;

            char bin_str[9] = {0};
            for (int i = 7; i >= 0; i--) {
                bin_str[7 - i] = (channels & (1 << i)) ? '1' : '0';
            }

            ESP_LOGD(TAG, "Scanning I2C bus %d on channels 0b%s", criteria.bus_index, bin_str);

            for (uint8_t chan = 0; chan < I2C_SWITCH_CHANNELS; chan++) {
                if (channels & (1 << chan)) {
                    i2c_switch_select(chan);
#endif

                    // Scan the bus for devices
                    uint8_t found[128] = {0};
                    uint8_t count      = 0;
                    i2c_manager_scan(criteria.bus_index, found, &count);

                    // Add the devices to the results array
                    for (uint8_t i = 0; i < count; i++) {
                        if (criteria.address == 0 || criteria.address == found[i]) {
                            // Skip the I2C switch device address
                            if (found[i] == I2C_SWITCH_ADDR) {
                                continue;
                            }

                            // Add the device to the results array
                            (*results)[device_count++] = (i2c_manager_device_config_t){
                                .bus_index = criteria.bus_index,
#ifdef CONFIG_I2C_SWITCH_ENABLED
                                .channel = chan,
#endif
                                .config =
                                    {
                                        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
                                        .device_address  = found[i],
                                        .scl_speed_hz    = I2C_DEFAULT_BUS_SPEED,
                                    },
                            };
                        }
                    }
#ifdef CONFIG_I2C_SWITCH_ENABLED
                }
            }
#endif

            I2C_BUS_UNLOCK(criteria.bus_index);
        }
    }

    return device_count;
}

esp_err_t i2c_manager_add_device(i2c_manager_device_config_t *device, i2c_master_dev_handle_t *handle) {
    ESP_RETURN_ON_ERROR(check_device_config(device), TAG, "Invalid I2C device configuration");

    // Make sure we don't already have this device added
    for (uint8_t i = 0; i < i2c_device_count; i++) {
        if (i2c_devices[i].bus_index == device->bus_index && i2c_devices[i].address == device->config.device_address) {
#ifdef CONFIG_I2C_SWITCH_ENABLED
            if (i2c_devices[i].channel == device->channel) {
#endif
                ESP_LOGE(TAG, "I2C device already added to bus %d", device->bus_index);
                return ESP_ERR_INVALID_STATE;
#ifdef CONFIG_I2C_SWITCH_ENABLED
            }
#endif
        }
    }

    // Add the device to the bus to get a handle
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(i2c_bus[device->bus_index].handle, &device->config, handle), TAG,
                        "Failed to add I2C device to bus %d", device->bus_index);

    // Add the device to the list of known devices we're tracking
    i2c_manager_known_device_t known_device = {
        .bus_index = device->bus_index,
#ifdef CONFIG_I2C_SWITCH_ENABLED
        .channel = device->channel,
#endif
        .address = device->config.device_address,
        .handle  = *handle,
    };
    i2c_devices[i2c_device_count++] = known_device;

    return ESP_OK;
}

esp_err_t i2c_manager_remove_device(i2c_manager_known_device_t *device) {
    for (uint8_t i = 0; i < i2c_device_count; i++) {
        if (i2c_devices[i].bus_index == device->bus_index && i2c_devices[i].address == device->address) {
#ifdef CONFIG_I2C_SWITCH_ENABLED
            if (i2c_devices[i].channel == device->channel) {
#endif
                i2c_master_bus_rm_device(i2c_devices[i].handle);
                i2c_device_count--;
                for (uint8_t j = i; j < i2c_device_count; j++) {
                    i2c_devices[j] = i2c_devices[j + 1];
                }
                return ESP_OK;
#ifdef CONFIG_I2C_SWITCH_ENABLED
            }
#endif
        }
    }

    return ESP_ERR_NOT_FOUND;
}

esp_err_t i2c_manager_find_device(i2c_manager_known_device_t *device, i2c_master_dev_handle_t *handle) {
    for (uint8_t i = 0; i < i2c_device_count; i++) {
        if (i2c_devices[i].bus_index == device->bus_index && i2c_devices[i].address == device->address) {
#ifdef CONFIG_I2C_SWITCH_ENABLED
            if (i2c_devices[i].channel == device->channel) {
#endif
                *handle = i2c_devices[i].handle;
                return ESP_OK;
#ifdef CONFIG_I2C_SWITCH_ENABLED
            }
#endif
        }
    }

    return ESP_ERR_NOT_FOUND;
}

esp_err_t i2c_manager_get_device(i2c_manager_device_config_t *device, i2c_master_dev_handle_t *handle, bool *added) {
    ESP_RETURN_ON_ERROR(check_device_config(device), TAG, "Invalid I2C device configuration");
    i2c_manager_known_device_t known_device = {
        .bus_index = device->bus_index,
#ifdef CONFIG_I2C_SWITCH_ENABLED
        .channel = device->channel,
#endif
        .address = device->config.device_address,
    };
    if (i2c_manager_find_device(&known_device, handle) != ESP_OK) {
        ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(i2c_bus[device->bus_index].handle, &device->config, handle), TAG,
                            "Failed to add I2C device to bus");
    }
    return ESP_OK;
}

esp_err_t i2c_manager_transmit(i2c_manager_device_config_t *device, uint8_t *data, size_t size, TickType_t timeout_ms) {
    esp_err_t ret                      = ESP_OK;
    i2c_master_dev_handle_t dev_handle = NULL;
    bool transient                     = false;

    // Get the device handle
    ESP_RETURN_ON_ERROR(i2c_manager_get_device(device, &dev_handle, &transient), TAG, "Failed to get I2C device");
    I2C_BUS_LOCK(device->bus_index, ESP_FAIL)

// Select the I2C switch channel if needed
#ifdef CONFIG_I2C_SWITCH_ENABLED
    i2c_switch_select(device->channel);
#endif

    // Transmit the data
    if (i2c_master_transmit(dev_handle, data, size, timeout_ms) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to transmit I2C data: %s", esp_err_to_name(ret));
        ret = ESP_FAIL;
    }

    // Cleanup
    I2C_BUS_UNLOCK(device->bus_index);
    if (transient) {
        i2c_master_bus_rm_device(dev_handle);
    }

    return ret;
}

esp_err_t i2c_manager_receive(i2c_manager_device_config_t *device, uint8_t *data, size_t size, TickType_t timeout_ms) {
    esp_err_t ret                      = ESP_OK;
    i2c_master_dev_handle_t dev_handle = NULL;
    bool transient                     = false;

    // Get the device handle
    ESP_RETURN_ON_ERROR(i2c_manager_get_device(device, &dev_handle, &transient), TAG, "Failed to get I2C device");
    I2C_BUS_LOCK(device->bus_index, ESP_FAIL)

// Select the I2C switch channel if needed
#ifdef CONFIG_I2C_SWITCH_ENABLED
    i2c_switch_select(device->channel);
#endif

    // Receive the data
    if (i2c_master_receive(dev_handle, data, size, timeout_ms) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to transmit I2C data: %s", esp_err_to_name(ret));
        ret = ESP_FAIL;
    }

    // Cleanup
    I2C_BUS_UNLOCK(device->bus_index);
    if (transient) {
        i2c_master_bus_rm_device(dev_handle);
    }

    return ret;
}

esp_err_t i2c_manager_transmit_receive(i2c_manager_device_config_t *device, uint8_t *tx_data, size_t tx_size, uint8_t *rx_data,
                                       size_t rx_size, TickType_t timeout_ms) {
    esp_err_t ret                      = ESP_OK;
    i2c_master_dev_handle_t dev_handle = NULL;
    bool transient                     = false;

    // Get the device handle
    ESP_RETURN_ON_ERROR(i2c_manager_get_device(device, &dev_handle, &transient), TAG, "Failed to get I2C device");
    I2C_BUS_LOCK(device->bus_index, ESP_FAIL)

// Select the I2C switch channel if needed
#ifdef CONFIG_I2C_SWITCH_ENABLED
    i2c_switch_select(device->channel);
#endif

    // Transmit and receive the data
    if (i2c_master_transmit_receive(dev_handle, tx_data, tx_size, rx_data, rx_size, timeout_ms) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to transmit/receive I2C data: %s", esp_err_to_name(ret));
        ret = ESP_FAIL;
    }

    // Cleanup
    I2C_BUS_UNLOCK(device->bus_index);
    if (transient) {
        i2c_master_bus_rm_device(dev_handle);
    }

    return ret;
}

esp_err_t i2c_manager_read_eeprom(i2c_manager_device_config_t *device, uint32_t address, uint8_t *data, size_t size) {
    // Write a single byte to the EEPROM specifying the address to read from and then read the data
    uint8_t addr_buffer = address & 0xFF;
    ESP_RETURN_ON_ERROR(i2c_manager_transmit_receive(device, &addr_buffer, sizeof(addr_buffer), data, size, 100), TAG,
                        "Failed to read EEPROM data");

    return ESP_OK;
}
