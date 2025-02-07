idf_component_register(SRCS "wifi_manager.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_common esp_netif esp_event esp_wifi nvs)
