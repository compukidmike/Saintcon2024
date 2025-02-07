#include "power_manager.h"
#include "battery.h"
#include "charger.h"
#include "type_c.h"
#include "load_switch.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "string.h"
#include "ui.h"

static const char *TAG = "power_manager";

int log_tasks() {
    // Get the number of tasks
    uint32_t total_tasks = uxTaskGetNumberOfTasks();
    ESP_LOGI(TAG, "Total tasks: %lu", total_tasks);

    // Get the task status
    TaskStatus_t *task_status_array = NULL;
    uint32_t total_runtime          = 0;
    task_status_array               = pvPortMalloc(total_tasks * sizeof(TaskStatus_t));
    if (task_status_array == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for task status array");
    } else {
        total_tasks = uxTaskGetSystemState(task_status_array, total_tasks, &total_runtime);
        for (uint32_t i = 0; i < total_tasks; i++) {
            char core_id[2] = {0};
            core_id[0]      = (task_status_array[i].xCoreID == tskNO_AFFINITY ? 'N' : '0' + task_status_array[i].xCoreID);
            ESP_LOGI(TAG, "Task [%s] %s: %lu bytes", core_id, task_status_array[i].pcTaskName,
                     task_status_array[i].usStackHighWaterMark);
        }
        size_t free_heap_size        = esp_get_free_heap_size();
        size_t min_free_heap_size    = esp_get_minimum_free_heap_size();
        size_t free_internal_mem     = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        size_t free_psram_mem        = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
        size_t min_free_internal_mem = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL);
        size_t min_free_psram_mem    = heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM);
        ESP_LOGI(TAG, "----------------------------------------");
        ESP_LOGI(TAG, "Free heap: %u, Minimum free heap since boot: %u", free_heap_size, min_free_heap_size);
        ESP_LOGI(TAG, "Free internal memory: %u, Free PSRAM memory: %u", free_internal_mem, free_psram_mem);
        ESP_LOGI(TAG, "Minimum free internal memory: %u, Minimum free PSRAM memory: %u", min_free_internal_mem,
                 min_free_psram_mem);
        free(task_status_array);
    }

    return 0;
}

// ----------------------------------------------------------------------------
// Ring buffer logging - to capture some things while disconnected
//

#define LOG_BUFFER_SIZE        2048
#define MAX_LOG_MESSAGE_LENGTH 256

static char log_buffer[LOG_BUFFER_SIZE];
static int log_buffer_head                = 0;
static int log_buffer_tail                = 0;
static SemaphoreHandle_t log_buffer_mutex = NULL;

static void log_buffer_init() {
    log_buffer_mutex = xSemaphoreCreateMutex();
}

void log_buffer_add(const char *message) {
    xSemaphoreTake(log_buffer_mutex, portMAX_DELAY);
    int len = strlen(message);
    if (len >= MAX_LOG_MESSAGE_LENGTH) {
        len = MAX_LOG_MESSAGE_LENGTH - 1;
    }

    if ((log_buffer_head + len + 1) % LOG_BUFFER_SIZE != log_buffer_tail) {
        memcpy(&log_buffer[log_buffer_head], message, len);
        log_buffer[log_buffer_head + len] = '\0';
        log_buffer_head                   = (log_buffer_head + len + 1) % LOG_BUFFER_SIZE;
    }
    xSemaphoreGive(log_buffer_mutex);
}

void log_buffer_flush() {
    xSemaphoreTake(log_buffer_mutex, portMAX_DELAY);
    while (log_buffer_tail != log_buffer_head) {
        printf("%s\n", &log_buffer[log_buffer_tail]);
        log_buffer_tail = (log_buffer_tail + strlen(&log_buffer[log_buffer_tail]) + 1) % LOG_BUFFER_SIZE;
    }
    xSemaphoreGive(log_buffer_mutex);
}

//
// ----------------------------------------------------------------------------

