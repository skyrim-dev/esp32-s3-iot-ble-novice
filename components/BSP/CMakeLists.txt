idf_component_register(
    SRCS 
        "../driver/wifi/wifi.c"
        "../protocol/hw_iot/hw_iot_json.c"
        "../protocol/hw_iot/hw_iot_topic.c"
        "../protocol/mqtt/mqtt_config.c"
    INCLUDE_DIRS 
        "../driver/wifi"
        "../protocol/hw_iot"
        "../protocol/mqtt"
    REQUIRES esp_netif mqtt esp_timer nvs_flash json esp_wifi esp_driver_gpio
)
