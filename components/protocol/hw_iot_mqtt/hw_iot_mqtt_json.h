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
    char *sw_version;       /* 软件版本 */
    char *fw_version;       /* 固件版本 */
} hw_iot_mqtt_ota_version_report_json_t;

//=============================================================================//
/* 设备上报软升级状态 */
typedef struct
{
    char *object_device_id; /* 设备 ID */
    char *version;          /* 设备当前版本号 */
    char *description;      /* 升级状态描述信息 */
    int progress;           /* 升级进度 */
    int result_code;        /* 升级状态 */
    /* 升级状态说明
    0：success（处理成功）
    1：device in use（设备使用中）
    2：already the latest version（已经是最新版本）
    3：low battery（电量不足）
    4：insufficient storage space（剩余空间不足）
    5：download timeout（下载超时）
    6：upgrade package verification failure（升级包校验失败）
    7：unsupported upgrade package type（升级包类型不支持）
    8：insufficient memory（内存不足）
    9：upgrade package installation failure（升级包安装失败）
    255：internal exception（内部异常）
    */
} hw_iot_mqtt_ota_status_report_json_t;

char *hw_iot_mqtt_properties_report_json(hw_iot_mqtt_properties_report_json_t *json);   // 事件属性上报，服务结构体转换为 JSON 字符串
char *hw_iot_mqtt_command_response_json(hw_iot_mqtt_command_response_json_t *json);     // 命令确认响应，属性结构体转换为 JSON 字符串
char *hw_iot_mqtt_ota_version_report_json(hw_iot_mqtt_ota_version_report_json_t *json); // 设备上报软固件版本，属性结构体转换为 JSON 字符串
char *hw_iot_mqtt_ota_status_report_json(hw_iot_mqtt_ota_status_report_json_t *json);   // 设备上报软升级状态，属性结构体转换为 JSON 字符串

#endif
