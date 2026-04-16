idf_component_register(
    SRCS 
        "../driver/wifi/wifi.c"
        "../protocol/mqtt/hw_iot/hw_iot_mqtt_json.c"
        "../protocol/mqtt/hw_iot/hw_iot_mqtt_topic.c"
        "../protocol/mqtt/mqtt_config.c"
    INCLUDE_DIRS 
        "../driver/wifi"
        "../protocol/mqtt/hw_iot"
        "../protocol/mqtt"
    REQUIRES esp_netif mqtt esp_timer nvs_flash json esp_wifi esp_driver_gpio
)
