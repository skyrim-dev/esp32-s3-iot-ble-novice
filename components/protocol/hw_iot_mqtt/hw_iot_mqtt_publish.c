#include <esp_log.h>
#include <mqtt_client.h>

#include "hw_iot_mqtt_publish.h"
#include "hw_iot_mqtt_config.h"
#include "hw_iot_mqtt_topic.h"
#include "hw_iot_mqtt_json.h"

int hw_iot_mqtt_publish(char *topic, char *json_str)
{
    char *TAG = "hw_iot_mqtt_publish";
    if (!topic || !json_str || strlen(json_str) <= 0) // 参数为空
    {
        ESP_LOGE(TAG, "Invalid message data");
        return ESP_FAIL;
    }

    if (!mqtt_handle) // 因为 mqtt_handle 是全局变量，所以可以检测 mqtt 客户端是否初始化
    {
        ESP_LOGE(TAG, "MQTT client not initialized");
        return ESP_FAIL;
    }

    /* 发布MQTT消息 (QoS=0) */
    int msg_id = esp_mqtt_client_publish(mqtt_handle, topic, json_str, strlen(json_str), 0, 0);
    if (msg_id < 0) // 发布消息失败
    {
        ESP_LOGE(TAG, "Failed to publish message, msg_id=%d", msg_id);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "topic: %s", topic);
    ESP_LOGI(TAG, "Payload: %s", json_str);

    return ESP_OK;
}

int hw_iot_mqtt_properties_publish(void)
{
    const char *TAG = "hw_iot_mqtt_properties_publish";
    hw_iot_mqtt_properties_report_json_t json = {
        {
            {"BasicData", {"luminance"}, {100}},
        }};
    char *topic = hw_iot_mqtt_topic_get(HW_IOT_TOPIC_PROPERTIES_REPORT, HW_IOT_DEVICE_ID, NULL);
    char *json_str = hw_iot_mqtt_properties_report_json(&json);
    if (hw_iot_mqtt_publish(topic, json_str) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to publish properties report");
        return ESP_FAIL;
    }
    free(json_str);
    return ESP_OK;
}

int hw_iot_mqtt_command_report(char *request_id)
{
    const char *TAG = "hw_iot_mqtt_command_report";
    if (!request_id) // request_id 不能为空
    {
        ESP_LOGE(TAG, "Invalid request_id");
        return ESP_FAIL;
    }
    hw_iot_mqtt_command_response_json_t command_response_json = {
        .result_code = 0,
        .response_name = "COMMAND_RESPONSE",
        .result = "success",
    };
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
    return ESP_OK;
}

int hw_iot_mqtt_ota_version_publish(void)
{
    const char *TAG = "hw_iot_mqtt_ota_version_publish";
    hw_iot_mqtt_ota_response_version_json_t json = {
        .object_device_id = HW_IOT_DEVICE_ID,
        .sw_version = "1.0.0",
        .fw_version = "1.0.0",
    };
    char *topic = hw_iot_mqtt_topic_get(HW_IOT_TOPIC_OTA_VERSION_REPORT, HW_IOT_DEVICE_ID, NULL);
    char *json_str = hw_iot_mqtt_ota_version_report_json(&json);
    if (hw_iot_mqtt_publish(topic, json_str) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to publish firmware version report");
        return ESP_FAIL;
    }
    free(json_str);
    return ESP_OK;
}