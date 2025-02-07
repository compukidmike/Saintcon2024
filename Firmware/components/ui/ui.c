#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include "esp_mac.h"
#include "esp_log.h"
#include "esp_timer.h"

// Main UI headers
#include "ui.h"
#include "ui_events.h"
#include "theme.h"

// Component dependencies
#include "api.h"
#include "badge.h"
#include "battery.h"
#include "charger.h"
#include "display.h"
#include "minibadge.h"

// UI screens
#include "screens/hwtest.h"
#include "screens/main.h"
#include "screens/splash.h"
#include "screens/update.h"

// UI main content screens
#include "content.h"
#include "loadanim.h"
#include "statusbar.h"

static const char *TAG = "ui";

const char *screen_labels[] = {
    "None", "Splash", "Main", "Update", "HW Test",
};

#define UI_EVENT_QUEUE_SIZE     30
#define UI_SCREEN_FADE_DURATION 500

static bool ui_initialized          = false;
static ui_state_t state             = {0};
static ui_screen_t screen_trans_to  = SCREEN_NONE; // While transitioning to a new screen
static ui_screen_t screen_next      = SCREEN_NONE; // The next screen to transition to if we're currently transitioning
static QueueHandle_t ui_event_queue = NULL;
static TaskHandle_t ui_task_handle  = NULL;

// Function prototypes
static void ui_task(void *_arg);
static void render_main();

bool ui_ready() {
    return ui_initialized;
}

void ui_init() {
    if (ui_initialized) {
        ESP_LOGW(TAG, "UI already initialized");
        return;
    }

    // Initialize LVGL plugins and features
    lv_bin_decoder_init();
    lv_lodepng_init();
    lv_fs_stdio_init();
    lv_fs_posix_init();

    // Initialize styles
    style_init();

    // Intialize the event queue
    if (ui_event_queue == NULL && (ui_event_queue = xQueueCreate(UI_EVENT_QUEUE_SIZE, sizeof(ui_event_t))) == NULL) {
        ESP_LOGE(TAG, "Failed to create UI event queue");
        return;
    }

    // Try to get current battery and charging status
    battery_status_t battery_status        = battery_get_status();
    charger_system_status_t charger_status = charger_get_status();
    state.battery_level                    = battery_status.level;
    state.battery_charging =
        charger_status.chrg_stat == CHRG_STAT_PRE_CHARGING || charger_status.chrg_stat == CHRG_STAT_FAST_CHARGING;
    state.power_connected = charger_status.pg_stat;

    // Create the UI task
    if (xTaskCreate(ui_task, "ui_task", 8192, NULL, 6, &ui_task_handle) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create UI task");
        return;
    }

    // Render the splash screen initially
    update_screen(SCREEN_SPLASH);

    ui_initialized = true;
}

