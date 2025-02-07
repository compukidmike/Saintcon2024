#pragma once

typedef struct {
    uint32_t version;
    bool hw_pass;                     // Initial hardware tests passed
    bool registered;                  // User has registered
    badge_wrist_t wrist;              // Wrist preference
    uint8_t brightness;               // Display brightness preference
    uint32_t screen_timeout;          // Screen timeout in seconds
    char handle[BADGE_HANDLE_LENGTH]; // User handle
    int xp;                           // Experience points
    int level;                        // User level
    bool enabled;                     // Badge enabled
    bool badge_team;                  // Badge team member
    bool can_level;                   // Can level up others
    char community[64];               // Community name if user is staff
} badge_config_v3_t;
// clang-format off
#define BADGE_DEFAULTS_V3                   \
    (badge_config_v3_t){                    \
        .version        = 3,                \
        .hw_pass        = false,            \
        .registered     = false,            \
        .wrist          = BADGE_WRIST_LEFT, \
        .brightness     = LCD_BACKLIGHT_ON, \
        .screen_timeout = 30,               \
        .handle         = "",               \
        .xp             = 0,                \
        .level          = 0,                \
        .enabled        = false,            \
        .badge_team     = false,            \
        .can_level      = false,            \
        .community      = "",               \
    }
// clang-format on
