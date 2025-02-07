idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES
                           "console"
                           "accel"
                           "api"
                           "badge"
                           "battery"
                           "charger"
                           "display"
                           "i2c_manager"
                           "ir_comm"
                           "nvs"
                           "power_manager"
                           "type_c"
                           "ui"
                           "wifi_manager")

# Define the location of the SPIFFS directory
set(SPIFFS_DIR ${CMAKE_SOURCE_DIR}/spiffs)
set(SPIFFS_TIMESTAMP ${CMAKE_BINARY_DIR}/spiffs.timestamp)

# Create a custom target to update the timestamp file
add_custom_command(
    OUTPUT ${SPIFFS_TIMESTAMP}
    COMMAND ${CMAKE_COMMAND} -E echo "Updating SPIFFS timestamp" > ${SPIFFS_TIMESTAMP}
    DEPENDS ${SPIFFS_DIR}
    COMMENT "Checking if SPIFFS needs to be rebuilt"
    VERBATIM
)

# Use file(GLOB_RECURSE) to track all files in the SPIFFS directory
file(GLOB_RECURSE SPIFFS_FILES ${SPIFFS_DIR}/*)

# Create a custom target that will run the spiffs_create_partition_image command
add_custom_target(
    spiffs_image ALL
    DEPENDS ${SPIFFS_TIMESTAMP} ${SPIFFS_FILES}
)

# Use spiffs_create_partition_image with the timestamp file as a dependency
# spiffs_create_partition_image(storage ${SPIFFS_DIR} FLASH_IN_PROJECT DEPENDS spiffs_image)