static void screen_event_cb(lv_event_t *e) {
    // Make sure it's an event we care about
    const lv_event_code_t handle_codes[] = {
        LV_EVENT_SCREEN_UNLOAD_START,
        LV_EVENT_SCREEN_LOAD_START,
        LV_EVENT_SCREEN_LOADED,
        LV_EVENT_SCREEN_UNLOADED,
    };
    lv_event_code_t code = lv_event_get_code(e);
    bool handling        = false;
    for (int i = 0; i < sizeof(handle_codes) / sizeof(handle_codes[0]); i++) {
        if (code == handle_codes[i]) {
            handling = true;
            break;
        }
    }
    if (!handling) {
        return;
    }

    // Get the screen and user data
    lv_obj_t *screen        = lv_event_get_target(e);
    ui_screen_t screen_type = SCREEN_NONE;
    void *data              = lv_obj_get_user_data(screen);
    if (data == NULL) {
        ESP_LOGW(TAG, "No user data provided for screen event: %d", code);
        // if (code == LV_EVENT_SCREEN_LOAD_START || code == LV_EVENT_SCREEN_LOADED) {
        //     ESP_LOGD(TAG, "... setting screen type to: %s", screen_labels[screen_trans_to]);
        //     screen_type = screen_trans_to;
        // } else {
        //     ESP_LOGD(TAG, "... setting screen type to current state: %s", screen_labels[state.screen]);
        //     screen_type = state.screen;
        // }
    } else {
        screen_type = *(ui_screen_t *)data;
        ESP_LOGD(TAG, "Screen type: %d", screen_type);
    }

    if (code == LV_EVENT_SCREEN_LOAD_START) {
        ESP_LOGD(TAG, "Screen load start: %p, type: %s", screen, screen_labels[screen_type]);
        screen_trans_to = screen_type;
    } else if (code == LV_EVENT_SCREEN_LOADED) {
        ESP_LOGD(TAG, "Screen loaded: %p, type: %s", screen, screen_labels[screen_type]);

        // If we're transitioning to a new screen, update the state
        if (screen_trans_to != SCREEN_NONE) {
            ESP_LOGD(TAG, "Setting screen state to: %s", screen_labels[screen_type]);
            state.screen    = screen_type;
            screen_trans_to = SCREEN_NONE;
        }

        // If we're transitioning to a new screen, do it now
        if (screen_next != SCREEN_NONE) {
            ESP_LOGD(TAG, "Transitioning to next screen: %s", screen_labels[screen_next]);
            set_screen(screen_next);
            screen_next = SCREEN_NONE;
        }
    } else if (code == LV_EVENT_SCREEN_UNLOAD_START) {
        ESP_LOGD(TAG, "Screen unload start: %p, type: %s", screen, screen_labels[screen_type]);
        switch (screen_type) {
            case SCREEN_SPLASH: //
                splash_screen_shutdown();
                break;
            case SCREEN_MAIN:
                status_bar_shutdown(); // TODO: Make a per-screen shutdown function and move this there
                break;
            default: //
                ESP_LOGD(TAG, "No specific cleanup required for screen type: %s", screen_labels[screen_type]);
                break;
        }
    } else if (code == LV_EVENT_SCREEN_UNLOADED) {
        ESP_LOGD(TAG, "Screen unloaded: %p, type: %s", screen, screen_labels[screen_type]);
        if (state.screen == SCREEN_MAIN && screen_trans_to == SCREEN_NONE) {
            lv_async_call(render_main, NULL);
            vTaskDelay(1);
        }
    }
}

void update_screen(ui_screen_t new_screen) {
    if (screen_trans_to != SCREEN_NONE) {
        if (screen_trans_to != new_screen) {
            ESP_LOGW(TAG, "Already transitioning to screen: %s ... queueing %s for next screen change",
                     screen_labels[screen_trans_to], screen_labels[new_screen]);
            screen_next = new_screen;
        } else {
            ESP_LOGW(TAG, "Already transitioning to screen: %s ... ignoring duplicate request", screen_labels[new_screen]);
        }
        return;
    }

    if (lvgl_lock(portMAX_DELAY, __FILE__, __LINE__)) {
        lv_obj_t *screen                = NULL;
        lv_screen_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;

        switch (new_screen /*  == SCREEN_NONE ? state.screen : new_screen */) {
            case SCREEN_SPLASH:
                screen    = create_splash_screen();
                anim_type = LV_SCR_LOAD_ANIM_FADE_IN;
                break;
            case SCREEN_UPDATE:
                screen    = create_update_screen();
                anim_type = LV_SCR_LOAD_ANIM_MOVE_LEFT;
                break;
            case SCREEN_MAIN:
                screen    = create_main_screen();
                anim_type = state.screen == SCREEN_UPDATE   ? LV_SCR_LOAD_ANIM_MOVE_RIGHT
                            : state.screen == SCREEN_HWTEST ? LV_SCR_LOAD_ANIM_MOVE_LEFT
                                                            : LV_SCR_LOAD_ANIM_FADE_IN;
                break;
            case SCREEN_HWTEST:
                screen    = create_hwtest_screen();
                anim_type = LV_SCR_LOAD_ANIM_MOVE_RIGHT;
                break;
            default: //
                ESP_LOGW(TAG, "Unknown screen type: %d", new_screen);
                break;
        }

        // Load the new screen with an animation and event callback
        if (screen != NULL) {
            ESP_LOGD(TAG, "Loading screen: %s", screen_labels[new_screen]);
            lv_obj_add_event_cb(screen, screen_event_cb, LV_EVENT_ALL, NULL);
            // lv_screen_load_anim(screen, anim_type, UI_SCREEN_FADE_DURATION, 100, true);
            lv_screen_load_anim(screen, anim_type, 0, 0, true);
            ESP_LOGD(TAG, "Screen load initiated: %s", screen_labels[new_screen]);
        }

        lvgl_unlock(__FILE__, __LINE__);
    }
}

