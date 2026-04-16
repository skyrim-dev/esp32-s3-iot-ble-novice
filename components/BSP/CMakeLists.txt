idf_component_register(
    SRCS 
        "hw_iot_mqtt/hw_iot_mqtt.c"
        "hw_iot_mqtt/hw_iot_mqtt_json.c"
        "hw_iot_mqtt/hw_iot_mqtt_topic.c"
        "my_wifi/wifi.c"
    INCLUDE_DIRS "hw_iot_mqtt" "my_wifi"
    REQUIRES esp_netif mqtt esp_timer nvs_flash json esp_wifi esp_driver_gpio
)
