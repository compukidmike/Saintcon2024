# Images
set(embedded_images)
file(GLOB_RECURSE embedded_images images/*.c)

# Fonts
set(embedded_fonts)
file(GLOB_RECURSE embedded_fonts fonts/*.c)

# Components
set(component_sources)
file(GLOB_RECURSE component_sources components/*.c)

# Screens
set(screen_sources)
file(GLOB_RECURSE screen_sources screens/*.c)

# Pages
set(page_sources)
file(GLOB_RECURSE page_sources pages/*.c)

idf_component_register(SRCS "components.c" "content.c" "loadanim.c" "onboarding.c" "statusbar.c" "theme.c" "ui_events.c" "ui.c" ${component_sources} ${screen_sources} ${page_sources} ${embedded_images} ${embedded_fonts}
                       INCLUDE_DIRS "include"
                       REQUIRES "accel" "api" "badge" "battery" "charger" "display" "load_switch" "lvgl" "power_manager" "type_c" "wifi_manager")

include_directories(${CMAKE_BINARY_DIR}/include)
target_include_directories(${COMPONENT_LIB} PRIVATE ".")
