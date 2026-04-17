#include <stdio.h>
#include <string.h>

#include <esp_err.h>
#include <esp_log.h>
#include <driver/gpio.h>

#include "hw_iot_mqtt_config.h"
#include "hw_iot_mqtt_topic.h"
#include "hw_iot_mqtt_json.h"
#include "wifi.h"

static void led_init(void)
{
    gpio_reset_pin(GPIO_NUM_1);
    gpio_set_direction(GPIO_NUM_1, GPIO_MODE_OUTPUT);
}

void app_main(void)
{
    /* 初始化日志模块 */
    ESP_LOGI("main", "Hello world!");

    /* 初始化LED */
    led_init();

    /* 初始化WiFi */
    wifi_init();

    /* 等待WiFi连接成功，获取到IP地址后，初始化MQTT客户端 */
    xSemaphoreTake(wifi_connected_semaphore, portMAX_DELAY); // 等待WiFi连接成功
    ESP_LOGI("main", "WiFi connected");

    /* 初始化MQTT客户端 */
    hw_iot_mqtt_init();

    /* 测试属性上报 */
    hw_iot_mqtt_properties_report_json_t json = {
        {
            {"BasicData", {"luminance"}, {1}},
        }};
    char *topic = hw_iot_mqtt_topic_get(HW_IOT_TOPIC_PROPERTIES_REPORT, HW_IOT_DEVICE_ID, NULL);
    char *json_str = hw_iot_mqtt_properties_report_json(&json);
    ESP_LOGI("main", "topic: %s, json_str: %s", topic, json_str);
    hw_iot_mqtt_publish(topic, json_str);
    free(json_str);
}
