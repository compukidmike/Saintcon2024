#include <stdio.h>
#include "type_c.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "i2c_manager.h"

static const char *TAG = "type_c";

// I²C address of the PI5USB30216C USB Type-C controller
#define PI5USB30216C_I2C_ADDR 0b0011101

// I²C switch channel and interrupt pin used
#define I2C_SWITCH_INTERRUPT I2C_SWITCH_INT1

// Registers of the PI5USB30216C USB Type-C controller
#define REG_DEVICE_ID 0x01 - 1
#define REG_CONTROL   0x02 - 1
#define REG_INTERRUPT 0x03 - 1
#define REG_STATUS    0x04 - 1

// Device configuration
i2c_manager_device_config_t typec_device = {
    .bus_index = I2C_BUS_OTHER,
    .channel   = I2C_SWITCH_CHANNEL_3,
    .config =
        {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address  = PI5USB30216C_I2C_ADDR,
            .scl_speed_hz    = 400000,
        },
};

// Semaphore for handling Type-C ID interrupts
static SemaphoreHandle_t typec_id_interrupt = NULL;

// Event group and queue for Type-C events and status
EventGroupHandle_t type_c_event_group = NULL;
QueueHandle_t type_c_status_queue     = NULL;

// Current status
static typec_status_t current_status;

/**
 * @brief ISR handler for the Type-C ID GPIO input
 */
static void IRAM_ATTR typec_id_isr_handler(void *_arg) {
    (void)_arg; // Unused
    BaseType_t task_woken = pdFALSE;
    xSemaphoreGiveFromISR(typec_id_interrupt, &task_woken);
    portYIELD_FROM_ISR(task_woken);
}

/**
 * @brief Task to handle interrupts from the Type-C controller's ID output
 */
static void typec_id_task(void *_arg) {
    (void)_arg; // Unused
    while (1) {
        if (xSemaphoreTake(typec_id_interrupt, portMAX_DELAY) == pdTRUE) {
            // Read and send status
            typec_read_status(&current_status);
            xQueueSend(type_c_status_queue, &current_status, portMAX_DELAY);

            // Notify other components
            xEventGroupSetBits(type_c_event_group, TYPE_C_EVENT_ID);
        }
    }
}

/**
 * @brief Task to handle interrupts from the Type-C controller
 */
static void typec_interrupt_task(void *_arg) {
    (void)_arg; // Unused
    while (1) {
        if (xEventGroupWaitBits(i2c_switch_int, I2C_SWITCH_INTERRUPT, pdTRUE, pdTRUE, portMAX_DELAY) & I2C_SWITCH_INTERRUPT) {
            // Read and send status
            typec_read_status(&current_status);
            xQueueSend(type_c_status_queue, &current_status, portMAX_DELAY);

            // Notify other components
            xEventGroupSetBits(type_c_event_group, TYPE_C_EVENT_INT);
        }
    }
}

esp_err_t typec_init(void) {
    // Add the controller to the I²C manager device list bus
    i2c_master_dev_handle_t dev_handle = NULL;
    ESP_RETURN_ON_ERROR(i2c_manager_add_device(&typec_device, &dev_handle), TAG, "Failed to add I2C device to bus");

    // Configure the ENB (enable) GPIO for output
    gpio_config_t enb_conf = {
        .pin_bit_mask = 1ULL << CONFIG_TYPEC_ENB_GPIO,
        .mode         = GPIO_MODE_OUTPUT,
    };
    gpio_config(&enb_conf);

    // Set/keep ENB low to enable the Type-C controller
    gpio_set_level(CONFIG_TYPEC_ENB_GPIO, 0);

    // Configure the ID GPIO as input for detecting the Type-C plug
    gpio_config_t id_conf = {
        .pin_bit_mask = 1ULL << CONFIG_TYPEC_ID_GPIO,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .intr_type    = GPIO_INTR_ANYEDGE,
    };
    gpio_config(&id_conf);

    // Create the semaphore for handling Type-C ID interrupts
    typec_id_interrupt = xSemaphoreCreateBinary();

    // Create the event group and queue for Type-C events and status
    type_c_event_group  = xEventGroupCreate();
    type_c_status_queue = xQueueCreate(TYPE_C_QUEUE_SIZE, sizeof(typec_status_t));

    // Register the ISR handler
    gpio_isr_handler_add(CONFIG_TYPEC_ID_GPIO, typec_id_isr_handler, NULL);

    // Create tasks to handle interrupts from the controller and the ID output
    xTaskCreate(typec_interrupt_task, "typec_interrupt_task", 4096, NULL, 10, NULL);
    xTaskCreate(typec_id_task, "typec_id_task", 4096, NULL, 10, NULL);

    return ESP_OK;
}

esp_err_t typec_read_all(uint8_t *data) {
    ESP_RETURN_ON_ERROR(i2c_manager_receive(&typec_device, data, 4, 100), TAG, "Failed to read all registers");
    return ESP_OK;
}

