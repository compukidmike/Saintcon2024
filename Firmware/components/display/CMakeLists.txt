idf_component_register(SRCS "display.c" "touch.c"
                       INCLUDE_DIRS "include"
                       REQUIRES "driver" "lvgl" "esp_lcd" "esp_lcd_touch_gt911" "i2c_manager" "ui")
