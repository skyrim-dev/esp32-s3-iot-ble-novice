#include <mqtt_client.h>
#include <string.h>
#include <esp_log.h>

#include "hw_iot_mqtt_subscribe.h"
#include "hw_iot_mqtt_json.h"
#include "hw_iot_mqtt_topic.h"
#include "hw_iot_mqtt_client.h"
#include "hw_iot_mqtt_publish.h"

hw_iot_mqtt_subscribe_type_t hw_iot_mqtt_subscribe_type(esp_mqtt_event_handle_t receive_data)
{
    char *TAG = "hw_iot_mqtt_subscribe_type";
    if (!receive_data)
    {
        ESP_LOGE(TAG, "Invalid event handle");
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

    ESP_LOGI(TAG, "topic length: %d", receive_data->topic_len);
    ESP_LOGI(TAG, "data length: %d", receive_data->data_len);
    ESP_LOGI(TAG, "topic: %.*s", receive_data->topic_len, topic);
    ESP_LOGI(TAG, "data: %.*s", receive_data->data_len, json_payload);

    if (strstr(topic, "sys/messages/down")) // 消息主题
    {
        ESP_LOGI(TAG, "Message subscribe");
        return HW_IOT_MQTT_MESSAGE_SUBSCRIBE;
    }
    else if (strstr(topic, "sys/commands/request_id=")) // 命令主题
    {
        ESP_LOGI(TAG, "Command subscribe");
        return HW_IOT_MQTT_COMMAND_SUBSCRIBE;
    }
    else if (strstr(json_payload, "$ota") && strstr(topic, "sys/events/down")) // OTA服务主题
    {
        ESP_LOGI(TAG, "OTA subscribe");
        if (strstr(json_payload, "version_query")) // 版本查询事件
        {
            ESP_LOGI(TAG, "OTA: Version query subscribe");
            return HW_IOT_MQTT_OTA_VERSION_QUERY_SUBSCRIBE;
        }
        else if (strstr(json_payload, "firmware_upgrade")) // 软/固件升级事件
        {
            ESP_LOGI(TAG, "OTA: Firmware upgrade subscribe");
            return HW_IOT_MQTT_OTA_SFW_UPGRADE_SUBSCRIBE;
        }
    }

    return HW_IOT_MQTT_SUBSCRIBE_TYPE_INVALID;
}

