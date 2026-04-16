idf_component_register(
    SRCS "wifi/wifi.c"
    INCLUDE_DIRS "wifi"
    REQUIRES esp_netif esp_wifi nvs_flash
)
