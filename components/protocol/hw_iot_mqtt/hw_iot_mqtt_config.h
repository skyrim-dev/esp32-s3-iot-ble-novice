#ifndef HW_IOT_MQTT_CONFIG_H
#define HW_IOT_MQTT_CONFIG_H

#include <mqtt_client.h>
#include "sdkconfig.h"

// ========== 华为云连接参数 ==========
#define HW_IOT_PRODUCTKEY CONFIG_HW_IOT_PRODUCTKEY
#define HW_IOT_CLIENT_ID CONFIG_HW_IOT_CLIENT_ID
#define HW_IOT_USERNAME CONFIG_HW_IOT_USERNAME
#define HW_IOT_PASSWORD CONFIG_HW_IOT_PASSWORD
#define HW_IOT_HOSTNAME CONFIG_HW_IOT_HOSTNAME
#define HW_IOT_PORT CONFIG_HW_IOT_PORT
#define HW_IOT_URI "mqtts://" HW_IOT_HOSTNAME ":" STRINGIFY(HW_IOT_PORT)

// ========== 设备信息 ==========
#define HW_IOT_DEVICE_ID CONFIG_HW_IOT_DEVICE_ID

// 辅助宏：将数字转换为字符串
#define STRINGIFY(x) STRINGIFY2(x)
#define STRINGIFY2(x) #x

// ========== 函数声明 ==========
extern esp_mqtt_client_handle_t mqtt_handle;
void mqtt_event_callback(void *event_handler_arg,
                         esp_event_base_t event_base,
                         int32_t event_id,
                         void *event_data);          // MQTT 事件回调函数
int hw_iot_mqtt_report(char *topic, char *json_str); // 发布 JSON 字符串到指定主题
void hw_iot_mqtt_init(void);                                // 初始化 MQTT 客户端并连接到华为云 IoT 平台

#endif