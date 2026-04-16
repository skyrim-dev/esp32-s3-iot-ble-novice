#ifndef HW_IOT_PROTOCOL_H
#define HW_IOT_PROTOCOL_H

#include <mqtt_client.h>



typedef enum
{
    HW_IOT_TOPIC_PROPERTIES_REPORT = 0, /**< 属性上报 $oc/devices/{device_id}/sys/properties/report*/
    HW_IOT_TOPIC_COMMAND_RESPONSE,      /**< 命令确认响应 $oc/devices/{device_id}/sys/commands/response/request_id={request_id} */
    HW_IOT_TOPIC_VERSION_REPORT,        /**< 版本上报 $oc/devices/{device_id}/sys/events/up */
} hw_iot_topic_type_t;

char *hw_iot_mqtt_topic_get(hw_iot_topic_type_t type, char *device_id, char *request_id);             // 获取指定类型的Topic字符串
int hw_iot_mqtt_topic_get_command_request_id(esp_mqtt_event_handle_t receive_data, char *request_id); // 获取命令请求ID

#endif // HW_IOT_MQTT_TOPIC_H
