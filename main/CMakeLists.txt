idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES protocol network esp_driver_gpio
)