void enqueue_ui_event(ui_event_t *event) {
    if (!ui_initialized) {
        ESP_LOGW(TAG, "UI not initialized ... not enqueuing event: %s", ui_event_type_map[event->type]);
        return;
    } else {
        ESP_LOGD(TAG, "Enqueuing UI event: %s", ui_event_type_map[event->type]);
    }

    // Ensure the event pointer is 4-byte aligned
    assert(((uintptr_t)event & 0x3) == 0);

    if (ui_event_queue != NULL) {
        if (xQueueSend(ui_event_queue, event, pdMS_TO_TICKS(100)) != pdTRUE) {
            ESP_LOGW(TAG, "Failed to enqueue UI event: %d", event->type);
        }
    } else {
        ESP_LOGW(TAG, "UI event queue not initialized");
    }
}

void set_status_wifi_state(wifi_status_t state) {
    ui_event_t event = {
        .type            = UI_EVENT_SET_WIFI_STATE,
        .data.wifi_state = state,
    };
    enqueue_ui_event(&event);
}

void set_status_battery_charging(bool charging) {
    ui_event_t event = {
        .type                  = UI_EVENT_SET_BATTERY_CHARGING,
        .data.battery_charging = charging,
    };
    enqueue_ui_event(&event);
}

void set_status_battery_level(battery_level_t level) {
    ui_event_t event = {
        .type               = UI_EVENT_SET_BATTERY_LEVEL,
        .data.battery_level = level,
    };
    enqueue_ui_event(&event);
}

void set_status_power_connected(bool connected) {
    ui_event_t event = {
        .type                 = UI_EVENT_SET_POWER_CONNECTED,
        .data.power_connected = connected,
    };
    enqueue_ui_event(&event);
}

void set_status_label(const char *label) {
    ui_event_t event = {
        .type = UI_EVENT_SET_LABEL,
    };
    strncpy(event.data.label, label, sizeof(event.data.label));
    enqueue_ui_event(&event);
}

void set_status_minibadge_update() {
    ui_event_t event = {
        .type                 = UI_EVENT_SET_MINIBADGE_UPDATE,
        .data.minibadge_count = minibadge_get_count(),
    };
    enqueue_ui_event(&event);
}

void set_status_alert_count(uint8_t count) {
    ui_event_t event = {
        .type             = UI_EVENT_SET_ALERT_COUNT,
        .data.alert_count = count,
    };
    enqueue_ui_event(&event);
}

void set_screen(ui_screen_t new_screen) {
    if (state.screen == new_screen) {
        ESP_LOGW(TAG, "Already on screen: %s ... not enqueueing SET_SCREEN event", screen_labels[new_screen]);
        return;
    }
    ui_event_t event = {
        .type        = UI_EVENT_SET_SCREEN,
        .data.screen = new_screen,
    };
    enqueue_ui_event(&event);
}

ui_screen_t get_screen() {
    return state.screen;
}

void set_ota_state(ota_state_t state) {
    ui_event_t event = {
        .type           = UI_EVENT_OTA_STATE,
        .data.ota_state = state,
    };
    enqueue_ui_event(&event);
}

static void update_status() {
    if (state.screen == SCREEN_MAIN) {
        ESP_LOGD(TAG, "Updating status bar");
        lv_async_call(render_status, &state);
        vTaskDelay(2);
    }
}

/**
 * @brief Render the main screen content - status bar and main content area
 *      This function should be called from within the LVGL context
 */
