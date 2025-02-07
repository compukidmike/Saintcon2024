#include <stdio.h>
#include "esp_log.h"
#include "esp_timer.h"

#include "accel.h"
#include "i2c_manager.h"

static const char *TAG = "accel";

// I2C switch channel and interrupt pin used
#define MC3419_MOTION_INTERRUPT I2C_SWITCH_INT2
#define MC3419_FIFO_INTERRUPT   I2C_SWITCH_INT0

// Event group bits for accelerometer interrupts
#define ACCEL_EVENT_MOTION (1 << 0)
#define ACCEL_EVENT_FIFO   (1 << 1)

// Device configuration
i2c_manager_device_config_t accel_device = {
    .bus_index = I2C_BUS_OTHER,
    .channel   = I2C_SWITCH_CHANNEL_2,
    .config =
        {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address  = MC3419_I2C_ADDR,
            .scl_speed_hz    = 400000, // Chip can support up to 1MHz so adjust if needed
        },
};

// Event group for accelerometer interrupts
static EventGroupHandle_t accel_event_group;
static QueueHandle_t accel_event_queue;

// Structure to send data via the queue
typedef struct {
    mc3419_accel_data_t data;
    uint32_t timestamp;
} accel_event_t;

static void accel_event_task(void *arg) {
    const static mc3419_interrupt_t interrupts[] = {MC3419_INT_TILT,  MC3419_INT_FLIP,    MC3419_INT_ANYM,
                                                    MC3419_INT_SHAKE, MC3419_INT_TILT_35, MC3419_INT_ACQ};
    ESP_LOGD(TAG, "Accelerometer event task started");
    mc3419_intr_stat_t previous_intr_stat = {0};

    while (true) {
        // ESP_LOGD(TAG, "Waiting for accelerometer interrupt");
        // EventBits_t bits =
        //     xEventGroupWaitBits(i2c_switch_int, MC3419_FIFO_INTERRUPT | MC3419_MOTION_INTERRUPT, pdTRUE, pdFALSE,
        //     portMAX_DELAY);

        // Read the interrupt status register to determine which interrupt(s) occurred
        mc3419_intr_stat_t intr_stat;
        esp_err_t err = accel_get_interrupt_status(&intr_stat);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read interrupt status");
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        // // Reset the interrupt status since we've read it
        // if (intr_stat.raw != 0) {
        //     ESP_ERROR_CHECK(accel_reset_interrupt_status());
        // }

        // Check if there are any new interrupts
        if (intr_stat.raw != previous_intr_stat.raw) {
            // DEBUG: Log the interrupt status for each interrupt
            char buffer[256] = {0};
            size_t offset    = 0;
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\nMC3419 Interrupt Triggered: \n");
            for (size_t i = 0; i < sizeof(interrupts) / sizeof(interrupts[0]); i++) {
                switch (interrupts[i]) {
                    case MC3419_INT_TILT:
                        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "  Tilt: %d\n", intr_stat.tilt_int);
                        break;
                    case MC3419_INT_FLIP:
                        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "  Flip: %d\n", intr_stat.flip_int);
                        break;
                    case MC3419_INT_ANYM:
                        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "  AnyMotion: %d\n", intr_stat.anym_int);
                        break;
                    case MC3419_INT_SHAKE:
                        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "  Shake: %d\n", intr_stat.shake_int);
                        break;
                    case MC3419_INT_TILT_35:
                        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "  Tilt 35: %d\n", intr_stat.tilt_35_int);
                        break;
                    case MC3419_INT_ACQ:
                        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "  Acquisition: %d\n", intr_stat.acq_int);
                        break;
                    default: break;
                }
            }
            ESP_LOGD(TAG, "%s", buffer);

            // if (bits & MC3419_FIFO_INTERRUPT) {
            //     ESP_LOGD(TAG, "FIFO interrupt detected");
            //     // Read the FIFO data
            //     mc3419_accel_data_t data;
            //     ESP_ERROR_CHECK(accel_read_data(&data));
            //     // Send the data to the queue
            //     accel_event_t event = {
            //         .data      = data,
            //         .timestamp = esp_timer_get_time(),
            //     };
            //     xQueueSend(accel_event_queue, &event, 0);
            // }

            // if (bits & MC3419_MOTION_INTERRUPT) {
            if (intr_stat.raw & (MC3419_INT_TILT | MC3419_INT_FLIP | MC3419_INT_ANYM | MC3419_INT_SHAKE | MC3419_INT_TILT_35)) {
                ESP_LOGD(TAG, "Motion interrupt detected");
                // Read the status register to clear the interrupt
                mc3419_status_t status;
                ESP_ERROR_CHECK(accel_get_status(&status));
                // Send the event to the event group
                xEventGroupSetBits(accel_event_group, ACCEL_EVENT_MOTION);
            }

            // Update previous interrupt status
            previous_intr_stat = intr_stat;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static size_t binstr(uint8_t x, const char *separator, char *buffer, size_t buffer_size) {
    size_t o = 0;
    for (int i = 7; i >= 0; i--) {
        const char *sep = i == 0 ? "" : separator;
        o += snprintf(buffer + o, buffer_size - o, "%d%s", (x >> i) & 0x01, sep);
    }
    return o;
}

