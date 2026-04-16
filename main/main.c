#include <stdio.h>
#include <string.h>

#include <esp_err.h>
#include <esp_log.h>
#include <driver/gpio.h>

#include "hw_iot_mqtt.h"
#include "wifi.h"
#include "mqtt_hw_iot_message_report.h"

static void led_init(void)
{
    gpio_reset_pin(GPIO_NUM_1);
    gpio_set_direction(GPIO_NUM_1, GPIO_MODE_OUTPUT);
}

void app_main(void)
{
    ESP_LOGI("main", "Hello world!");

    /* 初始化LED */
    led_init();
    /* 初始化WiFi */
    wifi_init();

    /* 等待WiFi连接成功，获取到IP地址后，初始化MQTT客户端 */
    xSemaphoreTake(wifi_connected_semaphore, portMAX_DELAY); // 等待WiFi连接成功
    ESP_LOGI("main", "WiFi connected");

    mqtt_hw_iot_init();

    /* 测试物模型属性上报 */
    HW_IOT_DM_DES *des = hw_iot_malloc_des(); // 创建一个物模型描述结构体，当前描述结构体中只有services_json字段
    if (des)
    {
        /* 传入物模型描述结构体des，添加服务属性 */
        hw_iot_set_mes_des(des, "BasicData", "luminance", 110); // 添加luminance属性
        hw_iot_set_current_time(des, "BasicData");              // 添加当前UTC时间
        hw_iot_mes_string(des);                                 // 将物模型结构体序列化为JSON字符串
        hw_iot_report_properties(des);                          // 上报设备属性到华为云IoT平台
    }
}  
