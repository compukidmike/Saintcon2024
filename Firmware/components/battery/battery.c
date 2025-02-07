#include <math.h>
#include "battery.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"
#include "esp_check.h"
#include "charger.h"

static const char *TAG = "battery";

#ifdef CONFIG_BADGE_HW_VERSION_2
    #define BATTERY_ADC_UNIT    ADC_UNIT_1    // GPIO 10 (ADC1_CH8)
    #define BATTERY_ADC_CHANNEL ADC_CHANNEL_8 // GPIO 10 (ADC1_CH8)
#elif CONFIG_BADGE_HW_VERSION_3
    #define BATTERY_ADC_UNIT    ADC_UNIT_1    // GPIO 10 (ADC1_CH9)
    #define BATTERY_ADC_CHANNEL ADC_CHANNEL_9 // GPIO 10 (ADC1_CH9)
#endif
#define BATTERY_ATTEN       ADC_ATTEN_DB_12                        // ADC attenuation value - 12 dB = 0mV ~ 3100mV
#define BATTERY_ADC_VREF    1100                                   // 1100 mV for ESP32-S3 (but can range from 1000 to 1200 mV)
#define BATTERY_ADC_MAX     ((1 << SOC_ADC_DIGI_MAX_BITWIDTH) - 1) // 4095 for 12-bit ADC (ESP32-S3)
#define BATTERY_VOLTAGE_MIN 3500                                   // 3500 mV
#define BATTERY_VOLTAGE_MAX 4200                                   // 4200 mV
#define BATTERY_VDIV_R1     100000                                 // 100 kΩ
#define BATTERY_VDIV_R2     100000                                 // 100 kΩ
#define BATTERY_UPDATE_MS   60 * 1000                              // Every minute
// #define BATTERY_UPDATE_MS   10 * 1000                              // Every 10 seconds

// Calibration factor: multiplier to adjust for measured voltage difference. These are just a series of manual measurements I did.
// clang-format off
#define BATTERY_CAL_FACTOR \
    ( \
        1.01539626451 + \
        1.01443768997 + \
        1.01495184997 + \
        1.01599796851 + \
        1.01448906965   \
    ) / 5
// clang-format on

// Event group for battery updates
EventGroupHandle_t battery_event_group = NULL;

static adc_oneshot_unit_handle_t adc_handle = NULL;
static bool adc_calibrated                  = false;
static adc_cali_handle_t cali_handle        = NULL;
static battery_status_t battery_status      = {0};

StackType_t *stack_mem   = NULL;
StaticTask_t *task_mem   = NULL;
TaskHandle_t task_handle = NULL;

/**
 * @brief Initialize the ADC calibration scheme.
 *      The ESP32-S3 supports curve fitting calibration so we'll use that. Mostly copied from the ESP-IDF example at
 * https://github.com/espressif/esp-idf/blob/v5.2.1/examples/peripherals/adc/oneshot_read/main/oneshot_read_main.c
 *
 * @param unit
 * @param channel
 * @param atten
 * @param out_handle
 * @return true/false whether calibration was successful
 */
