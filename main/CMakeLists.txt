idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "." "../components/driver/wifi" "../components/protocol/hw_iot" "../components/protocol/mqtt"
    REQUIRES BSP esp_driver_gpio
)