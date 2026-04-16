idf_component_register(
    SRCS 
        "hw_iot_mqtt_config.c"
        "hw_iot_mqtt_json.c"
        "hw_iot_mqtt_topic.c"
    INCLUDE_DIRS "."
    REQUIRES esp_netif mqtt nvs_flash json esp_wifi
)
