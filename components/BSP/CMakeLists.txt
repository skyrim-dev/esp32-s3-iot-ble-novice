idf_component_register(
    SRCS 
        "hw_iot_mqtt/hw_iot_mqtt.c"
        "hw_iot_mqtt/protocol/mqtt/json/json_util.c"
        "hw_iot_mqtt/hw_iot_mqtt_topic.c"
        "hw_iot_mqtt/hw_iot_mqtt_ota.c"
        "my_wifi/wifi.c"
    INCLUDE_DIRS "hw_iot_mqtt" "my_wifi" "hw_iot_mqtt/protocol/mqtt/json"
    REQUIRES esp_netif mqtt esp_timer nvs_flash json esp_wifi esp_driver_gpio
)
