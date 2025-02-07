#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "ui.h"

/**
 * @brief UI event types
 */
#define UI_EVENT_TYPE_LIST  \
    X(NONE)                 \
    X(SET_WIFI_STATE)       \
    X(SET_BATTERY_CHARGING) \
    X(SET_BATTERY_LEVEL)    \
    X(SET_POWER_CONNECTED)  \
    X(SET_USB_CONNECTED)    \
    X(SET_LABEL)            \
    X(SET_MINIBADGE_UPDATE) \
    X(SET_ALERT_COUNT)      \
    X(SET_SCREEN)           \
    X(OTA_STATE)
#undef X
typedef enum {
#define X(val) UI_EVENT_##val,
    UI_EVENT_TYPE_LIST
#undef X
} ui_event_type_t;
extern const char *ui_event_type_map[];
ui_event_type_t get_ui_event_type(const char *type_str);

// UI event data
typedef struct {
    ui_event_type_t type;
    union {
        wifi_status_t wifi_state;
        bool battery_charging;
        battery_level_t battery_level;
        bool power_connected;
        bool usb_connected;
        char label[64];
        uint8_t minibadge_count;
        uint8_t alert_count;
        ui_screen_t screen;
        ota_state_t ota_state;
    } data; // NOTE: This should pretty much stay in sync with ui_state_t
} __attribute__((aligned(4))) ui_event_t;

#ifdef __cplusplus
}
#endif
