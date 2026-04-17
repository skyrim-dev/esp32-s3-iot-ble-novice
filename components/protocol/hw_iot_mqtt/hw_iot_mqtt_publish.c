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