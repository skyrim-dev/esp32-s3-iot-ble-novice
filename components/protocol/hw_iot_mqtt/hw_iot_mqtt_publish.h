#ifndef __HW_IOT_MQTT_PUBLISH_H__
#define __HW_IOT_MQTT_PUBLISH_H__

#include <esp_err.h>
#include "hw_iot_mqtt_json.h"

esp_err_t hw_iot_mqtt_publish(char *topic, char *json_str);
esp_err_t hw_iot_mqtt_properties_publish(void);
esp_err_t hw_iot_mqtt_command_report(char *request_id);
esp_err_t hw_iot_mqtt_ota_version_report_publish(void);
esp_err_t hw_iot_mqtt_ota_status_report_publish(const hw_iot_mqtt_ota_status_report_json_t *report);

#endif
