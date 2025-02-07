#include <time.h>
#include "esp_mac.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "content.h"
#include "home.h"
#include "theme.h"
#include "ui.h"
#include "version.h"

static const char *TAG = "pages/home";

// Images
LV_IMG_DECLARE(resistance_logo);

// Timers
static lv_timer_t *logo_press_timer = NULL;
static lv_timer_t *clock_timer      = NULL;

// Labels
static lv_obj_t *clock_label = NULL;

// Timer callback to change the screen to the TEST screen after 5 seconds of pressing the SAINTCON logo
static void logo_press_timer_cb(lv_timer_t *timer) {
    set_screen(SCREEN_HWTEST);
    lv_timer_delete(timer);
    logo_press_timer = NULL;
}

// Event handler for long press on the SAINTCON logo to show the badge test screen
static void logo_press_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_PRESSING) {
        if (logo_press_timer == NULL) {
            logo_press_timer = lv_timer_create(logo_press_timer_cb, 3000, NULL);
        }
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        if (logo_press_timer != NULL) {
            lv_timer_delete(logo_press_timer);
            logo_press_timer = NULL;
        }
    }
}

static void clock_cleanup(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_DELETE) {
        if (clock_timer != NULL) {
            lv_timer_delete(clock_timer);
            clock_timer = NULL;
        }
    }
}

const char *get_formatted_time() {
    time_t now;
    struct tm timeinfo;
    static char time_buffer[20];
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(time_buffer, sizeof(time_buffer), "%I:%M %p", &timeinfo);
    return time_buffer;
}

static void clock_timer_cb(lv_timer_t *timer) {
    // Update the clock with the current time
    lv_label_set_text(clock_label, get_formatted_time());
}

void home_page_create(lv_obj_t *parent) {
    // Show the resistance logo on the main screen
    lv_obj_t *resistance_logo_img = lv_image_create(parent);
    lv_image_set_src(resistance_logo_img, &resistance_logo);
    lv_obj_align(resistance_logo_img, LV_ALIGN_TOP_MID, 0, 20);

    // Set an event handler to detect long presses
    lv_obj_add_flag(resistance_logo_img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(resistance_logo_img, logo_press_handler, LV_EVENT_ALL, NULL);

    // Display the badge ID in the main content area 5px above the bottom
    lv_obj_t *badge_version_label = lv_label_create(parent);
    lv_label_set_text(badge_version_label, "v" FIRMWARE_VERSION_STRING);
    lv_obj_set_style_text_font(badge_version_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(badge_version_label, lv_color_hex(0xAAAAAA), LV_PART_MAIN);
    lv_obj_align(badge_version_label, LV_ALIGN_BOTTOM_MID, 0, -15);

    // Add a container to show a clock over top of the logo
    lv_obj_t *clock_container = lv_obj_create(parent);
    lv_obj_set_size(clock_container, 120, 40);
    lv_obj_set_style_bg_color(clock_container, lv_color_hex(CONTENT_BG), LV_PART_MAIN);
    lv_obj_set_style_radius(clock_container, 5, LV_PART_MAIN);
    lv_obj_set_style_border_width(clock_container, 3, LV_PART_MAIN);
    lv_obj_set_style_border_color(clock_container, lv_color_hex(0xFF6A00), LV_PART_MAIN);
    lv_obj_remove_flag(clock_container, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scrollbar_mode(clock_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align(clock_container, LV_ALIGN_TOP_MID, 0, 15);
    lv_obj_add_event_cb(clock_container, clock_cleanup, LV_EVENT_DELETE, NULL);

    // Create a label to display the time
    clock_label = lv_label_create(clock_container);
    lv_label_set_text(clock_label, get_formatted_time());
    lv_obj_set_style_text_font(clock_label, &bm_mini_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(clock_label, lv_color_hex(GRAY_TINT_4), LV_PART_MAIN);
    lv_obj_align(clock_label, LV_ALIGN_CENTER, 0, 3);
    lv_obj_add_flag(clock_label, LV_OBJ_FLAG_EVENT_BUBBLE); // To allow the long press to pass through

    // Create a timer to update the clock every second
    clock_timer = lv_timer_create(clock_timer_cb, 1000, NULL);
}