#define CHARGER_WATCHDOG_TIMEOUT WATCHDOG_80s
#define CHARGER_WATCHDOG_PERIOD  pdMS_TO_TICKS(60000) // Check the charger status every 60 seconds

EventGroupHandle_t power_event_group          = NULL;
static TimerHandle_t charger_watchdog_timer   = NULL;
static TimerHandle_t enable_check_timer       = NULL;
static TaskHandle_t power_manager_task_handle = NULL;

// Function prototypes
static void charger_event_task(void *_arg);
static void battery_event_task(void *_arg);
static void typec_event_task(void *_arg);
static void load_switch_event_task(void *_arg);
static void power_manager_task(void *_arg);
static void handle_charger_update();
static void handle_battery_update();
static void handle_typec_update();
static void charger_watchdog_timer_callback(TimerHandle_t xTimer);
static void enable_check_timer_callback(TimerHandle_t xTimer);

typedef struct {
    charger_system_status_t charger_status;
    battery_status_t battery_status;
    charger_new_fault_t charger_fault;
    typec_status_t typec_status;
} power_manager_state_t;

static power_manager_state_t power_state = {0};

// Load switch flag tracking
#define LOAD_SWITCH_FLAG_TIMEOUT_US 5000000 // 5 seconds
static int64_t load_switch_flag = 0;        // Track the last time the load switch flag was detected

// Charger fault status tracking
#define OTG_FAULT_TIMEOUT_US     60 * 1000000 // 60 seconds
#define THERMAL_FAULT_TIMEOUT_US 60 * 1000000 // 60 seconds
#define BAT_FAULT_TIMEOUT_US     60 * 1000000 // 60 seconds
static int64_t charger_bad_otg   = {0};       // Track the last concerning OTG fault
static int64_t charger_bad_therm = {0};       // Track the last concerning thermal fault
static int64_t charger_bad_bat   = {0};       // Track the last concerning BAT fault

esp_err_t power_manager_init() {
    // Initialize the log buffer
    log_buffer_init();

    // Create event group for power management
    power_event_group = xEventGroupCreate();

    // Initialize charger, battery, and Type-C components
    esp_err_t err;
    ESP_LOGD(TAG, "Initializing charger component");
    err = charger_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize charger: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGD(TAG, "Initializing battery component");
    err = battery_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize battery: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGD(TAG, "Initializing Type-C component");
    err = typec_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Type-C: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGD(TAG, "Initializing load switch component");
    err = load_switch_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize load switch: %s", esp_err_to_name(err));
        return err;
    }
    load_switch_enable();

    // Create a timer for periodic charger watchdog resets
    charger_set_watchdog_timer(CHARGER_WATCHDOG_TIMEOUT);
    charger_watchdog_timer =
        xTimerCreate("charger_watchdog_timer", CHARGER_WATCHDOG_PERIOD, pdTRUE, NULL, charger_watchdog_timer_callback);
    if (charger_watchdog_timer == NULL) {
        ESP_LOGE(TAG, "Failed to create charger watchdog timer");
        return ESP_FAIL;
    }
    xTimerStart(charger_watchdog_timer, 0);

    // Create a timer for an enable check - see if things need to be re-enabled after fault conditions
    enable_check_timer = xTimerCreate("enable_check_timer", pdMS_TO_TICKS(1000), pdTRUE, NULL, enable_check_timer_callback);
    if (enable_check_timer == NULL) {
        ESP_LOGE(TAG, "Failed to create enable check timer");
        return ESP_FAIL;
    }
    xTimerStart(enable_check_timer, 0);

    // Set Type-C options
    ESP_LOGD(TAG, "Setting Type-C options");
    typec_set_role(TYPEC_ROLE_DUAL);
    typec_set_accessory_detection(true);

    // Create power management task
    ESP_LOGD(TAG, "Creating power management task");
    xTaskCreate(power_manager_task, "power_manager_task", 3072, NULL, 10, &power_manager_task_handle);

    // Create tasks for Type-C and charger events
    ESP_LOGD(TAG, "Creating battery, Type-C, and charger event tasks");
    xTaskCreate(charger_event_task, "charger_event_task", 2272, NULL, 10, NULL);
    xTaskCreate(battery_event_task, "battery_event_task", 2272, NULL, 5, NULL);
    xTaskCreate(load_switch_event_task, "load_switch_event_task", 2272, NULL, 5, NULL);
    xTaskCreate(typec_event_task, "typec_event_task", 2192, NULL, 10, NULL);

    return ESP_OK;
}

