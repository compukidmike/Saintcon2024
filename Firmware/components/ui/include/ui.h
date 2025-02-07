#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "esp_err.h"
#include "lvgl.h"

#include "badge.h"
#include "battery.h"
#include "wifi_manager.h"

// Compmonent headers to expose externally
#include "../content.h"
#include "../onboarding.h"
#include "../pages/levelup.h"
#include "../pages/tower_battle.h"

typedef enum {
    SCREEN_NONE,   // No screen
    SCREEN_SPLASH, // The splash screen
    SCREEN_MAIN,   // The main screen
    SCREEN_UPDATE, // The OTA firmware update screen
    SCREEN_HWTEST, // The hardware test screen
} ui_screen_t;

typedef struct {
    wifi_status_t wifi_state;      // We only have a single wifi icon built in... we'll just change the color
    bool battery_charging;         // Whether to show the charging icon
    battery_level_t battery_level; // The battery level to display (0 - empty, 1, 2, 3, 4 - full)
    bool power_connected;          // Whether to show the power icon
    bool usb_connected;            // Whether to show the USB icon (someone has connected something to the USB port)
    char label[64];                // The label to display on the status bar
    uint8_t minibadge_count;       // The number of minibadges to display
    uint8_t alert_count;           // The number of alerts to display
    ui_screen_t screen;            // The current screen to display
} ui_state_t;

// Status labels for each screen type
extern const char *screen_labels[];

void ui_init();
bool ui_ready();
void update_screen(ui_screen_t screen);

void show_msgbox(const char *text);

// Status bar
void set_status_wifi_state(wifi_status_t state);
void set_status_battery_charging(bool charging);
void set_status_power_connected(bool connected);
void set_status_battery_level(battery_level_t level);
void set_status_usb_connected(bool connected);
void set_status_label(const char *label);
void set_status_minibadge_update();
void set_status_alert_count(uint8_t count);

// Content
void set_screen(ui_screen_t screen);
ui_screen_t get_screen();

// OTA update
void set_ota_state(ota_state_t state);

#ifdef __cplusplus
}
#endif
