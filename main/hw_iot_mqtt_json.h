#ifndef HW_IOT_MQTT_JSON_H
#define HW_IOT_MQTT_JSON_H

#include <cJSON.h>
#include <string.h>

#define HW_IOT_MQTT_SERVICES_NUM 1   /* 服务上报 json 服务数量 */
#define HW_IOT_MQTT_PROPERTIES_NUM 1 /* 属性上报 json 属性数量 */

/* 事件属性上报，属性结构体 */
typedef struct
{
    char *service_id;                                   /* 服务 ID */
    char *properties_id[HW_IOT_MQTT_PROPERTIES_NUM];    /* 事件属性 ID 数组 */
    double properties_value[HW_IOT_MQTT_PROPERTIES_NUM]; /* 事件属性值 数组 */
} hw_iot_mqtt_properties_json_t;

/* 事件属性上报，服务结构体 */
typedef struct
{
    hw_iot_mqtt_properties_json_t json[HW_IOT_MQTT_SERVICES_NUM]; /* 属性结构体数组 */
} hw_iot_mqtt_properties_report_json_t;

char *hw_iot_mqtt_properties_report_json(hw_iot_mqtt_properties_report_json_t *json);

#endif
