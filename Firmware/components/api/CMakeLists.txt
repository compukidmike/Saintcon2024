idf_component_register(SRCS "api_client.cpp" "api.cpp" "types.cpp"
                       INCLUDE_DIRS "include"
                       REQUIRES "app_update" "esp_http_client" "esp_https_ota" "nlohmann-json" "badge" "nvs"
                       EMBED_TXTFILES "certs/isrgrootx1.pem")
