#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_log.h>
#include <esp_err.h>

#include "hw_iot_mqtt_topic.h"
#include "hw_iot_mqtt_client.h"

static char topic_str[256] = {0};

char *hw_iot_mqtt_topic_get(hw_iot_topic_type_t type, char *device_id, char *request_id)
{
    memset(topic_str, 0, sizeof(topic_str)); // 清空静态缓冲区
    if (!device_id)
    {
        return NULL;
    }
    int required_len = 0;
    switch (type)
    {
    case HW_IOT_TOPIC_PROPERTIES_REPORT:
        required_len = snprintf(NULL, 0, "$oc/devices/%s/sys/properties/report", device_id); // 属性上报主题长度
        if (required_len < sizeof(topic_str))                                                // 属性上报主题长度小于缓冲区大小
        {
            snprintf(topic_str, sizeof(topic_str), "$oc/devices/%s/sys/properties/report", device_id);
        }
        break;
    case HW_IOT_TOPIC_COMMAND_RESPONSE:
        required_len = snprintf(NULL, 0, "$oc/devices/%s/sys/commands/response/request_id=%s", device_id, request_id); // 命令确认响应主题长度
        if (required_len < sizeof(topic_str))                                                                          // 命令确认响应主题长度小于缓冲区大小
        {
            snprintf(topic_str, sizeof(topic_str), "$oc/devices/%s/sys/commands/response/request_id=%s", device_id, request_id);
        }
        break;
    case HW_IOT_TOPIC_OTA_VERSION_REPORT:
        required_len = snprintf(NULL, 0, "$oc/devices/%s/sys/events/up", device_id); // 版本上报主题长度
        if (required_len < sizeof(topic_str))                                        // 版本上报主题长度小于缓冲区大小
        {
            snprintf(topic_str, sizeof(topic_str), "$oc/devices/%s/sys/events/up", device_id);
        }
        break;
    }
    return topic_str;
}

esp_err_t hw_iot_mqtt_topic_get_command_request_id(esp_mqtt_event_handle_t receive_data, char *request_id)
{
    const char *TAG = "hw_iot_mqtt_topic_get_command_request_id";
    if (!receive_data || !strstr(receive_data->topic, "sys/commands/request_id="))
    {
        ESP_LOGW(TAG, "Invalid command topic");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "receive command, topic: %.*s", receive_data->topic_len, receive_data->topic);

    char topic[128] = {0};
    int copy_len = (receive_data->topic_len < sizeof(topic) - 1)
                       ? receive_data->topic_len
                       : sizeof(topic) - 1;       // 计算要复制的长度，确保不超过缓冲区大小
    memcpy(topic, receive_data->topic, copy_len); // 复制主题字符串到本地缓冲区
    topic[copy_len] = '\0';                       // 确保字符串以空字符结尾
    char *ptr = strstr(topic, "request_id=");     // 查找请求ID字符串
    if (ptr)
    {
        ptr += strlen("request_id=");              // 跳过"request_id="前缀
        char *end = strchr(ptr, '/');              // 查找请求ID后面的斜杠，确定请求ID的结束位置
        int len = end ? (end - ptr) : strlen(ptr); // 计算请求ID的长度，如果没有斜杠，则使用剩余字符串的长度
        memcpy(request_id, ptr, len);              // 复制请求ID到本地缓冲区
        request_id[len] = '\0';                    // 确保字符串以空字符结尾
        ESP_LOGI(TAG, "request_id: %s", request_id);
    }
    return ESP_OK;
}