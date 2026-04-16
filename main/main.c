#include <stdio.h>
#include <string.h>

#include <esp_err.h>
#include <esp_log.h>
#include <driver/gpio.h>

#include "hw_iot_mqtt.h"
#include "wifi.h"
#include "mqtt_hw_iot_message_report.h"
#include "hw_iot_mqtt_json.h"
#include "hw_iot_mqtt_topic.h"

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

    hw_iot_mqtt_init();

    // /* 测试物模型属性上报 */
    // HW_IOT_DM_DES *des = hw_iot_malloc_des(); // 创建一个物模型描述结构体，当前描述结构体中只有services_json字段
    // if (des)
    // {
    //     /* 传入物模型描述结构体des，添加服务属性 */
    //     hw_iot_set_mes_des(des, "BasicData", "luminance", 110); // 添加luminance属性
    //     hw_iot_set_current_time(des, "BasicData");              // 添加当前UTC时间
    //     hw_iot_mes_string(des);                                 // 将物模型结构体序列化为JSON字符串
    //     hw_iot_report_properties(des);                          // 上报设备属性到华为云IoT平台
    // }

// topic: $oc/devices/69cc7e9c6b6c4d5f8d58bd94_3c-84-27-c0-2e-6c/sys/properties/report
// {"services":[{"service_id":"BasicData","properties":{"luminance":110},"event_time":"19700101T015007Z"}]}
// topic: $oc/devices/69cc7e9c6b6c4d5f8d58bd94_3c-84-27-c0-2e-6c/sys/properties/report
// {"services":[{"service_id":"BasicData","properties":{"luminance":110},"event_time":"19700101T020304Z"}]}

    hw_iot_mqtt_properties_report_json_t json = {
        {
            {
                "BasicData", {"luminance"}, {10}
            }
        }
    };
    char *json_str = hw_iot_mqtt_properties_report_json(&json);
    char *topic = hw_iot_mqtt_topic_get(HW_IOT_TOPIC_PROPERTIES_REPORT, DEVICE_ID, NULL);
    ESP_LOGI("main", "topic: %s, json_str: %s", topic, json_str);
    hw_iot_mqtt_report(topic, json_str);
}
