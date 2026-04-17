#ifndef __HW_IOT_MQTT_SUBSCRIBE_H__
#define __HW_IOT_MQTT_SUBSCRIBE_H__

typedef enum
{
    HW_IOT_MQTT_SUBSCRIBE_TYPE_INVALID = 0,
    HW_IOT_MQTT_COMMAND_SUBSCRIBE,
    HW_IOT_MQTT_MESSAGE_SUBSCRIBE,
    HW_IOT_MQTT_VERSION_QUERY_SUBSCRIBE,
    HW_IOT_MQTT_FIRMWARE_UPGRADE_SUBSCRIBE,
} hw_iot_mqtt_subscribe_type_t;

hw_iot_mqtt_subscribe_type_t hw_iot_mqtt_subscribe_type(esp_mqtt_event_handle_t receive_data);
int hw_iot_mqtt_subscribe_ack(hw_iot_mqtt_subscribe_type_t subscribe_type, esp_mqtt_event_handle_t receive_data);

#endif
