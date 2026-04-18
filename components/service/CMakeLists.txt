idf_component_register(
    SRCS 
        "ota/hw_iot_ota.c"
        "ota/ota_manager.c"
    INCLUDE_DIRS "ota"
    REQUIRES esp_https_ota mbedtls app_update protocol nvs_flash
)
