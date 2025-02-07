idf_component_register(SRCS "badge.c" "config.c" "ir.c" "migrate.c" "ota.c" "towers.c" "version.c"
                       INCLUDE_DIRS "include"
                       REQUIRES "api" "display" "ir_comm" "minibadge" "nvs" "spiffs" "ui" "wifi_manager")