static void log_task_memory_info() {
    ESP_LOGI(TAG,
             "\n"
             "Memory info for task %s\n"
             "  -- Free heap: %lu bytes\n"
             "  -- Minimum free heap: %lu bytes\n"
             "  -- Free stack: %d bytes\n"
             "  -- Free internal memory: %d bytes\n"
             "  -- Free external memory: %d bytes",
             pcTaskGetName(NULL),                                     //
             esp_get_free_heap_size(),                                //
             esp_get_minimum_free_heap_size(),                        //
             uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t), //
             heap_caps_get_free_size(MALLOC_CAP_INTERNAL),            //
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM)               //
    );
}

static char *str_charger_status(charger_system_status_t *status) {
    static char status_message[256];
    snprintf(status_message, sizeof(status_message),
             "Charger Status:\n"
             "  -- VSYS: %s\n"
             "  -- Thermal: %s\n"
             "  -- Power Good: %s\n"
             "  -- DPM: %s\n"
             "  -- Charge Status: %s\n"
             "  -- VBUS Status: %s\n",
             status->vsys_stat ? "BAT < VSYSMIN" : "BAT > VSYSMIN", //
             status->therm_stat ? "Thermal Regulation" : "Normal",  //
             status->pg_stat ? "Good" : "Not Good",                 //
             status->dpm_stat ? "VINDPM or IINDPM" : "Not DPM",     //
             (status->chrg_stat & 0x03) == 0   ? "Not Charging"
             : (status->chrg_stat & 0x03) == 1 ? "Pre-Charge"
             : (status->chrg_stat & 0x03) == 2 ? "Fast Charging"
                                               : "Charge Done",
             (status->vbus_stat & 0x03) == 0   ? "Unknown"
             : (status->vbus_stat & 0x03) == 1 ? "USB Host"
             : (status->vbus_stat & 0x03) == 2 ? "Adapter"
                                               : "OTG" //
    );
    return status_message;
}

static char *str_typec_status(typec_status_t *status) {
    static char status_message[256];
    snprintf(status_message, sizeof(status_message),
             "Type-C Status:\n"
             "  -- VBUS Detected: %s\n"
             "  -- Charging Current: %s\n"
             "  -- Port Status: %s\n"
             "  -- Plug Polarity: %s",
             status->vbus_detected ? "Yes" : "No",
             status->charging_current == TYPEC_CHARGING_DEFAULT  ? "Default"
             : status->charging_current == TYPEC_CHARGING_MEDIUM ? "Medium"
             : status->charging_current == TYPEC_CHARGING_HIGH   ? "High"
                                                                 : "Standby",
             status->port_status == TYPEC_PORT_DEVICE   ? "Device"
             : status->port_status == TYPEC_PORT_HOST   ? "Host"
             : status->port_status == TYPEC_PORT_AUDIO  ? "Audio"
             : status->port_status == TYPEC_PORT_DEBUG  ? "Debug"
             : status->port_status == TYPEC_PORT_ACTIVE ? "Active"
                                                        : "Standby",
             status->plug_polarity == TYPEC_PLUG_CC1       ? "CC1"
             : status->plug_polarity == TYPEC_PLUG_CC2     ? "CC2"
             : status->plug_polarity == TYPEC_PLUG_UNKNOWN ? "Unknown"
                                                           : "Standby" //
    );
    return status_message;
}

