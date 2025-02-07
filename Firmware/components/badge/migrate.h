#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "nvs.h"

void migrate_badge_config(nvs_handle_t nvs_handle, uint32_t stored_version);

#ifdef __cplusplus
}
#endif
