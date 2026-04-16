idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "." "../components/protocol/hw_iot_mqtt"
    REQUIRES protocol network esp_driver_gpio
)
