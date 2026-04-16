#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_log.h>

#include "hw_iot_mqtt_topic.h"
#include "hw_iot_mqtt.h"

static char topic_str[128] = {0};

/**
 * @brief 获取华为云IoT平台的MQTT主题字符串
 *
 * 根据主题类型、设备ID和请求ID生成符合华为云IoT平台规范的MQTT主题字符串。
 * 支持的主题类型包括：属性上报、命令响应、版本上报等。
 *
 * @param type 主题类型，枚举值定义如下：
 *             - HW_IOT_TOPIC_PROPERTIES_REPORT: 属性上报主题
 *             - HW_IOT_TOPIC_COMMAND_RESPONSE: 命令响应主题
 *             - HW_IOT_TOPIC_VERSION_REPORT: 版本上报主题
 * @param device_id 设备ID字符串，不能为NULL
 * @param request_id 请求ID字符串，仅在命令响应主题时使用，其他类型可为NULL
 *
 * @return char* 成功返回指向静态缓冲区的主题字符串指针，失败返回NULL
 *               注意：返回的指针指向静态缓冲区，后续调用会覆盖之前的内容
 *
 * @note
 *       1. 函数内部使用静态缓冲区存储生成的主题字符串，大小为128字节
 *       2. 每次调用都会清空缓冲区并重新生成主题
 *       3. 如果生成的主题字符串长度超过缓冲区大小，则不会生成主题
 *       4. device_id参数必须有效，否则返回NULL
 *       5. 返回的字符串不需要调用者释放，但需要在使用前复制到其他位置
 */
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
    case HW_IOT_TOPIC_VERSION_REPORT:
        required_len = snprintf(NULL, 0, "$oc/devices/%s/sys/events/up", device_id); // 版本上报主题长度
        if (required_len < sizeof(topic_str))                                        // 版本上报主题长度小于缓冲区大小
        {
            snprintf(topic_str, sizeof(topic_str), "$oc/devices/%s/sys/events/up", device_id);
        }
        break;
    }
    return topic_str;
}

/**
 * @brief 从MQTT命令主题中提取请求ID
 *
 * 该函数从华为云IoT平台下发的命令主题中解析并提取request_id。
 * 命令主题格式：$oc/devices/{device_id}/sys/commands/request_id={request_id}/...
 *
 * @param receive_data MQTT事件数据指针，包含接收到的主题信息
 *                    - receive_data->topic: 主题字符串
 *                    - receive_data->topic_len: 主题长度
 * @param request_id 用于存储提取的请求ID的缓冲区指针
 *                   调用者需要提供足够大的缓冲区（建议128字节）
 *
 * @return int 成功返回ESP_OK，失败返回ESP_FAIL
 *
 * @note
 *       1. 调用者必须提供有效的缓冲区，建议大小为128字节
 *       2. 函数内部会验证主题格式，确保包含"sys/commands/request_id="
 *       3. 如果主题格式不正确或参数无效，函数会返回ESP_FAIL
 *       4. 提取的request_id会以空字符结尾
 *       5. 函数会记录接收到的完整主题和提取的request_id到日志
 *       6. 适用于MQTT事件回调函数中使用
 */
int hw_iot_mqtt_topic_get_command_request_id(esp_mqtt_event_handle_t receive_data, char *request_id)
{
    if (!receive_data || !strstr(receive_data->topic, "sys/commands/request_id="))
    {
        ESP_LOGW("hw_iot_mqtt_topic_get_command_request_id", "Invalid command topic");
        return ESP_FAIL;
    }
    
    ESP_LOGI("hw_iot_mqtt_topic_get_command_request_id", "receive command, topic: %.*s", receive_data->topic_len, receive_data->topic);

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
        ESP_LOGI("hw_iot_mqtt_topic_get_command_request_id", "request_id: %s", request_id);
    }
    return ESP_OK;
}