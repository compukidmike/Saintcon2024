#include <string.h>
#include "esp_log.h"

#include "config.h"
#include "migrate.h"

static const char *TAG = "badge/config [migrate]";

/**
 * @brief Migrates the badge configuration to the latest version
 *
 * This function should handle any necessary transformations required
 * to bring an older version of the configuration struct up to date
 * with the latest version.
 *
 * @param stored_version The version of the stored configuration
 */
void migrate_badge_config(nvs_handle_t nvs_handle, uint32_t stored_version) {
    bool migrated_last = false;

    // Create a new config struct with the latest version to migrate to
    badge_config_v1_t config_v1 = BADGE_DEFAULTS_V1;
    badge_config_v2_t config_v2 = BADGE_DEFAULTS_V2;
    badge_config_v3_t config_v3 = BADGE_DEFAULTS_V3;
    badge_config_v4_t config_v4 = BADGE_DEFAULTS_V4;
    badge_config_v5_t config_v5 = BADGE_DEFAULTS_V5;

    // ----------------------------------------------------------------------- //
    //                        MIGRATION IMPLEMENTATION                         //
    // ----------------------------------------------------------------------- //

    // Version 1 to 2
    if (stored_version == 1) {
        ESP_LOGD(TAG, "Migrating badge config from version 1 to 2");
        size_t required_size = sizeof(badge_config_v1_t);
        esp_err_t err        = nvs_get_blob(nvs_handle, "badge_config", &config_v1, &required_size);
        if (err == ESP_OK) {
            config_v2.wrist      = config_v1.wrist;
            config_v2.brightness = config_v1.brightness;
            config_v2.xp         = config_v1.xp;
            config_v2.level      = config_v1.level;
            config_v2.enabled    = config_v1.enabled;
            config_v2.badge_team = config_v1.badge_team;
            config_v2.can_level  = config_v1.can_level;
            strncpy(config_v2.handle, config_v1.handle, sizeof(config_v2.handle));
            strncpy(config_v2.community, config_v1.community, sizeof(config_v2.community));

            migrated_last = true;
        }
    }

    // Version 2 to 3
    if (stored_version == 2 || migrated_last) {
        ESP_LOGD(TAG, "Migrating badge config from version 2 to 3");
        size_t required_size = sizeof(badge_config_v2_t);
        esp_err_t err        = ESP_OK;
        if (!migrated_last) {
            err = nvs_get_blob(nvs_handle, "badge_config", &config_v2, &required_size);
        }
        if (err == ESP_OK) {
            config_v3.hw_pass    = config_v2.hw_pass;
            config_v3.wrist      = config_v2.wrist;
            config_v3.brightness = config_v2.brightness;
            config_v3.xp         = config_v2.xp;
            config_v3.level      = config_v2.level;
            config_v3.enabled    = config_v2.enabled;
            config_v3.badge_team = config_v2.badge_team;
            config_v3.can_level  = config_v2.can_level;
            strncpy(config_v3.handle, config_v2.handle, sizeof(config_v3.handle));
            strncpy(config_v3.community, config_v2.community, sizeof(config_v3.community));

            migrated_last = true;
        }
    }

    // Version 3 to 4
    if (stored_version == 3 || migrated_last) {
        ESP_LOGD(TAG, "Migrating badge config from version 3 to 4");
        size_t required_size = sizeof(badge_config_v3_t);
        esp_err_t err        = ESP_OK;
        if (!migrated_last) {
            err = nvs_get_blob(nvs_handle, "badge_config", &config_v3, &required_size);
        }
        if (err == ESP_OK) {
            config_v4.hw_pass        = config_v3.hw_pass;
            config_v4.registered     = config_v3.registered;
            config_v4.wrist          = config_v3.wrist;
            config_v4.brightness     = config_v3.brightness;
            config_v4.screen_timeout = config_v3.screen_timeout;
            config_v4.xp             = config_v3.xp;
            config_v4.level          = config_v3.level;
            config_v4.enabled        = config_v3.enabled;
            config_v4.badge_team     = config_v3.badge_team;
            config_v4.can_level      = config_v3.can_level;
            strncpy(config_v4.handle, config_v3.handle, sizeof(config_v4.handle));
            strncpy(config_v4.community, config_v3.community, sizeof(config_v4.community));

            migrated_last = true;
        }
    }

    // Version 4 to 5
    if (stored_version == 4 || migrated_last) {
        ESP_LOGD(TAG, "Migrating badge config from version 4 to 5");
        size_t required_size = sizeof(badge_config_v4_t);
        esp_err_t err        = ESP_OK;
        if (!migrated_last) {
            err = nvs_get_blob(nvs_handle, "badge_config", &config_v4, &required_size);
        }
        if (err == ESP_OK) {
            config_v5.hw_pass          = config_v4.hw_pass;
            config_v5.registered       = config_v4.registered;
            config_v5.wrist            = config_v4.wrist;
            config_v5.brightness       = config_v4.brightness;
            config_v5.screen_timeout   = config_v4.screen_timeout;
            config_v5.id               = config_v4.id;
            config_v5.xp               = config_v4.xp;
            config_v5.level            = config_v4.level;
            config_v5.enabled          = config_v4.enabled;
            config_v5.badge_team       = config_v4.badge_team;
            config_v5.staff            = config_v4.staff;
            config_v5.blackbadge       = config_v4.blackbadge;
            config_v5.can_level        = config_v4.can_level;
            config_v5.community_levels = config_v4.community_levels;
            config_v5.coins            = config_v4.coins;
            strncpy(config_v5.handle, config_v4.handle, sizeof(config_v5.handle));
            strncpy(config_v5.community, config_v4.community, sizeof(config_v5.community));
        }
    }

    // Save the new config
    badge_config = config_v5;
    save_badge_config();
}
