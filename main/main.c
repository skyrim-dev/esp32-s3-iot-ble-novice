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

    /* 等待MQTT连接成功 */
    vTaskDelay(pdMS_TO_TICKS(2000));

    /* 测试物模型属性上报 */
    HW_IOT_DM_DES *des = hw_iot_malloc_des();   // 创建一个物模型描述结构体，当前描述结构体中只有services_json字段
    if (des)
    {
        /* 传入物模型描述结构体des，添加服务属性 */
        hw_iot_set_mes_des(des, "BasicData", "luminance", 110);
        hw_iot_set_current_time(des, "BasicData");
        hw_iot_mes_string(des);
        hw_iot_report_properties(des);
        hw_iot_free_des(des);
    }
}