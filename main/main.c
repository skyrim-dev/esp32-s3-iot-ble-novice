#include <stdio.h>
#include <string.h>

#include <esp_err.h>
#include <esp_log.h>

#include "mqtt_hw_iot.h"
#include "wifi.h"
#include "mqtt_hw_iot_message_up.h"



void app_main(void)
{
    ESP_LOGI("main", "Hello world!");

    /* 初始化WiFi */
    wifi_init();

    /* 等待WiFi连接成功，获取到IP地址后，初始化MQTT客户端 */
    xSemaphoreTake(wifi_connected_semaphore, portMAX_DELAY); // 等待WiFi连接成功
    ESP_LOGI("main", "WiFi connected");

    mqtt_hw_iot_init();

}