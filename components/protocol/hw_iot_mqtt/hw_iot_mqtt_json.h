#ifndef HW_IOT_MQTT_JSON_H
#define HW_IOT_MQTT_JSON_H

#include <cJSON.h>
#include <string.h>
#include <stdbool.h>

#define HW_IOT_MQTT_SERVICES_NUM 10   /* 服务上报 json 服务数量 */
#define HW_IOT_MQTT_PROPERTIES_NUM 10 /* 属性上报 json 属性数量 */

//=============================================================================//
/* 事件属性上报，属性结构体 */
typedef struct
{
    char *service_id;                                    /* 服务 ID */
    char *properties_id[HW_IOT_MQTT_PROPERTIES_NUM];     /* 事件属性 ID 数组 */
    double properties_value[HW_IOT_MQTT_PROPERTIES_NUM]; /* 事件属性值 数组 */
} hw_iot_mqtt_properties_json_t;
/* 事件属性上报，服务结构体 */
typedef struct
{
    hw_iot_mqtt_properties_json_t json[HW_IOT_MQTT_SERVICES_NUM]; /* 属性结构体数组 */
} hw_iot_mqtt_properties_report_json_t;

//=============================================================================//
/* 命令确认响应，属性结构体 */
typedef struct
{
    int result_code;     /* 标识命令的执行结果，0：成功，1：失败 */
    char *response_name; /* 响应名称 */
    char *result;        /* 命令的响应参数 */
} hw_iot_mqtt_command_response_json_t;

//=============================================================================//
/* 设备上报软固件版本 */
typedef struct
{
    char *object_device_id; /* 设备 ID */
    char *event_time;       /* 事件时间 */
    char *sw_version;       /* 软件版本 */
    char *fw_version;       /* 固件版本 */
} hw_iot_mqtt_firmware_version_json_t;

char *hw_iot_mqtt_properties_report_json(hw_iot_mqtt_properties_report_json_t *json); // 事件属性上报，服务结构体转换为 JSON 字符串
char *hw_iot_mqtt_command_response_json(hw_iot_mqtt_command_response_json_t *json);   // 命令确认响应，属性结构体转换为 JSON 字符串
char *hw_iot_mqtt_ota_version_report_json(hw_iot_mqtt_firmware_version_json_t *json);     // 设备上报软固件版本，属性结构体转换为 JSON 字符串
char *cJSON_UnformattedFree(cJSON *root_js);                                          // 将cJSON对象序列化为无格式JSON字符串并释放对象

#endif
