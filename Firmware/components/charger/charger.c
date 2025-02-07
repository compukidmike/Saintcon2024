#include <stdio.h>
#include "charger.h"
#include "driver/gpio.h"
#include "i2c_manager.h"

#define BQ24296M_PSEL_PIN CONFIG_CHARGER_PSEL_GPIO // Power select
#define BQ24296M_INT_PIN  CONFIG_CHARGER_INT_GPIO  // Interrupt
#define BQ24296M_OTG_PIN  CONFIG_CHARGER_OTG_GPIO  // OTG
#define BQ24296M_CE_PIN   CONFIG_CHARGER_CE_GPIO   // Charge enable

static const char *TAG = "charger";

// Device configuration
i2c_manager_device_config_t charger_device = {
    .bus_index = I2C_BUS_OTHER,
    .channel   = I2C_SWITCH_CHANNEL_3,
    .config =
        {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address  = BQ24296M_I2C_ADDRESS,
            .scl_speed_hz    = 400000,
        },
};

// Semaphore for handling charger interrupts
static SemaphoreHandle_t charger_interrupt = NULL;

// Event group and queue for charger events and status
EventGroupHandle_t charger_event_group = NULL;
QueueHandle_t charger_status_queue     = NULL;
QueueHandle_t charger_fault_queue      = NULL;

// Charger status and fault
static charger_system_status_t current_status;
static charger_new_fault_t current_fault;

/**
 * @brief ISR handler for the charger interrupt
 */
static void IRAM_ATTR charger_isr_handler(void *_arg) {
    (void)_arg;
    BaseType_t task_woken = pdFALSE;
    xSemaphoreGiveFromISR(charger_interrupt, &task_woken);
    portYIELD_FROM_ISR(task_woken);
}

/**
 * @brief Task to handle interrupts from the charger
 */
static void charger_interrupt_task(void *_arg) {
    (void)_arg; // Unused
    charger_system_status_t last_status = {0};
    charger_new_fault_t last_fault      = {0};
    while (1) {
        if (xSemaphoreTake(charger_interrupt, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "Charger interrupt detected");
            bool status_changed = false;
            charger_new_fault_t prev_fault;
            charger_read_status(&current_status);
            charger_read_faults(&prev_fault);    // 1st time read - clear any previous fault
            charger_read_faults(&current_fault); // 2nd time read - get any faults that still present
            ESP_LOGD(TAG, "Faults: 0x%02x, 0x%02x", prev_fault.raw, current_fault.raw);

            // Disable charging if charging is complete.
            // NOTE: I _was_ just going to rely on the power_manager to handle this, but it's not quick
            //       enough due to event delays so I'm going to handle this specific case here.
            if (current_status.chrg_stat == CHRG_STAT_CHARGE_DONE) {
                charger_set_charge_enable(false);
            }

            // Check if the status or fault has changed and notify
            if (current_fault.raw != last_fault.raw) {
                last_fault = current_fault;
                xQueueSend(charger_fault_queue, &current_fault, portMAX_DELAY);
                xEventGroupSetBits(charger_event_group, CHARGER_FAULT);
            }
            if (current_status.raw != last_status.raw) {
                status_changed = true;
                last_status    = current_status;
                xQueueSend(charger_status_queue, &current_status, portMAX_DELAY);
                xEventGroupSetBits(charger_event_group, CHARGER_EVENT);
            }

            if (status_changed) {
                // Read status and log details - this interrupt can be triggered by the following events:
                // - USB/adapter source identified (through PSEL detection and OTG pin)
                // - Good input source detected
                //   - not in sleep
                //   - VBUS below Vacov threshold
                //   - current limit above Ibadsrc
                // - Input removed or VBUS above Vacov threshold
                // - Charge complete
                // - Any fault event in REG09
                ESP_LOGD(TAG, "Raw Status: 0x%02x", current_status.raw);
                ESP_LOGD(TAG, "VSYS: %s", current_status.vsys_stat ? "BAT < VSYSMIN" : "BAT > VSYSMIN");
                ESP_LOGD(TAG, "Thermal: %s", current_status.therm_stat ? "Thermal regulation" : "Normal");
                ESP_LOGD(TAG, "PG Status: %s", current_status.pg_stat ? "Power good" : "Power not good");
                ESP_LOGD(TAG, "DPM Status: %s", current_status.dpm_stat ? "VINDPM or IINDPM" : "Not in DPM");
                ESP_LOGD(TAG, "Charge: %s",
                         current_status.chrg_stat == CHRG_STAT_NOT_CHARGING    ? "Not charging"
                         : current_status.chrg_stat == CHRG_STAT_PRE_CHARGING  ? "Pre-charge"
                         : current_status.chrg_stat == CHRG_STAT_FAST_CHARGING ? "Fast charge"
                                                                               : "Charge termination");
                ESP_LOGD(TAG, "VBUS: %s",
                         current_status.vbus_stat == VBUS_STAT_UNKNOWN    ? "Unknown"
                         : current_status.vbus_stat == VBUS_STAT_USB_HOST ? "USB host"
                         : current_status.vbus_stat == VBUS_STAT_ADAPTER  ? "Adapter"
                                                                          : "OTG");

                // Read faults and log details
                ESP_LOGD(TAG, "Faults: 0x%02x", current_fault.raw);
            }
        }
    }
}

