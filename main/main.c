#include <stdio.h>
#include <string.h>

#include <esp_err.h>
#include <esp_log.h>
#include <driver/gpio.h>

#include "hw_iot_mqtt_client.h"
#include "hw_iot_mqtt_topic.h"
#include "hw_iot_mqtt_json.h"
#include "hw_iot_mqtt_publish.h"
#include "wifi.h"

static void led_init(void)
{
    gpio_reset_pin(GPIO_NUM_1);
    gpio_set_direction(GPIO_NUM_1, GPIO_MODE_OUTPUT);
}

void app_main(void)
{
    char *TAG = "main";
    /* 初始化日志模块 */
    ESP_LOGI(TAG, "Hello world!");

    /* 初始化LED */
    led_init();

    /* 获取应用版本号 */
    ESP_LOGI(TAG, "app_version: %s", get_app_version());

    /* 初始化WiFi */
    wifi_init();

    /* 等待WiFi连接成功，获取到IP地址后，初始化MQTT客户端 */
    xSemaphoreTake(wifi_connected_semaphore, portMAX_DELAY); // 等待WiFi连接成功
    ESP_LOGI(TAG, "WiFi connected");

    /* 初始化MQTT客户端 */
    hw_iot_mqtt_init();

    /* mqtt发布属性 */
    hw_iot_mqtt_properties_publish();
}
