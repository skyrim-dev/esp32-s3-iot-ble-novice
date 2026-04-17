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
        ESP_LOGE("mqtt_hw_iot", "Invalid event handle");
        return -1;
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

    if (strstr(topic, "sys/messages/down")) // 消息主题
    {
        return HW_IOT_MQTT_MESSAGE_SUBSCRIBE;
    }
    else if (strstr(topic, "sys/commands/request_id=")) // 命令主题
    {
        return HW_IOT_MQTT_COMMAND_SUBSCRIBE;
    }
    else if (strstr(json_payload, "\"service_id\": \"$ota\"")) // OTA服务主题
    {
        if (strstr(json_payload, "\"event_type\": \"version_query\"")) // 版本查询事件
        {
            return HW_IOT_MQTT_VERSION_QUERY_SUBSCRIBE;
        }
        else if (strstr(json_payload, "\"event_type\": \"firmware_upgrade\"")) // 固件升级事件
        {
            return HW_IOT_MQTT_FIRMWARE_UPGRADE_SUBSCRIBE;
        }
    }

    return -1;
}

int hw_iot_mqtt_subscribe_ack(hw_iot_mqtt_subscribe_type_t subscribe_type, esp_mqtt_event_handle_t receive_data)
{
    if (subscribe_type == HW_IOT_MQTT_SUBSCRIBE_TYPE_INVALID)
    {
        ESP_LOGE("mqtt_hw_iot", "Invalid subscribe type");
        return ESP_FAIL;
    }
    switch (subscribe_type)
    {
    case HW_IOT_MQTT_MESSAGE_SUBSCRIBE:

        break;
    case HW_IOT_MQTT_COMMAND_SUBSCRIBE:
        char request_id[128] = {0};                                                                                                               // 命令请求 ID
        hw_iot_mqtt_command_response_json_t command_response_json = {.result_code = 0, .response_name = "COMMAND_RESPONSE", .result = "success"}; // 命令响应 JSON 结构体
        if (hw_iot_mqtt_topic_get_command_request_id(receive_data, request_id) != ESP_OK)                                                         // 从 topic 中提取 request_id
        {
            ESP_LOGW("mqtt_hw_iot", "Failed to get request_id from topic");
            command_response_json.result_code = 1; // 作失败处理
            return ESP_FAIL;
        }
        char *command_response_topic = hw_iot_mqtt_topic_get(HW_IOT_TOPIC_COMMAND_RESPONSE, HW_IOT_DEVICE_ID, request_id); // 获取命令响应 topic
        if (command_response_topic == NULL)
        {
            ESP_LOGW("mqtt_hw_iot", "Failed to get command_response_topic");
            command_response_json.result_code = 1; // 作失败处理
            return ESP_FAIL;
        }
        char *command_response_json_str = hw_iot_mqtt_command_response_json(&command_response_json); // 生成命令响应 JSON 字符串
        ESP_LOGI("mqtt_hw_iot", "request_id: %s", request_id);
        ESP_LOGI("mqtt_hw_iot", "command_response_topic: %s", command_response_topic);
        ESP_LOGI("mqtt_hw_iot", "command_response_json_str: %s", command_response_json_str);
        hw_iot_mqtt_publish(command_response_topic, command_response_json_str); // 发布命令响应
        free(command_response_json_str);
        break;
    case HW_IOT_MQTT_VERSION_QUERY_SUBSCRIBE:

        break;
    case HW_IOT_MQTT_FIRMWARE_UPGRADE_SUBSCRIBE:
        break;
    default:
        break;
    }
    return ESP_OK;
}