esp_err_t typec_read_device_id(typec_device_id_t *device_id) {
    esp_err_t ret;
    uint8_t data[4] = {0};

    // Read all 4 registers
    ret = typec_read_all(data);
    if (ret != ESP_OK) {
        return ret;
    }

    ESP_LOG_BUFFER_HEX(TAG, data, 4);

    // Save the device ID, version ID, and vendor ID
    device_id->chip_id    = data[REG_DEVICE_ID] >> 5;
    device_id->version_id = (data[REG_DEVICE_ID] >> 3) & 0x03;
    device_id->vendor_id  = data[REG_DEVICE_ID] & 0x07;

    return ESP_OK;
}

esp_err_t typec_read_status(typec_status_t *status) {
    esp_err_t ret;
    uint8_t data[4] = {0};

    // Read all 4 registers
    ret = typec_read_all(data);
    if (ret != ESP_OK) {
        return ret;
    }

    // Save the status
    status->vbus_detected    = (data[REG_STATUS] >> 7) & 0x01;
    status->charging_current = (data[REG_STATUS] >> 5) & 0x03;
    status->port_status      = (data[REG_STATUS] >> 2) & 0x07;
    status->plug_polarity    = (data[REG_STATUS] >> 0) & 0x03;

    return ESP_OK;
}

typec_status_t typec_get_status() {
    return current_status;
}

static esp_err_t typec_write_control_register(uint8_t control_value) {
    esp_err_t ret;

    // Read the current values of all four registers
    uint8_t data[4] = {0};
    ret             = typec_read_all(data);
    if (ret != ESP_OK) {
        return ret;
    }

    // Update the control register value
    data[REG_CONTROL] = control_value;

    // Write the updated control register value by writing all four registers
    ESP_RETURN_ON_ERROR(i2c_manager_transmit(&typec_device, data, 4, 100), TAG, "Failed to write control register");

    return ESP_OK;
}

esp_err_t typec_set_power_saving(bool enable) {
    uint8_t control_value;
    esp_err_t ret;

    // Read the current control register value
    uint8_t data[4] = {0};
    ret             = typec_read_all(data);
    if (ret != ESP_OK) {
        return ret;
    }

    control_value = data[REG_CONTROL];

    // Set or clear the power-saving bit (bit 7)
    if (enable) {
        control_value |= 0x80;
    } else {
        control_value &= ~0x80;
    }

    // Write the updated control register value
    return typec_write_control_register(control_value);
}

esp_err_t typec_set_dual_role(bool try_snk) {
    uint8_t control_value;
    esp_err_t ret;

    // Read the current control register value
    uint8_t data[4] = {0};
    ret             = typec_read_all(data);
    if (ret != ESP_OK) {
        return ret;
    }

    control_value = data[REG_CONTROL];

    // Set or clear the dual-role bit (bit 6)
    if (try_snk) {
        control_value |= 0x40;
    } else {
        control_value &= ~0x40;
    }

    // Write the updated control register value
    return typec_write_control_register(control_value);
}

esp_err_t typec_set_accessory_detection(bool enable) {
    uint8_t control_value;
    esp_err_t ret;

    // Read the current control register value
    uint8_t data[4] = {0};
    ret             = typec_read_all(data);
    if (ret != ESP_OK) {
        return ret;
    }

    control_value = data[REG_CONTROL];

    // Set or clear the accessory detection bit (bit 5)
    if (enable) {
        control_value |= 0x20;
    } else {
        control_value &= ~0x20;
    }

    // Write the updated control register value
    return typec_write_control_register(control_value);
}

esp_err_t typec_set_charging_current(typec_charging_current_t current) {
    uint8_t control_value;
    esp_err_t ret;

    // Read the current control register value
    uint8_t data[4] = {0};
    ret             = typec_read_all(data);
    if (ret != ESP_OK) {
        return ret;
    }

    control_value = data[REG_CONTROL];

    // Set the charging current bits (bits 4:3)
    control_value &= ~0x18; // Clear the current bits
    control_value |= ((current - 1) << 3);

    // Write the updated control register value
    return typec_write_control_register(control_value);
}

esp_err_t typec_set_role(typec_port_role_t role) {
    uint8_t control_value;
    esp_err_t ret;

    // Read the current control register value
    uint8_t data[4] = {0};
    ret             = typec_read_all(data);
    if (ret != ESP_OK) {
        return ret;
    }

    control_value = data[REG_CONTROL];

    // Set the role bits (bits 2:1)
    control_value &= ~0x06; // Clear the role bits
    control_value |= (role << 1);

    // Write the updated control register value
    return typec_write_control_register(control_value);
}

esp_err_t typec_set_interrupt_mask(bool mask) {
    uint8_t control_value;
    esp_err_t ret;

    // Read the current control register value
    uint8_t data[4] = {0};
    ret             = typec_read_all(data);
    if (ret != ESP_OK) {
        return ret;
    }

    control_value = data[REG_CONTROL];

    // Set or clear the interrupt mask bit (bit 0)
    if (mask) {
        control_value |= 0x01;
    } else {
        control_value &= ~0x01;
    }

    // Write the updated control register value
    return typec_write_control_register(control_value);
}