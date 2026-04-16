#include <stdio.h>
#include <string.h>

#include <esp_log.h>
#include <mqtt_client.h>
#include <cJSON.h>

#include "hw_iot_mqtt_config.h"
#include "hw_iot_mqtt_json.h"
#include "hw_iot_mqtt_topic.h"

extern const char _binary_cert_pem_start[] asm("_binary_cert_pem_start");
extern const char _binary_cert_pem_end[] asm("_binary_cert_pem_end");

esp_mqtt_client_handle_t mqtt_handle = NULL;

/**
 * @brief MQTT事件回调函数
 *
 * 该函数是ESP-IDF MQTT客户端的事件回调函数，用于处理各种MQTT事件。
 * 支持的事件类型包括：连接、断开、发布确认、订阅确认、数据接收等。
 *
 * @param event_handler_arg 事件处理器的用户参数，当前未使用
 * @param event_base 事件基础类型，用于区分不同的事件源
 * @param event_id 事件ID，标识具体的MQTT事件类型
 *                   - MQTT_EVENT_CONNECTED: 已连接到MQTT代理服务器
 *                   - MQTT_EVENT_DISCONNECTED: 已从MQTT代理服务器断开
 *                   - MQTT_EVENT_PUBLISHED: 消息发布确认
 *                   - MQTT_EVENT_SUBSCRIBED: 订阅确认
 *                   - MQTT_EVENT_UNSUBSCRIBED: 取消订阅确认
 *                   - MQTT_EVENT_DATA: 接收到MQTT消息数据
 * @param event_data 事件数据指针，类型为esp_mqtt_event_handle_t，包含MQTT事件的详细信息
 *                    - topic: 主题字符串
 *                    - topic_len: 主题长度
 *                    - data: 数据载荷
 *                    - data_len: 数据长度
 *
 * @note
 *       1. 该函数由ESP-IDF MQTT客户端在发生MQTT事件时自动调用
 *       2. 在MQTT_EVENT_DATA事件中，会自动提取request_id并发送命令响应
 *       3. 命令响应的result_code固定为0，表示成功
 *       4. 如果无法从topic中提取request_id，会记录错误日志并返回
 *       5. 函数内部会调用hw_iot_mqtt_report发送命令响应
 *       6. 日志标签使用"mqtt_hw_iot"
 */
void mqtt_event_callback(void *event_handler_arg,
                         esp_event_base_t event_base,
                         int32_t event_id,
                         void *event_data)
{
    esp_mqtt_event_handle_t receive_data = event_data;
    switch (event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI("mqtt_hw_iot", "Connected to broker");
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI("mqtt_hw_iot", "Disconnected from broker");
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI("mqtt_hw_iot", "mqtt publish ack");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI("mqtt_hw_iot", "ESP32 Subscribed ack");
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI("mqtt_hw_iot", "topic length: %d", receive_data->topic_len);
        ESP_LOGI("mqtt_hw_iot", "data length: %d", receive_data->data_len);
        ESP_LOGI("mqtt_hw_iot", "topic: %.*s", receive_data->topic_len, receive_data->topic);
        ESP_LOGI("mqtt_hw_iot", "data: %.*s", receive_data->data_len, receive_data->data);
        /* 测试命令响应 */
        hw_iot_mqtt_command_response_json_t command_response_json = {
            .result_code = 0,
            .response_name = "COMMAND_RESPONSE",
            .result = "success"};
        char request_id[128] = {0};
        if (hw_iot_mqtt_topic_get_command_request_id(receive_data, request_id) != ESP_OK) // 从 topic 中提取 request_id
        {
            ESP_LOGW("mqtt_hw_iot", "Failed to get request_id from topic");
            return;
        }
        char *command_response_topic = hw_iot_mqtt_topic_get(HW_IOT_TOPIC_COMMAND_RESPONSE, HW_IOT_DEVICE_ID, request_id); // 获取命令响应 topic
        char *command_response_json_str = hw_iot_mqtt_command_response_json(&command_response_json);                // 生成命令响应 JSON 字符串
        ESP_LOGI("mqtt_hw_iot", "request_id: %s", request_id);
        ESP_LOGI("mqtt_hw_iot", "command_response_topic: %s", command_response_topic);
        ESP_LOGI("mqtt_hw_iot", "command_response_json_str: %s", command_response_json_str);
        hw_iot_mqtt_report(command_response_topic, command_response_json_str); // 发布命令响应
        break;
    default:
        break;
    }
}