// Custom log function that replaces a format specifier of '%b' with a binary string using binstr
static void LOG_B(const char *tag, const char *format, uint8_t x) {
    char buffer[256] = {0};
    size_t offset    = 0;
    for (size_t i = 0; format[i] != '\0'; i++) {
        if (format[i] == '%' && format[i + 1] == 'b') {
            offset += binstr(x, " - ", buffer + offset, sizeof(buffer) - offset);
            i++;
        } else {
            buffer[offset++] = format[i];
        }
    }
    ESP_LOGD(tag, "%s", buffer);
}

esp_err_t accel_init(void) {
    i2c_master_dev_handle_t dev_handle = NULL;
    esp_err_t ret;
    ESP_RETURN_ON_ERROR(i2c_manager_add_device(&accel_device, &dev_handle), TAG, "Failed to add I2C device to bus");

    // DEBUG: Get the current mode
    mc3419_mode_t mode;
    ret = accel_get_mode(&mode);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get device mode");
        return ret;
    }
    LOG_B(TAG, "Current Mode: %b", mode.state);

    // First put the chip in standby mode
    ret = accel_set_mode(MODE_STANDBY);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set device to standby mode");
        return ret;
    }

    // // Reset
    // ret = accel_reset();
    // if (ret != ESP_OK) {
    //     ESP_LOGE(TAG, "Failed to reset device");
    //     return ret;
    // } else {
    //     ESP_LOGD(TAG, "Accelerometer reset");
    // }

    // DEBUG: Get the current mode
    ret = accel_get_mode(&mode);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get device mode");
        return ret;
    }
    LOG_B(TAG, "Current Mode: %b", mode.state);

    // Set the GPIO polarity
    ret = accel_set_gpio_polarity(false, false);
    // ret = accel_set_gpio_polarity(true, true);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set GPIO polarity");
        return ret;
    }

    // // DEBUG: Get the current sample rate
    // mc3419_sr_t sr;
    // ret = accel_get_sample_rate(&sr);
    // if (ret != ESP_OK) {
    //     ESP_LOGE(TAG, "Failed to get sample rate");
    //     return ret;
    // }
    // LOG_B(TAG, "Current Sample Rate: %b", sr.raw);

    // // Set the sample rate to 125Hz
    // ret = accel_set_sample_rate(RATE_1000_HZ);
    // if (ret != ESP_OK) {
    //     ESP_LOGE(TAG, "Failed to set sample rate");
    //     return ret;
    // }

    // // DEBUG: Get the current sample rate
    // ret = accel_get_sample_rate(&sr);
    // if (ret != ESP_OK) {
    //     ESP_LOGE(TAG, "Failed to get sample rate");
    //     return ret;
    // }
    // LOG_B(TAG, "Current Sample Rate: %b", sr.raw);

    // Set the g-range to ±4g
    ret = accel_set_range(RANGE_2G);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set range");
        return ret;
    }

    // // Set the g-range to ±4g
    // ret = accel_set_range(RANGE_4G);
    // if (ret != ESP_OK) {
    //     ESP_LOGE(TAG, "Failed to set range");
    //     return ret;
    // }

    // DEBUG: Get the current interrupt configuration
    mc3419_intr_ctrl_t intr_ctrl;
    ret = accel_get_interrupt_config(&intr_ctrl);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get interrupt configuration");
        return ret;
    }
    LOG_B(TAG, "Interrupt Configuration: %b", intr_ctrl.raw);

    // // Enable the features we want to use
    // // ret = accel_enable_features(MC3419_FEATURE_TF | MC3419_FEATURE_ANYM | MC3419_FEATURE_SHAKE | MC3419_FEATURE_TILT_35);
    // // ret = accel_enable_features(MC3419_FEATURE_ANYM);
    // ret = accel_enable_features(MC3419_FEATURE_ANYM | MC3419_FEATURE_SHAKE | MC3419_FEATURE_TILT_35);
    // // ret = accel_enable_features(MC3419_FEATURE_TF | MC3419_FEATURE_ANYM);
    // if (ret != ESP_OK) {
    //     ESP_LOGE(TAG, "Failed to set features");
    //     return ret;
    // }

    // DEBUG: Get the current interrupt configuration
    ret = accel_get_interrupt_config(&intr_ctrl);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get interrupt configuration");
        return ret;
    }
    LOG_B(TAG, "Interrupt Configuration: %b", intr_ctrl.raw);

    // Enable the interrupts we want to use
    // ret = accel_set_interrupt_config(MC3419_INT_TILT | MC3419_INT_FLIP | MC3419_INT_ANYM | MC3419_INT_SHAKE |
    //                                  MC3419_INT_TILT_35 | MC3419_INT_ACQ, true);
    // ret = accel_set_interrupt_config(MC3419_INT_ANYM, true);
    // ret = accel_set_interrupt_config(MC3419_INT_TILT_35 | MC3419_INT_ACQ, true);
    ret = accel_set_interrupt_config(/* MC3419_INT_ANYM | */ MC3419_INT_ACQ, true);
    // ret = accel_set_interrupt_config(MC3419_INT_ANYM, true);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set interrupts");
        return ret;
    }

    // DEBUG: Get the current interrupt configuration
    ret = accel_get_interrupt_config(&intr_ctrl);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get interrupt configuration");
        return ret;
    }
    LOG_B(TAG, "Interrupt Configuration: %b", intr_ctrl.raw);

    // Put the device into wake mode
    ret = accel_set_mode(MODE_WAKE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set device to wake mode");
        return ret;
    }

    // DEBUG: Get the current mode
    ret = accel_get_mode(&mode);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get device mode");
        return ret;
    }
    LOG_B(TAG, "Current Mode: %b", mode.state);

    // Create the event group
    accel_event_group = xEventGroupCreate();
    if (accel_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create event group");
        return ESP_FAIL;
    }

    // Create the event queue
    accel_event_queue = xQueueCreate(10, sizeof(accel_event_t));

    // Create the event task
    // xTaskCreate(accel_event_task, "accel_event_task", 4096, NULL, 5, NULL);

    return ESP_OK;
}

