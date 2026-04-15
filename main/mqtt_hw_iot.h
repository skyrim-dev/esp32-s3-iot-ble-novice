#ifndef MQTT_HW_IOT_H
#define MQTT_HW_IOT_H

#include <mqtt_client.h>

// protocol: MQTTS
#define HW_IOT_PRODUCTKEY "69cc7e9c6b6c4d5f8d58bd94"    // 产品密钥
/* 密钥鉴权 */
/* MQTT连接参数 */
#define HW_IOT_CLIENT_ID "69cc7e9c6b6c4d5f8d58bd94_3c-84-27-c0-2e-6c_0_0_2026040109"
#define HW_IOT_USERNAME "69cc7e9c6b6c4d5f8d58bd94_3c-84-27-c0-2e-6c"    // 设备ID（device_id）
#define HW_IOT_PASSWORD "57c89eb42bd6edf2c42b50fdd03ce1d35dc82cd4ce091918fa9ad92e219d5c2e"

#define HW_IOT_HOSTNAME "f1614c4895.iotda-device.cn-south-4.myhuaweicloud.com"
#define HW_IOT_PORT 8883

extern esp_mqtt_client_handle_t mqtt_handle;

// MQTT初始化函数声明
void mqtt_hw_iot_init(void);

// MQTT事件回调函数声明
void mqtt_event_callback(void *event_handler_arg,
                         esp_event_base_t event_base,
                         int32_t event_id,
                         void *event_data);

#endif // MQTT_HW_IOT_H