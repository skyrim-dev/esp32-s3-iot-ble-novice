idf_component_register(
    SRCS 
        "hw_iot_json.c"
        "hw_iot_topic.c"
        "protocol/mqtt/mqtt_config.c"
        "driver/wifi/wifi.c"
    INCLUDE_DIRS "hw_iot_mqtt" "protocol/mqtt" "driver/wifi"
    REQUIRES esp_netif mqtt esp_timer nvs_flash json esp_wifi esp_driver_gpio
)
