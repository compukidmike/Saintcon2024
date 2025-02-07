#pragma once

typedef struct {
    uint32_t version;
    bool registered;                  // User has registered
    badge_wrist_t wrist;              // Wrist preference
    uint8_t brightness;               // Display brightness preference
    char handle[BADGE_HANDLE_LENGTH]; // User handle
    int xp;                           // Experience points
    int level;                        // User level
    bool enabled;                     // Badge enabled
    bool badge_team;                  // Badge team member
    bool can_level;                   // Can level up others
    char community[64];               // Community name if user is staff
} badge_config_v1_t;
// clang-format off
#define BADGE_DEFAULTS_V1               \
    (badge_config_v1_t){                \
        .version    = 1,                \
        .registered = false,            \
        .wrist      = BADGE_WRIST_LEFT, \
        .brightness = LCD_BACKLIGHT_ON, \
        .handle     = "",               \
        .xp         = 0,                \
        .level      = 0,                \
        .enabled    = false,            \
        .badge_team = false,            \
        .can_level  = false,            \
        .community  = "",               \
    }
// clang-format on
