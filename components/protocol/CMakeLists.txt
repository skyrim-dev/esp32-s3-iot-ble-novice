idf_component_register(
    SRCS 
        "hw_iot_mqtt/hw_iot_mqtt_client.c"
        "hw_iot_mqtt/hw_iot_mqtt_json.c"
        "hw_iot_mqtt/hw_iot_mqtt_topic.c"
        "hw_iot_mqtt/hw_iot_mqtt_subscribe.c"
        "hw_iot_mqtt/hw_iot_mqtt_publish.c"
    INCLUDE_DIRS "hw_iot_mqtt"
    REQUIRES esp_netif mqtt esp_timer nvs_flash json esp_wifi esp_driver_gpio app_update

    EMBED_FILES "hw_iot_mqtt/cert.pem"
)