static void charger_watchdog_timer_callback(TimerHandle_t xTimer) {
    (void)xTimer;
    xEventGroupSetBits(power_event_group, POWER_EVENT_CHARGER_UPDATE);
    ESP_LOGD(TAG, "Charger watchdog timer callback");
}

static void battery_event_task(void *_arg) {
    (void)_arg;
    for (;;) {
        // Wait for any battery-related events
        xEventGroupWaitBits(battery_event_group, BATTERY_UPDATE, pdTRUE, pdFALSE, portMAX_DELAY);
        ESP_LOGD(TAG, "Battery event detected");

        power_state.battery_status = battery_get_status();

        xEventGroupSetBits(power_event_group, POWER_EVENT_BATTERY_UPDATE);
    }
}

static void enable_check_timer_callback(TimerHandle_t xTimer) {
    (void)xTimer;
    // Check if the charger needs to be re-enabled
    if (charger_bad_therm - esp_timer_get_time() > THERMAL_FAULT_TIMEOUT_US) {
        charger_set_charge_enable(true);
    }

    // Check if the load switch needs to be re-enabled
    if (load_switch_flag != 0 && esp_timer_get_time() - load_switch_flag > LOAD_SWITCH_FLAG_TIMEOUT_US) {
        load_switch_enable();
        load_switch_flag = 0;
    }
}

static void typec_event_task(void *_arg) {
    (void)_arg;
    for (;;) {
        // Wait for any Type-C events
        EventBits_t bits =
            xEventGroupWaitBits(type_c_event_group, TYPE_C_EVENT_ID | TYPE_C_EVENT_INT, pdTRUE, pdFALSE, portMAX_DELAY);
        ESP_LOGD(TAG, "Type-C event detected");

        power_state.typec_status = typec_get_status();

        // TODO: Remove this code once all the testing is done
        char msg[48];
        snprintf(msg, sizeof(msg), "(%lu) Type-C Event: %s\n", esp_log_timestamp(), bits & TYPE_C_EVENT_ID ? "ID" : "INT");
        log_buffer_add(msg);
        log_buffer_add(str_typec_status(&power_state.typec_status));

        xEventGroupSetBits(power_event_group, POWER_EVENT_TYPEC_UPDATE);
    }
}

static void charger_event_task(void *_arg) {
    (void)_arg;
    for (;;) {
        // Wait for any charger-related events
        EventBits_t event_bits =
            xEventGroupWaitBits(charger_event_group, CHARGER_EVENT | CHARGER_FAULT, pdTRUE, pdFALSE, portMAX_DELAY);
        ESP_LOGD(TAG, "Charger event detected: %s", event_bits & CHARGER_EVENT ? "CHARGER_EVENT" : "CHARGER_FAULT");

        if (event_bits & CHARGER_EVENT) {
            power_state.charger_status = charger_get_status();
        }

        if (event_bits & CHARGER_FAULT) {
            power_state.charger_fault = charger_get_faults();
        }

        xEventGroupSetBits(power_event_group, POWER_EVENT_CHARGER_UPDATE);
    }
}

static void load_switch_event_task(void *_arg) {
    (void)_arg;
    for (;;) {
        // Wait for the load switch FLG pin
        xEventGroupWaitBits(i2c_switch_int, LOAD_SWITCH_INTERRUPT, pdTRUE, pdFALSE, portMAX_DELAY);

        // Disable the load switch and store the time so we can re-enable it after a delay
        load_switch_flag = esp_timer_get_time();
        load_switch_disable();

        // Set the event bit for the power manager
        xEventGroupSetBits(power_event_group, POWER_EVENT_LOAD_SWITCH_FLAG);
        log_task_memory_info();

        // Flush the log buffer. TODO: Remove this later after testing
        log_buffer_flush();
        log_tasks();
    }
}

