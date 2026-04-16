idf_component_register(
    SRCS 
        "protocol/hw_iot/hw_iot_json.c"
        "protocol/hw_iot/hw_iot_topic.c"
        "driver/wifi/wifi.c"
    INCLUDE_DIRS "hw_iot_mqtt" "protocol/hw_iot" "driver/wifi"
    REQUIRES esp_netif mqtt esp_timer nvs_flash json esp_wifi esp_driver_gpio
)
