idf_component_register(SRCS "power_manager.c"
                       INCLUDE_DIRS "include"
                       REQUIRES "charger" "battery" "type_c" "load_switch" "ui")
