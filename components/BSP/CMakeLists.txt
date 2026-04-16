idf_component_register(
    SRCS 
        "hw_iot_mqtt/mqtt_config.c"
        "hw_iot_mqtt/protocol/mqtt/json/json_util.c"
        "hw_iot_mqtt/hw_iot_mqtt_topic.c"
        "hw_iot_mqtt/hw_iot_mqtt_ota.c"
        "hw_iot_mqtt/drivers/wifi/wifi.c"
    INCLUDE_DIRS "hw_iot_mqtt" "hw_iot_mqtt/protocol/mqtt/json" "hw_iot_mqtt/drivers/wifi"
    REQUIRES esp_netif mqtt esp_timer nvs_flash json esp_wifi esp_driver_gpio
)