esp_err_t accel_reset() {
    // Put the device in standby mode
    ESP_RETURN_ON_ERROR(accel_set_mode(MODE_STANDBY), TAG, "Failed to set device to standby mode");
    // Reset the device
    mc3419_register_t reg = 0x1C;
    uint8_t data[2]       = {reg, 0x80};
    ESP_RETURN_ON_ERROR(i2c_manager_transmit(&accel_device, data, sizeof(data), 100), TAG, "Failed to reset device");
    vTaskDelay(pdMS_TO_TICKS(50)); // Wait for the device to reset
    // Disable interrupts
    reg     = MC3419_REG_INTR_CTRL;
    data[0] = reg;
    data[1] = 0x00;
    ESP_RETURN_ON_ERROR(i2c_manager_transmit(&accel_device, data, sizeof(data), 100), TAG, "Failed to disable interrupts");
    // Set analog gain to 1.00X
    reg     = 0x2B;
    data[0] = reg;
    data[1] = 0x00;
    ESP_RETURN_ON_ERROR(i2c_manager_transmit(&accel_device, data, sizeof(data), 100), TAG, "Failed to set analog gain");
    // Disable DCM
    reg     = 0x15;
    data[0] = reg;
    data[1] = 0x00;
    ESP_RETURN_ON_ERROR(i2c_manager_transmit(&accel_device, data, sizeof(data), 100), TAG, "Failed to disable DCM");

    // Make sure the device is in standby mode
    ESP_RETURN_ON_ERROR(accel_set_mode(MODE_STANDBY), TAG, "Failed to set device to standby mode");
    vTaskDelay(pdMS_TO_TICKS(50)); // Wait for the device to reset

    return ESP_OK;
}

