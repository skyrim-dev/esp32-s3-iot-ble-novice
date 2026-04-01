#include <stdio.h>
#include <esp_log.h>
#include "wifi.h"

void app_main(void)
{
    ESP_LOGI("main", "Hello world!");

    wifi_init();
}
