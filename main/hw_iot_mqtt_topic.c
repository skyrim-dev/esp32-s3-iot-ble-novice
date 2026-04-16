#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hw_iot_mqtt_topic.h"

static char topic_str[128] = {0};

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