esp_err_t accel_get_device_id(mc3419_chip_id_t *id) {
    mc3419_register_t reg = MC3419_REG_CHIP_ID;
    // ESP_RETURN_ON_ERROR(mc3419_read_register(0x18, id, 1), TAG, "Failed to read device ID");
    ESP_RETURN_ON_ERROR(i2c_manager_transmit_receive(&accel_device, &reg, 1, id, 1, 100), TAG, "Failed to read device ID");
    return ESP_OK;
}

esp_err_t accel_set_mode(mc3419_mode_state_t state) {
    mc3419_register_t reg = MC3419_REG_MODE;
    mc3419_mode_t mode    = {0};
    // Get the current mode
    ESP_RETURN_ON_ERROR(i2c_manager_transmit_receive(&accel_device, &reg, 1, &mode.raw, 1, 100), TAG,
                        "Failed to read mode state");
    // Print the current mode state bits
    char buffer[256] = {0};
    size_t offset    = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Current Mode State bits: | ");
    binstr(mode.raw, " - ", buffer + offset, sizeof(buffer) - offset);
    ESP_LOGD(TAG, "%s |", buffer);
    ESP_LOGD(TAG, "Mode State: %02X", mode.state);

    // Set the new mode state
    mode.state = state;
    // Write the new mode state
    uint8_t data[2] = {reg, mode.raw};
    ESP_RETURN_ON_ERROR(i2c_manager_transmit(&accel_device, data, sizeof(data), 100), TAG, "Failed to set mode state");
    return ESP_OK;
}

esp_err_t accel_get_mode(mc3419_mode_t *mode) {
    mc3419_register_t reg = MC3419_REG_MODE;
    // Read the mode state register
    ESP_RETURN_ON_ERROR(i2c_manager_transmit_receive(&accel_device, &reg, 1, &mode->raw, 1, 100), TAG,
                        "Failed to read mode state");
    return ESP_OK;
}

esp_err_t accel_set_interrupt_config(uint8_t intr_en, bool autoclear) {
    mc3419_register_t reg        = MC3419_REG_INTR_CTRL;
    mc3419_intr_ctrl_t intr_ctrl = {0}; // POR is 0x00... we'll apply the bitmask values
    // Get the current interrupt enable register so we can preserve the auto-clear bit
    ESP_RETURN_ON_ERROR(i2c_manager_transmit_receive(&accel_device, &reg, 1, &intr_ctrl.raw, 1, 100), TAG,
                        "Failed to read interrupt enable");
    // Print the current interrupt enable bits
    char buffer[256] = {0};
    size_t offset    = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Current Interrupt Enable bits: | ");
    binstr(intr_ctrl.raw, " - ", buffer + offset, sizeof(buffer) - offset);
    ESP_LOGD(TAG, "%s |", buffer);
    // Set the new interrupt enable bits for each interrupt
    intr_ctrl.tilt_int_en    = (intr_en & MC3419_INT_TILT) ? 1 : 0;
    intr_ctrl.flip_int_en    = (intr_en & MC3419_INT_FLIP) ? 1 : 0;
    intr_ctrl.anym_int_en    = (intr_en & MC3419_INT_ANYM) ? 1 : 0;
    intr_ctrl.shake_int_en   = (intr_en & MC3419_INT_SHAKE) ? 1 : 0;
    intr_ctrl.tilt_35_int_en = (intr_en & MC3419_INT_TILT_35) ? 1 : 0;
    intr_ctrl.acq_int_en     = (intr_en & MC3419_INT_ACQ) ? 1 : 0;
    intr_ctrl.auto_clr_en    = autoclear ? 1 : 0;
    // Write the interrupt enable register
    uint8_t data[2] = {reg, intr_ctrl.raw};
    ESP_RETURN_ON_ERROR(i2c_manager_transmit(&accel_device, data, sizeof(data), 100), TAG, "Failed to set interrupt enable");
    return ESP_OK;
}

