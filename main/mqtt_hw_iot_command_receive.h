#ifndef __MQTT_HW_IOT_COMMAND_RECEIVE_H__
#define __MQTT_HW_IOT_COMMAND_RECEIVE_H__

#include <mqtt_client.h>
#include <cJSON.h>

typedef struct
{
    char topic[128];
    char request_id[128];
    cJSON *command_js;
    cJSON *object_device_id;
    cJSON *service_id;
    cJSON *command_name;
    cJSON *paras;
} hw_iot_cmd_receive_t;

extern hw_iot_cmd_receive_t hw_iot_cmd_receive;

void hw_iot_command_parse(esp_mqtt_event_handle_t receive_data);
void hw_iot_command_receive_ack(char *request_id);

#endif
