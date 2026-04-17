#include <mqtt_client.h>
#include <string.h>
#include <esp_log.h>

#include "hw_iot_mqtt_subscribe.h"
#include "hw_iot_mqtt_json.h"
#include "hw_iot_mqtt_topic.h"
#include "hw_iot_mqtt_config.h"
#include "hw_iot_mqtt_publish.h"

hw_iot_mqtt_subscribe_type_t hw_iot_mqtt_subscribe_type(esp_mqtt_event_handle_t receive_data)
{
    if (!receive_data)
    {
        ESP_LOGE("hw_iot_mqtt_subscribe_type", "Invalid event handle");
        return HW_IOT_MQTT_SUBSCRIBE_TYPE_INVALID;
    }

    char topic[256] = {0};
    char json_payload[256] = {0};
    // 安全复制主题，确保以空字符结尾
    int topic_len = (receive_data->topic_len < sizeof(topic) - 1) ? receive_data->topic_len : sizeof(topic) - 1;
    memcpy(topic, receive_data->topic, topic_len);
    topic[topic_len] = '\0';
    // 安全复制数据载荷，确保以空字符结尾
    int data_len = (receive_data->data_len < sizeof(json_payload) - 1) ? receive_data->data_len : sizeof(json_payload) - 1;
    memcpy(json_payload, receive_data->data, data_len);
    json_payload[data_len] = '\0';

    ESP_LOGI("hw_iot_mqtt_subscribe_type", "topic length: %d", receive_data->topic_len);
    ESP_LOGI("hw_iot_mqtt_subscribe_type", "data length: %d", receive_data->data_len);
    ESP_LOGI("hw_iot_mqtt_subscribe_type", "topic: %.*s", receive_data->topic_len, topic);
    ESP_LOGI("hw_iot_mqtt_subscribe_type", "data: %.*s", receive_data->data_len, json_payload);

    if (strstr(topic, "sys/messages/down")) // 消息主题
    {
        ESP_LOGI("hw_iot_mqtt_subscribe_type", "Message subscribe");
        return HW_IOT_MQTT_MESSAGE_SUBSCRIBE;
    }
    else if (strstr(topic, "sys/commands/request_id=")) // 命令主题
    {
        ESP_LOGI("hw_iot_mqtt_subscribe_type", "Command subscribe");
        return HW_IOT_MQTT_COMMAND_SUBSCRIBE;
    }
    else if (strstr(json_payload, "$ota") && strstr(topic, "sys/events/down")) // OTA服务主题
    {
        ESP_LOGI("hw_iot_mqtt_subscribe_type", "OTA subscribe");
        if (strstr(json_payload, "version_query")) // 版本查询事件
        {
            ESP_LOGI("hw_iot_mqtt_subscribe_type", "OTA: Version query subscribe");
            return HW_IOT_MQTT_VERSION_QUERY_SUBSCRIBE;
        }
        else if (strstr(json_payload, "firmware_upgrade")) // 固件升级事件
        {
            ESP_LOGI("hw_iot_mqtt_subscribe_type", "OTA: Firmware upgrade subscribe");
            return HW_IOT_MQTT_FIRMWARE_UPGRADE_SUBSCRIBE;
        }
    }

    return HW_IOT_MQTT_SUBSCRIBE_TYPE_INVALID;
}

int hw_iot_mqtt_subscribe_ack(hw_iot_mqtt_subscribe_type_t subscribe_type, esp_mqtt_event_handle_t receive_data)
{
    if (subscribe_type == HW_IOT_MQTT_SUBSCRIBE_TYPE_INVALID)
    {
        ESP_LOGE("hw_iot_mqtt_subscribe_ack", "Invalid subscribe type");
        return ESP_FAIL;
    }
    switch (subscribe_type)
    {
    case HW_IOT_MQTT_MESSAGE_SUBSCRIBE:
        ESP_LOGI("hw_iot_mqtt_subscribe_ack", "Message subscribe ack");
        break;
    case HW_IOT_MQTT_COMMAND_SUBSCRIBE:
        ESP_LOGI("hw_iot_mqtt_subscribe_ack", "Command subscribe ack");
        char request_id[128] = {0};                                                                                                               // 命令请求 ID
        hw_iot_mqtt_command_response_json_t command_response_json = {.result_code = 0, .response_name = "COMMAND_RESPONSE", .result = "success"}; // 命令响应 JSON 结构体
        if (hw_iot_mqtt_topic_get_command_request_id(receive_data, request_id) != ESP_OK)                                                         // 从 topic 中提取 request_id
        {
            ESP_LOGW("hw_iot_mqtt_subscribe_ack", "Failed to get request_id from topic");
            command_response_json.result_code = 1; // 作失败处理
            return ESP_FAIL;
        }
        char *command_response_topic = hw_iot_mqtt_topic_get(HW_IOT_TOPIC_COMMAND_RESPONSE, HW_IOT_DEVICE_ID, request_id); // 获取命令响应 topic
        if (command_response_topic == NULL)
        {
            ESP_LOGW("hw_iot_mqtt_subscribe_ack", "Failed to get command_response_topic");
            command_response_json.result_code = 1; // 作失败处理
            return ESP_FAIL;
        }
        char *command_response_json_str = hw_iot_mqtt_command_response_json(&command_response_json); // 生成命令响应 JSON 字符串
        ESP_LOGI("hw_iot_mqtt_subscribe_ack", "request_id: %s", request_id);
        ESP_LOGI("hw_iot_mqtt_subscribe_ack", "command_response_topic: %s", command_response_topic);
        ESP_LOGI("hw_iot_mqtt_subscribe_ack", "command_response_json_str: %s", command_response_json_str);
        hw_iot_mqtt_publish(command_response_topic, command_response_json_str); // 发布命令响应
        free(command_response_json_str);
        break;
    case HW_IOT_MQTT_VERSION_QUERY_SUBSCRIBE:
        ESP_LOGI("hw_iot_mqtt_subscribe_ack", "Version query subscribe ack");
        break;
    case HW_IOT_MQTT_FIRMWARE_UPGRADE_SUBSCRIBE:
        ESP_LOGI("hw_iot_mqtt_subscribe_ack", "Firmware upgrade subscribe ack");
        break;
    default:
        break;
    }
    return ESP_OK;
}