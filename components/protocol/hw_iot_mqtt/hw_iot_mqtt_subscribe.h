#ifndef __HW_IOT_MQTT_SUBSCRIBE_H__
#define __HW_IOT_MQTT_SUBSCRIBE_H__

typedef enum
{
    HW_IOT_MQTT_SUBSCRIBE_TYPE_INVALID = 0,     // 无效订阅类型
    HW_IOT_MQTT_COMMAND_SUBSCRIBE = 1,          // 命令订阅
    HW_IOT_MQTT_MESSAGE_SUBSCRIBE = 2,          // 消息订阅
    HW_IOT_MQTT_VERSION_QUERY_SUBSCRIBE = 3,    // 版本查询订阅
    HW_IOT_MQTT_FIRMWARE_UPGRADE_SUBSCRIBE = 4, // 固件升级订阅
} hw_iot_mqtt_subscribe_type_t;

hw_iot_mqtt_subscribe_type_t hw_iot_mqtt_subscribe_type(esp_mqtt_event_handle_t receive_data);
int hw_iot_mqtt_subscribe_ack(hw_iot_mqtt_subscribe_type_t subscribe_type, esp_mqtt_event_handle_t receive_data);

#endif