esp_err_t accel_get_interrupt_config(mc3419_intr_ctrl_t *intr_ctrl) {
    mc3419_register_t reg = MC3419_REG_INTR_CTRL;
    // Read the interrupt enable register
    ESP_RETURN_ON_ERROR(i2c_manager_transmit_receive(&accel_device, &reg, 1, &intr_ctrl->raw, 1, 100), TAG,
                        "Failed to read interrupt enable");
    return ESP_OK;
}

esp_err_t accel_set_sample_rate(mc3419_rate_t rate) {
    mc3419_register_t reg = MC3419_REG_SR;
    mc3419_sr_t sr        = {0};
    // Get the current sample rate
    ESP_RETURN_ON_ERROR(i2c_manager_transmit_receive(&accel_device, &reg, 1, &sr.raw, 1, 100), TAG, "Failed to read sample rate");
    // Set the new sample rate
    sr.rate = rate;
    // Write the new sample rate
    uint8_t data[2] = {reg, sr.raw};
    ESP_RETURN_ON_ERROR(i2c_manager_transmit(&accel_device, data, sizeof(data), 100), TAG, "Failed to set sample rate");
    return ESP_OK;
}

esp_err_t accel_get_sample_rate(mc3419_sr_t *sr) {
    mc3419_register_t reg = MC3419_REG_SR;
    // Read the sample rate register
    ESP_RETURN_ON_ERROR(i2c_manager_transmit_receive(&accel_device, &reg, 1, &sr->raw, 1, 100), TAG,
                        "Failed to read sample rate");
    return ESP_OK;
}

esp_err_t accel_enable_features(uint8_t feature_en) {
    mc3419_register_t reg            = MC3419_REG_MOTION_CTRL;
    mc3419_motion_ctrl_t motion_ctrl = {0};
    // Get the current feature enable register
    ESP_RETURN_ON_ERROR(i2c_manager_transmit_receive(&accel_device, &reg, 1, &motion_ctrl.raw, 1, 100), TAG,
                        "Failed to read feature enable");
    // Print the current feature enable bits
    char buffer[256] = {0};
    size_t offset    = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Current Feature Enable bits: | ");
    binstr(motion_ctrl.raw, " - ", buffer + offset, sizeof(buffer) - offset);
    ESP_LOGD(TAG, "%s |", buffer);
    // Set the new feature enable bits
    motion_ctrl.tf_enable  = (feature_en & MC3419_FEATURE_TF) ? 1 : 0;
    motion_ctrl.anym_en    = (feature_en & MC3419_FEATURE_ANYM) ? 1 : 0;
    motion_ctrl.shake_en   = (feature_en & MC3419_FEATURE_SHAKE) ? 1 : 0;
    motion_ctrl.tilt_35_en = (feature_en & MC3419_FEATURE_TILT_35) ? 1 : 0;
    // Write the new feature enable register
    uint8_t data[2] = {reg, motion_ctrl.raw};
    ESP_RETURN_ON_ERROR(i2c_manager_transmit(&accel_device, data, sizeof(data), 100), TAG, "Failed to set feature enable");
    return ESP_OK;
}

esp_err_t accel_set_range(mc3419_range_res_t res) {
    mc3419_register_t reg = MC3419_REG_RANGE;
    mc3419_range_t range  = {0};
    // Get the current range
    ESP_RETURN_ON_ERROR(i2c_manager_transmit_receive(&accel_device, &reg, 1, &range.raw, 1, 100), TAG, "Failed to read range");
    // Set the new range
    range.range = res;
    // Write the new range
    uint8_t data[2] = {reg, range.raw};
    ESP_RETURN_ON_ERROR(i2c_manager_transmit(&accel_device, data, sizeof(data), 100), TAG, "Failed to set range");
    return ESP_OK;
}

esp_err_t accel_get_range(mc3419_range_t *range) {
    mc3419_register_t reg = MC3419_REG_RANGE;
    // Read the range register
    ESP_RETURN_ON_ERROR(i2c_manager_transmit_receive(&accel_device, &reg, 1, &range->raw, 1, 100), TAG, "Failed to read range");
    return ESP_OK;
}

esp_err_t accel_read_data(mc3419_accel_data_t *data) {
    mc3419_register_t reg = MC3419_REG_XOUT_EX_L; // Start reading from XOUT_EX_L for X, Y, Z data (6 bytes)
    // Read the raw data
    ESP_RETURN_ON_ERROR(i2c_manager_transmit_receive(&accel_device, &reg, 1, data, sizeof(*data), 100), TAG,
                        "Failed to read data");
    return ESP_OK;
}