static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle) {
    adc_cali_handle_t handle = NULL;
    esp_err_t ret            = ESP_FAIL;
    bool calibrated          = false;

    if (!calibrated) {
        ESP_LOGI(TAG, "ADC calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id  = unit,
            .chan     = channel,
            .atten    = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

/**
 * @brief Deregister ADC calibration scheme
 *
 * @param handle
 */
static esp_err_t adc_calibration_deinit(adc_cali_handle_t handle) {
    ESP_LOGI(TAG, "Deregister Curve Fitting calibration scheme");
    return adc_cali_delete_scheme_curve_fitting(handle);
}

/**
 * @brief Task to read the battery voltage and update the status
 */
static void battery_read_task(void *_arg) {
    (void)_arg; // Unused - suppress lint warning

    int adc_raw;
    int voltage;

    while (1) {
        // Temporarily turn off charging to get an accurate reading (if currently charging)
        ESP_LOGD(TAG, "Checking to see if we need to turn off charging");
        bool was_charging                     = false;
        charger_system_status_t charge_status = charger_get_status();
        if (charge_status.raw == 0                                // Haven't done the initial status read yet
            || charge_status.chrg_stat == CHRG_STAT_PRE_CHARGING  // Pre-charging counts as charging
            || charge_status.chrg_stat == CHRG_STAT_FAST_CHARGING // Fast charging
        ) {
            was_charging = true;
            ESP_LOGD(TAG, "Turning off charging");
            charger_set_charge_enable(false);
        }

        // Read the battery voltage
        esp_err_t ret = adc_oneshot_read(adc_handle, BATTERY_ADC_CHANNEL, &adc_raw);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read battery voltage: %s", esp_err_to_name(ret));
            continue;
        }
        ESP_LOGD(TAG, "ADC%d Channel[%d] Raw Data: %d", BATTERY_ADC_UNIT + 1, BATTERY_ADC_CHANNEL, adc_raw);

        // Turn the charger back on if needed
        if (was_charging) {
            ESP_LOGD(TAG, "Turning charging back on");
            charger_set_charge_enable(true);
        }

        if (adc_calibrated) {
            // Apply calibration
            ret = adc_cali_raw_to_voltage(cali_handle, adc_raw, &voltage);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to apply calibration: %s", esp_err_to_name(ret));
                continue;
            }
            ESP_LOGD(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", BATTERY_ADC_UNIT + 1, BATTERY_ADC_CHANNEL, voltage);
        } else {
            // Convert raw ADC value to voltage in mV
            voltage = (adc_raw * BATTERY_ADC_VREF) / BATTERY_ADC_MAX;
            ESP_LOGD(TAG, "ADC%d Channel[%d] Voltage: %d mV", BATTERY_ADC_UNIT + 1, BATTERY_ADC_CHANNEL, voltage);
        }

        // Adjust for voltage divider
        if (BATTERY_VDIV_R1 + BATTERY_VDIV_R2 > 0) {
            voltage = (voltage * (BATTERY_VDIV_R1 + BATTERY_VDIV_R2)) / BATTERY_VDIV_R2;
        }

        // Apply calibration factor
        voltage = (int)(voltage * BATTERY_CAL_FACTOR);

        // Update the battery status
        battery_status.voltage = voltage;
        if (voltage >= BATTERY_VOLTAGE_MAX) {
            battery_status.percentage = 100;
        } else if (voltage <= BATTERY_VOLTAGE_MIN) {
            battery_status.percentage = 0;
        } else {
            float v = (float)voltage / 1000.0f;
            // Copied curve fitting polynomial from
            // https://github.com/G6EJD/LiPo_Battery_Capacity_Estimator/blob/master/ReadBatteryCapacity_LIPO.ino
            battery_status.percentage = (uint8_t)(2808.3808 * pow(v, 4) - 43560.9157 * pow(v, 3) + 252848.5888 * pow(v, 2) -
                                                  650767.4615 * v + 626532.5703);
        }
        if (battery_status.percentage < 5) {
            battery_status.level = BATTERY_LEVEL_EMPTY;
        } else if (battery_status.percentage < 25) {
            battery_status.level = BATTERY_LEVEL_1;
        } else if (battery_status.percentage < 50) {
            battery_status.level = BATTERY_LEVEL_2;
        } else if (battery_status.percentage < 75) {
            battery_status.level = BATTERY_LEVEL_3;
        } else {
            battery_status.level = BATTERY_LEVEL_FULL;
        }

        // Notify the event group
        ESP_LOGD(TAG, "Battery status -- voltage: %d mV, percentage: %d%%, level: %s", battery_status.voltage,
                 battery_status.percentage,
                 battery_status.level == BATTERY_LEVEL_EMPTY ? "Empty"
                 : battery_status.level == BATTERY_LEVEL_1   ? "1"
                 : battery_status.level == BATTERY_LEVEL_2   ? "2"
                 : battery_status.level == BATTERY_LEVEL_3   ? "3"
                                                             : "Full");
        ESP_LOGD(TAG, "Sending battery update event");
        xEventGroupSetBits(battery_event_group, BATTERY_UPDATE);

        // Sleep until the next update
        vTaskDelay(pdMS_TO_TICKS(BATTERY_UPDATE_MS));
    }
}

esp_err_t battery_init() {
    // ADC unit init
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = BATTERY_ADC_UNIT,
    };
    ESP_RETURN_ON_ERROR(adc_oneshot_new_unit(&init_config, &adc_handle), TAG, "Failed to create ADC unit");

    // ADC calibration init
    adc_calibrated = adc_calibration_init(BATTERY_ADC_UNIT, BATTERY_ADC_CHANNEL, BATTERY_ATTEN, &cali_handle);

    // ADC channel config
    adc_oneshot_chan_cfg_t chan_config = {
        .atten    = BATTERY_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_RETURN_ON_ERROR(adc_oneshot_config_channel(adc_handle, BATTERY_ADC_CHANNEL, &chan_config), TAG,
                        "Failed to configure ADC channel");

    // Create the event group for battery updates
    battery_event_group = xEventGroupCreate();
    if (battery_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create battery event group");
        return ESP_FAIL;
    }

    // Create the task to read the battery voltage with stack allocated in SPIRAM
    uint32_t stack_size = 4096;
    stack_mem           = (StackType_t *)heap_caps_malloc(stack_size * sizeof(StackType_t), MALLOC_CAP_SPIRAM);
    if (stack_mem == NULL) {
        ESP_LOGE(TAG, "Failed to allocate stack memory");
        return ESP_FAIL;
    }
    task_mem = (StaticTask_t *)heap_caps_malloc(sizeof(StaticTask_t), MALLOC_CAP_INTERNAL);
    if (task_mem == NULL) {
        ESP_LOGE(TAG, "Failed to allocate task memory");
        heap_caps_free(stack_mem);
        return ESP_FAIL;
    }
    ESP_LOGD(TAG, "Creating battery read task");
    task_handle = xTaskCreateStatic(battery_read_task, "battery_read_task", stack_size, NULL, 4, stack_mem, task_mem);

    return ESP_OK;
}

esp_err_t battery_deinit() {
    // Stop the task
    if (task_handle != NULL) {
        vTaskDelete(task_handle);
        task_handle = NULL;
    }

    // Free the allocated memory for the task
    if (stack_mem != NULL) {
        heap_caps_free(stack_mem);
        stack_mem = NULL;
    }
    if (task_mem != NULL) {
        heap_caps_free(task_mem);
        task_mem = NULL;
    }

    // Deregister the calibration scheme
    return adc_calibration_deinit(cali_handle);
}

battery_status_t battery_get_status() {
    return battery_status;
}