esp_err_t charger_init() {
    // Add the charger to the I²C manager device list bus
    i2c_master_dev_handle_t dev_handle = NULL;
    ESP_RETURN_ON_ERROR(i2c_manager_add_device(&charger_device, &dev_handle), TAG, "Failed to add I2C device to bus");

    // Configure the PSEL, OTG, and CE pins for output
    gpio_config_t output_conf = {
        .pin_bit_mask = (1ULL << BQ24296M_PSEL_PIN | 1ULL << BQ24296M_OTG_PIN | 1ULL << BQ24296M_CE_PIN),
        .mode         = GPIO_MODE_OUTPUT,
    };
    gpio_config(&output_conf);

    // Configure the interrupt pin for input
    gpio_config_t int_conf = {
        .pin_bit_mask = 1ULL << BQ24296M_INT_PIN,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .intr_type    = GPIO_INTR_NEGEDGE,
    };
    gpio_config(&int_conf);

    // Set output pins to default values
    gpio_set_level(BQ24296M_PSEL_PIN, 0);
    gpio_set_level(BQ24296M_OTG_PIN, 1);
    gpio_set_level(BQ24296M_CE_PIN, 0);

    // Create the semaphore for handling charger interrupts
    charger_interrupt = xSemaphoreCreateBinary();

    // Create the event group and queues for charger events and status/faults
    charger_event_group  = xEventGroupCreate();
    charger_status_queue = xQueueCreate(CHARGER_QUEUE_SIZE, sizeof(charger_system_status_t));
    charger_fault_queue  = xQueueCreate(CHARGER_QUEUE_SIZE, sizeof(charger_new_fault_t));

    // Register the ISR handler
    ESP_RETURN_ON_ERROR(gpio_isr_handler_add(BQ24296M_INT_PIN, charger_isr_handler, NULL), TAG, "Failed to add ISR handler");

    // Create the task to handle charger interrupts
    xTaskCreate(charger_interrupt_task, "charger_interrupt_task", 4096, NULL, 10, NULL);

    return ESP_OK;
}

esp_err_t charger_read_register(bq24296m_register_t reg, uint8_t *data) {
    ESP_RETURN_ON_ERROR(i2c_manager_transmit_receive(&charger_device, (uint8_t *)&reg, 1, data, 1, 100), TAG,
                        "Failed to read register 0x%02x", reg);
    return ESP_OK;
}

esp_err_t charger_write_register(bq24296m_register_t reg, uint8_t data) {
    uint8_t buffer[2];
    buffer[0] = reg;
    buffer[1] = data;
    ESP_RETURN_ON_ERROR(i2c_manager_transmit(&charger_device, buffer, 2, 100), TAG,
                        "Failed to write data 0x%02x to register 0x%02x", data, reg);
    return ESP_OK;
}

esp_err_t charger_set_input_power(uint8_t voltage, uint8_t current) {
    charger_input_src_ctrl_t reg;
    ESP_RETURN_ON_ERROR(charger_read_register(INPUT_SRC_CTRL, &reg.raw), TAG, "Failed to read input power");
    reg.vindpm = voltage;
    reg.iinlim = current;
    ESP_RETURN_ON_ERROR(charger_write_register(INPUT_SRC_CTRL, reg.raw), TAG, "Failed to set input power");
    return ESP_OK;
}

esp_err_t charger_set_charge_enable(bool enable) {
    ESP_LOGD(TAG, "Setting charge enable to %d", enable);

    // Set the CE pin state
    ESP_RETURN_ON_ERROR(gpio_set_level(BQ24296M_CE_PIN, !enable), TAG, "Failed to set charge enable pin state");

    // Get current power-on configuration
    charger_power_on_config_t reg;
    ESP_RETURN_ON_ERROR(charger_read_register(POWER_ON_CONFIG, &reg.raw), TAG, "Failed to read power-on configuration");

    // Set the CHG bit
    reg.chg_config = enable;

    // Write the register back
    return charger_write_register(POWER_ON_CONFIG, reg.raw);
}