/**
 * @brief 发布MQTT消息到华为云IoT平台
 *
 * 该函数用于向华为云IoT平台发布MQTT消息，支持属性上报、命令响应等场景。
 * 消息以QoS=0的级别发布，即最多一次传输，不保证到达。
 *
 * @param topic MQTT主题字符串，指定消息发布的目标主题
 *              例如："$oc/devices/{device_id}/sys/properties/report"
 * @param json_str JSON格式的消息载荷字符串，包含要发送的数据
 *                  例如：{"services": [{"service_id": "xxx", "properties": {...}}]}
 *
 * @return int 成功返回0，失败返回-1
 *
 * @note
 *       1. 函数会检查topic和json_str参数的有效性，任一为空则返回失败
 *       2. 函数会检查MQTT客户端是否已初始化，未初始化则返回失败
 *       3. 消息以QoS=0级别发布，不保证消息可靠到达
 *       4. 发布成功后会记录消息ID和完整载荷到日志
 *       5. 发布失败会记录错误日志，包含失败的消息ID
 *       6. 适用于属性上报、命令响应等MQTT消息发布场景
 *       7. 日志标签使用"mqtt_hw_iot"
 */
int hw_iot_mqtt_report(char *topic, char *json_str)
{
    if (!topic || !json_str || strlen(json_str) <= 0) // 参数为空
    {
        ESP_LOGE("mqtt_hw_iot", "Invalid message data");
        return -1;
    }

    if (!mqtt_handle) // 因为 mqtt_handle 是全局变量，所以可以检测 mqtt 客户端是否初始化
    {
        ESP_LOGE("mqtt_hw_iot", "MQTT client not initialized");
        return -1;
    }

    /* 发布MQTT消息 (QoS=0) */
    int msg_id = esp_mqtt_client_publish(mqtt_handle, topic, json_str, strlen(json_str), 0, 0);
    if (msg_id < 0) // 发布消息失败
    {
        ESP_LOGE("mqtt_hw_iot", "Failed to publish message, msg_id=%d", msg_id);
        return -1;
    }

    ESP_LOGI("mqtt_hw_iot", "Properties reported, topic: %s, msg_id: %d", topic, msg_id);
    ESP_LOGI("mqtt_hw_iot", "Payload: %s", json_str);

    return 0;
}

/**
 * @brief 初始化华为云IoT平台的MQTT客户端
 *
 * 该函数用于初始化并启动连接到华为云IoT平台的MQTT客户端。
 * 配置包括服务器地址、端口、认证信息和SSL证书等。
 *
 * @note
 *       1. 函数会从配置宏中读取连接参数：
 *          - HW_IOT_URI: MQTT服务器地址
 *          - HW_IOT_PORT: MQTT服务器端口
 *          - HW_IOT_CLIENT_ID: 客户端ID
 *          - HW_IOT_USERNAME: 用户名
 *          - HW_IOT_PASSWORD: 密码
 *       2. 使用SSL/TLS加密连接，需要提供服务器证书
 *       3. 证书使用GlobalSign Root CA证书
 *       4. 函数会注册mqtt_event_callback作为事件回调函数
 *       5. 函数会自动启动MQTT客户端，开始连接服务器
 *       6. 连接状态和事件会通过回调函数通知
 *       7. MQTT客户端句柄保存在全局变量mqtt_handle中
 *       8. 该函数应该在应用程序启动时调用一次
 *       9. 如果多次调用，可能会导致资源泄漏或异常
 */
void hw_iot_mqtt_init(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {0};
    mqtt_cfg.broker.address.uri = HW_IOT_URI;
    mqtt_cfg.broker.address.port = HW_IOT_PORT;
    mqtt_cfg.credentials.client_id = HW_IOT_CLIENT_ID;
    mqtt_cfg.credentials.username = HW_IOT_USERNAME;
    mqtt_cfg.credentials.authentication.password = HW_IOT_PASSWORD;
    mqtt_cfg.broker.verification.certificate = _binary_cert_pem_start; // 验证证书
    mqtt_handle = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_handle, ESP_EVENT_ANY_ID, mqtt_event_callback, NULL);
    esp_mqtt_client_start(mqtt_handle);
}