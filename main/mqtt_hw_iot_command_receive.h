#ifndef __MQTT_HW_IOT_COMMAND_RECEIVE_H__
#define __MQTT_HW_IOT_COMMAND_RECEIVE_H__

#include <cJSON.h>

typedef struct
{
    char topic[128];         // Topic
    char request_id[128];    // Topic 请求ID字段
    cJSON *command_js;       // 命令JSON对象
    cJSON *object_device_id; // 设备ID字段
    cJSON *service_id;       // 服务ID字段
    cJSON *command_name;     // 命令名称字段
    cJSON *paras;            // 参数字段
} hw_iot_cmd_receive_t;

extern hw_iot_cmd_receive_t hw_iot_cmd_receive;

void hw_iot_command_receive_ack(char *request_id);

#endif