esp_err_t charger_set_minimum_system_voltage(charger_sys_min_t vsysmin) {
    // Get current power-on configuration
    charger_power_on_config_t reg;
    ESP_RETURN_ON_ERROR(charger_read_register(POWER_ON_CONFIG, &reg.raw), TAG, "Failed to read power-on configuration");

    // Set the SYS_MIN voltage
    reg.sys_min = vsysmin;

    // Write the register back
    return charger_write_register(POWER_ON_CONFIG, reg.raw);
}

esp_err_t charger_set_fast_charge_current(charger_ichg_t ichg, bool force_20pct) {
    // Get current charge current configuration
    charger_charge_current_ctrl_t reg;
    ESP_RETURN_ON_ERROR(charger_read_register(CHARGE_CURRENT_CTRL, &reg.raw), TAG, "Failed to read charge current configuration");

    // Set the charge current
    reg.ichg        = ichg;
    reg.force_20pct = force_20pct;

    // Write the register back
    return charger_write_register(CHARGE_CURRENT_CTRL, reg.raw);
}

esp_err_t charger_set_termination_current(charger_iterm_t iterm) {
    // Get current termination current configuration
    charger_prechg_term_current_ctrl_t reg;
    ESP_RETURN_ON_ERROR(charger_read_register(PRECHG_TERM_CURRENT_CTRL, &reg.raw), TAG,
                        "Failed to read termination current configuration");

    // Set the termination current
    reg.iterm = iterm;

    // Write the register back
    return charger_write_register(PRECHG_TERM_CURRENT_CTRL, reg.raw);
}

esp_err_t charger_set_charge_voltage_limit(charger_vreg_t vreg) {
    // Get current charge voltage configuration
    charger_charge_voltage_ctrl_t reg;
    ESP_RETURN_ON_ERROR(charger_read_register(CHARGE_VOLTAGE_CTRL, &reg.raw), TAG, "Failed to read charge voltage configuration");

    // Set the charge voltage
    reg.vreg = vreg;

    // Write the register back
    return charger_write_register(CHARGE_VOLTAGE_CTRL, reg.raw);
}

esp_err_t charger_reset_watchdog() {
    // Get current power-on configuration
    charger_power_on_config_t reg;
    ESP_RETURN_ON_ERROR(charger_read_register(POWER_ON_CONFIG, &reg.raw), TAG, "Failed to read power-on configuration");

    // Set the watchdog reset bit
    reg.wdt_reset = 1;

    // Write the register back
    return charger_write_register(POWER_ON_CONFIG, reg.raw);
}

esp_err_t charger_set_watchdog_timer(charger_watchdog_timer_t timer) {
    // Get current termination/timer configuration
    charger_charge_term_timer_ctrl_t reg;
    ESP_RETURN_ON_ERROR(charger_read_register(CHARGE_TERM_TIMER_CTRL, &reg.raw), TAG,
                        "Failed to read termination/timer configuration");

    // Set the watchdog timer
    reg.watchdog = timer;

    // Write the register back
    return charger_write_register(CHARGE_TERM_TIMER_CTRL, reg.raw);
}

esp_err_t charger_set_otg_mode(bool enable) {
    // Set OTG pin state
    ESP_RETURN_ON_ERROR(gpio_set_level(BQ24296M_OTG_PIN, enable), TAG, "Failed to set OTG pin state");

    // Get current power-on configuration
    charger_power_on_config_t reg;
    ESP_RETURN_ON_ERROR(charger_read_register(POWER_ON_CONFIG, &reg.raw), TAG, "Failed to read power-on configuration");

    // Set the OTG mode and CHG bits
    reg.otg_config = enable;
    reg.chg_config = !enable;

    // Write the register back
    ESP_RETURN_ON_ERROR(charger_write_register(POWER_ON_CONFIG, reg.raw), TAG, "Failed to set OTG mode");

    return ESP_OK;
}

esp_err_t charger_read_status(charger_system_status_t *status) {
    ESP_RETURN_ON_ERROR(charger_read_register(SYSTEM_STATUS, &status->raw), TAG, "Failed to read status register");
    return ESP_OK;
}

esp_err_t charger_read_faults(charger_new_fault_t *fault) {
    ESP_RETURN_ON_ERROR(charger_read_register(NEW_FAULT, &fault->raw), TAG, "Failed to read fault register");
    return ESP_OK;
}

esp_err_t charger_read_vendor(charger_vendor_status_t *vendor) {
    ESP_RETURN_ON_ERROR(charger_read_register(VENDOR_STATUS, &vendor->raw), TAG, "Failed to read vendor register");
    return ESP_OK;
}

charger_system_status_t charger_get_status() {
    return current_status;
}

charger_new_fault_t charger_get_faults() {
    return current_fault;
}
