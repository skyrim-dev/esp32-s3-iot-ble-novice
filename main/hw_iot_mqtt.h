#ifndef HW_IOT_MQTT_H
#define HW_IOT_MQTT_H

#include <mqtt_client.h>

#define HW_IOT_PRODUCTKEY "69cc7e9c6b6c4d5f8d58bd94"
#define HW_IOT_CLIENT_ID "69cc7e9c6b6c4d5f8d58bd94_3c-84-27-c0-2e-6c_0_0_2026040109"
#define HW_IOT_USERNAME "69cc7e9c6b6c4d5f8d58bd94_3c-84-27-c0-2e-6c"
#define HW_IOT_PASSWORD "57c89eb42bd6edf2c42b50fdd03ce1d35dc82cd4ce091918fa9ad92e219d5c2e"
#define HW_IOT_HOSTNAME "f1614c4895.iotda-device.cn-south-4.myhuaweicloud.com"
#define HW_IOT_URI "mqtts://" HW_IOT_HOSTNAME ":8883"
#define HW_IOT_PORT 8883

extern esp_mqtt_client_handle_t mqtt_handle;

void mqtt_event_callback(void *event_handler_arg,
                         esp_event_base_t event_base,
                         int32_t event_id,
                         void *event_data);
int hw_iot_mqtt_report(char *topic, char *json_str);
void hw_iot_mqtt_init(void);

#endif