static void render_main() {
    if (state.screen == SCREEN_MAIN) {
        ESP_LOGD(TAG, "Rendering main screen content");
        render_content();
        render_status(&state);

        // Start onboarding if not registered
        if (strlen(badge_config.handle) == 0) {
            onboarding_show();
        }

        // Check for OTA updates
        ota_check();
    }
}

static void ui_task(void *_arg) {
    ui_event_t event;
    while (true) {
        if (xQueueReceive(ui_event_queue, &event, portMAX_DELAY) == pdTRUE) {
            ESP_LOGD(TAG, "Received UI event: %s", ui_event_type_map[event.type]);
            switch (event.type) {
                case UI_EVENT_SET_WIFI_STATE: //
                    ESP_LOGD(TAG, "Setting wifi state to: %d", event.data.wifi_state);
                    state.wifi_state = event.data.wifi_state;
                    update_status();
                    break;
                case UI_EVENT_SET_BATTERY_CHARGING: //
                    ESP_LOGD(TAG, "Setting battery charging state to: %d", event.data.battery_charging);
                    state.battery_charging = event.data.battery_charging;
                    update_status();
                    break;
                case UI_EVENT_SET_BATTERY_LEVEL: //
                    ESP_LOGD(TAG, "Setting battery level to: %d", event.data.battery_level);
                    state.battery_level = event.data.battery_level;
                    update_status();
                    break;
                case UI_EVENT_SET_POWER_CONNECTED: //
                    ESP_LOGD(TAG, "Setting power connected state to: %d", event.data.power_connected);
                    state.power_connected = event.data.power_connected;
                    update_status();
                    break;
                case UI_EVENT_SET_LABEL: //
                    ESP_LOGD(TAG, "Setting status label to: %s", event.data.label);
                    strncpy(state.label, event.data.label, sizeof(state.label));
                    update_status();
                    break;
                case UI_EVENT_SET_MINIBADGE_UPDATE: //
                    ESP_LOGD(TAG, "Minibadge update event received");
                    state.minibadge_count = event.data.minibadge_count;
                    update_status();
                    break;
                case UI_EVENT_SET_ALERT_COUNT: //
                    ESP_LOGD(TAG, "Setting alert count to: %d", event.data.alert_count);
                    state.alert_count = event.data.alert_count;
                    update_status();
                    break;
                case UI_EVENT_SET_SCREEN: //
                    ESP_LOGD(TAG, "Event type: %d, screen: %d", event.type, event.data.screen);
                    if (state.screen != event.data.screen) {
                        ESP_LOGD(TAG, "Setting screen to: %s", screen_labels[event.data.screen]);
                        update_screen(event.data.screen);
                    }
                    break;
                case UI_EVENT_OTA_STATE: //
                    ESP_LOGD(TAG, "Setting OTA state");
                    ota_state_t ota_state = event.data.ota_state;
                    bool firmware_updating =
                        (ota_state.status == OTA_STATUS_DOWNLOADING || ota_state.status == OTA_STATUS_INSTALLING);

                    // If we're downloading/installing an OTA update, make sure we're on the update screen
                    if (firmware_updating && state.screen != SCREEN_UPDATE) {
                        set_screen(SCREEN_UPDATE);
                    }

                    // Send the updated OTA state to the update screen for rendering
                    if (state.screen == SCREEN_UPDATE) {
                        update_ota_state(ota_state);
                    }

                    if ((ota_state.status == OTA_STATUS_FAILED || ota_state.status == OTA_STATUS_SUCCESS) &&
                        state.screen == SCREEN_UPDATE) {
                        // If the update is complete, switch back to the main screen
                        set_screen(SCREEN_MAIN);
                    }

                    // Switch back to the main screen if we're not downloading or installing (usually due to an error)
                    if (!firmware_updating && state.screen == SCREEN_UPDATE) {
                        set_screen(SCREEN_MAIN);
                    }
                    break;
                default: //
                    ESP_LOGW(TAG, "Unknown UI event type: %d", event.type);
                    break;
            }
        }
    }
}