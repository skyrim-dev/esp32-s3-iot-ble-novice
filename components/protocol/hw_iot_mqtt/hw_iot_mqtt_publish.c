#include <esp_log.h>
#include <mqtt_client.h>

#include "hw_iot_mqtt_publish.h"
#include "hw_iot_mqtt_config.h"
#include "hw_iot_mqtt_topic.h"
#include "hw_iot_mqtt_json.h"

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

    // ESP_LOGI(TAG, "topic: %s", topic);
    // ESP_LOGI(TAG, "Payload: %s", json_str);

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

int hw_iot_mqtt_ota_version_publish(void)
{
    const char *TAG = "hw_iot_mqtt_ota_version_publish";
    hw_iot_mqtt_firmware_version_json_t json = {
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