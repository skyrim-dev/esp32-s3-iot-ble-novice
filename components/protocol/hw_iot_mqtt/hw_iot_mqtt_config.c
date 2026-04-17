#include <stdio.h>
#include <string.h>

#include <esp_log.h>
#include <mqtt_client.h>
#include <cJSON.h>

#include "hw_iot_mqtt_config.h"
#include "hw_iot_mqtt_json.h"
#include "hw_iot_mqtt_topic.h"
#include "hw_iot_mqtt_subscribe.h"
#include "hw_iot_mqtt_publish.h"

extern const char _binary_cert_pem_start[] asm("_binary_cert_pem_start");
extern const char _binary_cert_pem_end[] asm("_binary_cert_pem_end");

esp_mqtt_client_handle_t mqtt_handle = NULL;

void mqtt_event_callback(void *event_handler_arg,
                         esp_event_base_t event_base,
                         int32_t event_id,
                         void *event_data)
{
    const char *TAG = "mqtt_event_callback";
    esp_mqtt_event_handle_t receive_data = event_data;
    switch (event_id)
    {
    case MQTT_EVENT_CONNECTED: // 连接确认
        ESP_LOGI(TAG, "Connected to broker");
        break;
    case MQTT_EVENT_DISCONNECTED: // 断开连接确认
        ESP_LOGI(TAG, "Disconnected from broker");
        break;
    case MQTT_EVENT_PUBLISHED: // 消息发布确认
        ESP_LOGI(TAG, "mqtt publish ack");
        break;
    case MQTT_EVENT_SUBSCRIBED: // 订阅确认
        ESP_LOGI(TAG, "ESP32 Subscribed ack");
        break;
    case MQTT_EVENT_UNSUBSCRIBED: // 取消订阅确认
        ESP_LOGI(TAG, "ESP32 Unsubscribed ack");
        break;
    case MQTT_EVENT_DATA: // 接收到MQTT消息数据
        ESP_LOGI(TAG, "topic length: %d", receive_data->topic_len);
        ESP_LOGI(TAG, "data length: %d", receive_data->data_len);
        ESP_LOGI(TAG, "topic: %.*s", receive_data->topic_len, receive_data->topic);
        ESP_LOGI(TAG, "data: %.*s", receive_data->data_len, receive_data->data);
        /* 处理订阅确认 */
        hw_iot_mqtt_subscribe_type_t subscribe_type = hw_iot_mqtt_subscribe_type(receive_data);
        ESP_LOGI(TAG, "subscribe_type: %d", subscribe_type);
        if (hw_iot_mqtt_subscribe_ack(subscribe_type, receive_data) != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to subscribe ack");
        }
        break;
    default:
        break;
    }
}

int hw_iot_mqtt_subscribe_ack_public(hw_iot_mqtt_subscribe_type_t subscribe_type, esp_mqtt_event_handle_t receive_data)
{
    char *TAG = "hw_iot_mqtt_subscribe_ack";
    if (!receive_data)
    {
        ESP_LOGE(TAG, "Invalid event handle");
        return ESP_FAIL;
    }
    if (subscribe_type == HW_IOT_MQTT_SUBSCRIBE_TYPE_INVALID)
    {
        ESP_LOGE(TAG, "Invalid subscribe type");
        return ESP_FAIL;
    }
    switch (subscribe_type)
    {
    case HW_IOT_MQTT_MESSAGE_SUBSCRIBE:
        ESP_LOGI(TAG, "Message subscribe ack");
        break;
    case HW_IOT_MQTT_COMMAND_SUBSCRIBE:
        ESP_LOGI(TAG, "Command subscribe ack");
        char request_id[128] = {0};
        hw_iot_mqtt_command_response_json_t command_response_json = {.result_code = 0, .response_name = "COMMAND_RESPONSE", .result = "success"}; // 命令响应 JSON 结构体
        if (hw_iot_mqtt_topic_get_command_request_id(receive_data, request_id) != ESP_OK)                                                         // 从 topic 中提取 request_id
        {
            ESP_LOGW(TAG, "Failed to get request_id from topic");
            command_response_json.result_code = 1; // 作失败处理
            return ESP_FAIL;
        }
        char *command_response_topic = hw_iot_mqtt_topic_get(HW_IOT_TOPIC_COMMAND_RESPONSE, HW_IOT_DEVICE_ID, request_id); // 获取命令响应 topic
        if (command_response_topic == NULL)
        {
            ESP_LOGW(TAG, "Failed to get command_response_topic");
            command_response_json.result_code = 1; // 作失败处理
            return ESP_FAIL;
        }
        char *command_response_json_str = hw_iot_mqtt_command_response_json(&command_response_json); // 生成命令响应 JSON 字符串
        ESP_LOGI(TAG, "request_id: %s", request_id);
        ESP_LOGI(TAG, "command_response_topic: %s", command_response_topic);
        ESP_LOGI(TAG, "command_response_json_str: %s", command_response_json_str);
        hw_iot_mqtt_publish(command_response_topic, command_response_json_str); // 发布命令响应
        free(command_response_json_str);
        break;
    case HW_IOT_MQTT_VERSION_QUERY_SUBSCRIBE:
        ESP_LOGI(TAG, "Version query subscribe ack");
        if (hw_iot_mqtt_ota_version_publish() != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to report version");
            return ESP_FAIL;
        }
        break;
    case HW_IOT_MQTT_FIRMWARE_UPGRADE_SUBSCRIBE:
        ESP_LOGI(TAG, "Firmware upgrade subscribe ack");
        break;
    default:
        break;
    }
    return ESP_OK;
}

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