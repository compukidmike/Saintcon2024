#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

// Define event bits for power management
#define POWER_EVENT_BATTERY_UPDATE   BIT0
#define POWER_EVENT_TYPEC_UPDATE     BIT1
#define POWER_EVENT_CHARGER_UPDATE   BIT2
#define POWER_EVENT_LOAD_SWITCH_FLAG BIT3

// Global event group for power management
extern EventGroupHandle_t power_event_group;

// Initialize the power management component
esp_err_t power_manager_init();

#ifdef __cplusplus
}
#endif
