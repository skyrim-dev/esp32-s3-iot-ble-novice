#ifndef HW_IOT_MQTT_TOPIC_H
#define HW_IOT_MQTT_TOPIC_H

#define DEVICE_ID "69cc7e9c6b6c4d5f8d58bd94_3c-84-27-c0-2e-6c"

typedef enum
{
    HW_IOT_TOPIC_PROPERTIES_REPORT = 0, /**< 属性上报 $oc/devices/{device_id}/sys/properties/report*/
    HW_IOT_TOPIC_VERSION_REPORT,        /**< 版本上报 $oc/devices/{device_id}/sys/events/up */
    HW_IOT_TOPIC_ACK_RESPONSE,          /**< 命令确认响应 $oc/devices/{device_id}/sys/commands/response/request_id={request_id} */
} hw_iot_topic_type_t;

char *hw_iot_mqtt_topic_get(hw_iot_topic_type_t type, char *device_id, char *request_id); // 获取指定类型的Topic字符串

#endif // HW_IOT_MQTT_TOPIC_H
