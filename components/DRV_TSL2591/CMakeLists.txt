idf_component_register(
    SRCS tsl2591.c
    INCLUDE_DIRS .
    REQUIRES DRV_I2Cdev log esp_idf_lib_helpers
)
