#ifndef __HW_IOT_MQTT_PUBLISH_H__
#define __HW_IOT_MQTT_PUBLISH_H__

int hw_iot_mqtt_publish(char *topic, char *json_str);
int hw_iot_mqtt_properties_publish(void);
int hw_iot_mqtt_ota_version_publish(void);

#endif