esp_err_t accel_get_status(mc3419_status_t *status) {
    mc3419_register_t reg = MC3419_REG_STATUS;
    // Read the status register
    ESP_RETURN_ON_ERROR(i2c_manager_transmit_receive(&accel_device, &reg, 1, status, 1, 100), TAG, "Failed to read status");
    return ESP_OK;
}

esp_err_t accel_get_interrupt_status(mc3419_intr_stat_t *status) {
    mc3419_register_t reg = MC3419_REG_INTR_STAT;
    // Read the interrupt status register
    ESP_RETURN_ON_ERROR(i2c_manager_transmit_receive(&accel_device, &reg, 1, status, 1, 100), TAG,
                        "Failed to read interrupt status");
    return ESP_OK;
}

// esp_err_t accel_reset_interrupt_status() {
//     mc3419_register_t reg = MC3419_REG_INTR_STAT;
//     mc3419_intr_stat_t status;
//     ESP_LOGD(TAG, "Resetting interrupt status");
//     // Read the interrupt status register to get the current status
//     ESP_RETURN_ON_ERROR(i2c_manager_transmit_receive(&accel_device, &reg, 1, &status, 1, 100), TAG,
//                         "Failed to read interrupt status");
//     // Write back with a 1 in the reserved bit to clear the interrupt status
//     status._reserved0 = 1;
//     uint8_t data[2]   = {reg, status.raw};
//     ESP_RETURN_ON_ERROR(i2c_manager_transmit(&accel_device, data, sizeof(data), 100), TAG, "Failed to reset interrupt status");
//     return ESP_OK;
// }

esp_err_t accel_reset_interrupt_status() {
    mc3419_register_t reg = MC3419_REG_INTR_STAT;
    uint8_t clear         = 0xFF; // Write 0xFF to clear all interrupt statuses
    ESP_LOGD(TAG, "Resetting interrupt status");
    uint8_t data[2] = {reg, clear};
    ESP_RETURN_ON_ERROR(i2c_manager_transmit(&accel_device, data, sizeof(data), 100), TAG, "Failed to reset interrupt status");
    return ESP_OK;
}

esp_err_t accel_set_gpio_polarity(bool int1_active_high, bool int2_active_high) {
    mc3419_register_t reg = MC3419_REG_GPIO_CTRL;
    mc3419_gpio_ctrl_t gpio_ctrl;
    // Get the current GPIO control register
    ESP_RETURN_ON_ERROR(i2c_manager_transmit_receive(&accel_device, &reg, 1, &gpio_ctrl.raw, 1, 100), TAG,
                        "Failed to read GPIO control");
    // Set the new GPIO polarity
    gpio_ctrl.gpio1_intn1_iah = int1_active_high ? 1 : 0;
    gpio_ctrl.gpio2_intn2_iah = int2_active_high ? 1 : 0;
    // Write the new GPIO control register
    uint8_t data[2] = {reg, gpio_ctrl.raw};
    ESP_RETURN_ON_ERROR(i2c_manager_transmit(&accel_device, data, sizeof(data), 100), TAG, "Failed to set GPIO control");
    return ESP_OK;
}

esp_err_t accel_set_gpio_drive_mode(bool int1_push_pull, bool int2_push_pull) {
    mc3419_register_t reg = MC3419_REG_GPIO_CTRL;
    mc3419_gpio_ctrl_t gpio_ctrl;
    // Get the current GPIO control register
    ESP_RETURN_ON_ERROR(i2c_manager_transmit_receive(&accel_device, &reg, 1, &gpio_ctrl.raw, 1, 100), TAG,
                        "Failed to read GPIO control");
    // Set the new GPIO drive mode
    gpio_ctrl.gpio1_intn1_ipp = int1_push_pull ? 1 : 0;
    gpio_ctrl.gpio2_intn2_ipp = int2_push_pull ? 1 : 0;
    // Write the new GPIO control register
    uint8_t data[2] = {reg, gpio_ctrl.raw};
    ESP_RETURN_ON_ERROR(i2c_manager_transmit(&accel_device, data, sizeof(data), 100), TAG, "Failed to set GPIO control");
    return ESP_OK;
}