static void power_manager_task(void *_arg) {
    (void)_arg;
    for (;;) {
        // Wait for any power-related events
        EventBits_t event_bits = xEventGroupWaitBits(power_event_group,
                                                     POWER_EVENT_BATTERY_UPDATE | POWER_EVENT_TYPEC_UPDATE |
                                                         POWER_EVENT_CHARGER_UPDATE | POWER_EVENT_LOAD_SWITCH_FLAG,
                                                     pdTRUE, pdFALSE, portMAX_DELAY);

        if (event_bits & POWER_EVENT_CHARGER_UPDATE) {
            handle_charger_update();
        }

        if (event_bits & POWER_EVENT_BATTERY_UPDATE) {
            handle_battery_update();
        }

        if (event_bits & POWER_EVENT_TYPEC_UPDATE) {
            handle_typec_update();
        }

        if (event_bits & POWER_EVENT_LOAD_SWITCH_FLAG) {
            ESP_LOGD(TAG, "Load switch FLG pin event detected");
        }
    }
}

static void handle_charger_update() {
    ESP_LOGD(TAG, "Handling charger update");
    // If the charger has a fault, handle it
    if (power_state.charger_fault.raw != 0) {
        ESP_LOGW(TAG, "Charger fault detected: 0x%02x", power_state.charger_fault.raw);

        // Track the last time a concerning fault was detected
        int64_t now = esp_timer_get_time();
        if (power_state.charger_fault.otg_fault) {
            charger_bad_otg = now;
        }
        if (power_state.charger_fault.chrg_fault & CHRG_FAULT_THERMAL) {
            charger_bad_therm = now;
        }
        if (power_state.charger_fault.bat_fault) {
            charger_bad_bat = now;
        }

        // // If there's an OTG fault, disable OTG mode
        // if (charger_bad_otg - now < OTG_FAULT_TIMEOUT_US) {
        //     ESP_LOGW(TAG, "OTG fault detected, disabling OTG mode");
        //     charger_set_otg_mode(false);
        // }

        // // If there is a thermal fault, we need to protect the system the best we can
        // if (charger_bad_therm - now < THERMAL_FAULT_TIMEOUT_US) {
        //     ESP_LOGW(TAG, "Thermal fault detected, turning charger off, disabling load switch output, and disabling "
        //                   "boost output to VBUS");
        //     if (power_state.charger_status.chrg_stat != CHRG_STAT_NOT_CHARGING) {
        //         charger_set_charge_enable(false);
        //     }
        //     if (load_switch_enabled()) {
        //         load_switch_disable();
        //     }
        //     if (power_state.charger_status.vbus_stat == VBUS_STAT_USB_OTG) {
        //         charger_set_otg_mode(false);
        //     }
        // }

        // // If there is a battery OVP fault, turn off the charger
        // if (charger_bad_bat - now < BAT_FAULT_TIMEOUT_US) {
        //     ESP_LOGW(TAG, "Battery OVP fault detected, turning charger off");
        //     if (power_state.charger_status.chrg_stat != CHRG_STAT_NOT_CHARGING) {
        //         charger_set_charge_enable(false);
        //     }
        // }
    }

    char msg[128];

    // If we've reached charge termination, stop charging
    if (power_state.charger_status.chrg_stat == CHRG_STAT_CHARGE_DONE) {
        snprintf(msg, sizeof(msg),
                 "(%lu) Charger Update:\n"
                 "Charger status: Charge Done",
                 esp_log_timestamp());
        charger_set_charge_enable(false);
    }
    // If the battery is not full, enable charging
    else if (power_state.battery_status.voltage < 4100) {
        if (charger_bad_therm - esp_timer_get_time() > THERMAL_FAULT_TIMEOUT_US &&
            charger_bad_bat - esp_timer_get_time() > BAT_FAULT_TIMEOUT_US) {
            snprintf(msg, sizeof(msg),
                     "(%lu) Charger Update:\n"
                     "Charger status: Enabling",
                     esp_log_timestamp());
        } else {
            snprintf(msg, sizeof(msg),
                     "(%lu) Charger Update:\n"
                     "Charger status: Not enabling due to fault",
                     esp_log_timestamp());
        }

        charger_set_charge_enable(true);
    }

    // Reset the watchdog timer
    charger_reset_watchdog();
    xTimerReset(charger_watchdog_timer, 0);

    // Update the UI with the new charger status
    set_status_battery_charging(power_state.charger_status.chrg_stat == CHRG_STAT_PRE_CHARGING ||
                                power_state.charger_status.chrg_stat == CHRG_STAT_FAST_CHARGING);
    set_status_power_connected(power_state.charger_status.pg_stat);
}

