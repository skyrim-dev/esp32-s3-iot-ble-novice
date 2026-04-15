#include <stdio.h>
#include <string.h>

#include <esp_log.h>
#include <mqtt_client.h>
#include <mbedtls/md5.h>
#include <mbedtls/md.h>

#include "mqtt_hw_iot.h"

// 定义MQTT客户端句柄
esp_mqtt_client_handle_t mqtt_handle = NULL;

void mqtt_event_callback(void *event_handler_arg,
                         esp_event_base_t event_base,
                         int32_t event_id,
                         void *event_data)
{
    esp_mqtt_event_handle_t receive_data = event_data;
    switch (event_id)
    {
    case MQTT_EVENT_CONNECTED: // 连接成功
        ESP_LOGI("mqtt", "Connected to broker");
        break;
    case MQTT_EVENT_DISCONNECTED: // 断开连接
        ESP_LOGI("mqtt", "Disconnected from broker");
        break;
    case MQTT_EVENT_PUBLISHED: // 发布消息成功
        ESP_LOGI("mqtt", "mqtt publish ack");
        break;
    case MQTT_EVENT_SUBSCRIBED: // 订阅成功成功
        ESP_LOGI("mqtt", "ESP32 Subscribed ack");
        break;
    case MQTT_EVENT_UNSUBSCRIBED: // 收到解订阅消息
        break;
    case MQTT_EVENT_DATA: // ESP32收到消息
        /* 注意，需要日志打印出指定长度的字符串，否则容易接收到乱码 */
        ESP_LOGI("mqtt", "topic length: %d", receive_data->topic_len);
        ESP_LOGI("mqtt", "topic: %.*s", receive_data->topic_len, receive_data->topic);
        ESP_LOGI("mqtt", "data length: %d", receive_data->data_len);
        ESP_LOGI("mqtt", "data: %.*s", receive_data->data_len, receive_data->data);
        break;
    default:
        break;
    }
}

void mqtt_hw_iot_init(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {0};                      // 初始化配置结构体
    mqtt_cfg.broker.address.uri = HW_IOT_HOSTNAME;                   // 设置MQTT地址
    mqtt_cfg.broker.address.port = HW_IOT_PORT;                          // 设置MQTT端口
    mqtt_cfg.credentials.client_id = HW_IOT_CLIENT_ID;              // 设置客户端ID
    mqtt_cfg.credentials.username = HW_IOT_USERNAME;                // 设置用户名
    mqtt_cfg.credentials.authentication.password = HW_IOT_PASSWORD; // 设置密码
    mqtt_handle = esp_mqtt_client_init(&mqtt_cfg);                // 初始化MQTT客户端句柄
    /* 注册事件回调函数 */
    esp_mqtt_client_register_event(mqtt_handle, ESP_EVENT_ANY_ID, mqtt_event_callback, NULL);
    esp_mqtt_client_start(mqtt_handle); // 启动MQTT客户端
}

void mqtt_publish_message(const char *topic, const char *message)
{
    if (mqtt_handle != NULL) {
        esp_mqtt_client_publish(mqtt_handle, topic, message, strlen(message), 1, 0);
    }
}