static void handle_battery_update() {
    ESP_LOGD(TAG, "Handling battery update");
    // Get the battery status
    battery_get_status(&power_state.battery_status);
    ESP_LOGI(TAG, "Battery Voltage: %d mV, Percentage: %d%%", power_state.battery_status.voltage,
             power_state.battery_status.percentage);

    // Ensure charging is enabled if the battery is not full enough
    if (power_state.battery_status.percentage < 95 && charger_bad_therm - esp_timer_get_time() > THERMAL_FAULT_TIMEOUT_US &&
        charger_bad_bat - esp_timer_get_time() > BAT_FAULT_TIMEOUT_US) {
        charger_set_charge_enable(true);
    }

    // Handle low battery condition
    if (power_state.battery_status.percentage < 5) {
        ESP_LOGW(TAG, "Battery level critical, taking necessary actions");
    }

    // Update the UI with the new battery status
    ESP_LOGD(TAG, "Setting battery level in the UI: %s",
             power_state.battery_status.level == BATTERY_LEVEL_EMPTY ? "Empty"
             : power_state.battery_status.level == BATTERY_LEVEL_1   ? "1"
             : power_state.battery_status.level == BATTERY_LEVEL_2   ? "2"
             : power_state.battery_status.level == BATTERY_LEVEL_3   ? "3"
                                                                     : "Full");
    set_status_battery_level(power_state.battery_status.level);
}

static void handle_typec_update() {
    ESP_LOGD(TAG, "Handling Type-C update");
    bool enable_otg = false;

    // Enable OTG if a device is connected and VBUS is not detected
    if ((power_state.typec_status.port_status == TYPEC_PORT_DEVICE ||
         power_state.typec_status.port_status == TYPEC_PORT_ACTIVE) &&
        !power_state.typec_status.vbus_detected) {
        enable_otg = true;
    } else {
        enable_otg = false;
    }

    // Buffer the enable_otg flag, charger and Type-C status
    char msg[128];
    snprintf(msg, sizeof(msg),
             "(%lu) Type-C Update:\n"
             "Setting OTG mode: %s",
             esp_log_timestamp(), enable_otg ? "Enabled" : "Disabled");
    log_buffer_add(msg);
    log_buffer_add(str_charger_status(&power_state.charger_status));
    log_buffer_add(str_typec_status(&power_state.typec_status));

    // // Don't enable OTG if the charger has an OTG or thermal fault
    // if (enable_otg && (charger_bad_otg - esp_timer_get_time() < OTG_FAULT_TIMEOUT_US ||
    //                    charger_bad_therm - esp_timer_get_time() < THERMAL_FAULT_TIMEOUT_US)) {
    //     ESP_LOGD(TAG, "Type-C: Not enabling OTG due to charger fault");
    //     log_buffer_add("Type-C: Not enabling OTG due to charger fault");
    // } else {
    //     // Set OTG mode
    //     charger_set_otg_mode(enable_otg);
    // }

    charger_set_otg_mode(enable_otg);
}
