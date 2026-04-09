#include <stdio.h>
#include <string.h>

#include <esp_log.h>
#include <mqtt_client.h>
#include <mbedtls/md5.h>
#include <mbedtls/md.h>
#include <driver/gpio.h>
#include <cJSON.h>

#include "mqtt_hw_iot_command_receive.h"
#include "mqtt_hw_iot.h"

esp_mqtt_client_handle_t mqtt_handle = NULL;

extern const char *hw_iot_cert;

void mqtt_event_callback(void *event_handler_arg,
                         esp_event_base_t event_base,
                         int32_t event_id,
                         void *event_data)
{
    esp_mqtt_event_handle_t receive_data = event_data;
    switch (event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI("mqtt_hw_iot", "Connected to broker");
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI("mqtt_hw_iot", "Disconnected from broker");
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI("mqtt_hw_iot", "mqtt publish ack");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI("mqtt_hw_iot", "ESP32 Subscribed ack");
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI("mqtt_hw_iot", "topic length: %d", receive_data->topic_len);
        ESP_LOGI("mqtt_hw_iot", "data length: %d", receive_data->data_len);
        ESP_LOGI("mqtt_hw_iot", "topic: %.*s", receive_data->topic_len, receive_data->topic);
        ESP_LOGI("mqtt_hw_iot", "data: %.*s", receive_data->data_len, receive_data->data);
        if (strstr(receive_data->topic, "/sys/commands/"))
        {
            ESP_LOGI("mqtt_hw_iot", "receive topic and data");
            /* 解析字符串 */
            /* 解析Topic */
            int copy_len = (receive_data->topic_len < sizeof(hw_iot_cmd_receive.topic) - 1
                                ? receive_data->topic_len
                                : sizeof(hw_iot_cmd_receive.topic) - 1);     // 检查topic长度是否超过数组大小
            memcpy(hw_iot_cmd_receive.topic, receive_data->topic, copy_len); // 复制topic字符串到hw_iot_cmd_receive.topic数组
            hw_iot_cmd_receive.topic[copy_len] = '\0';                       // 添加字符串结束符
            char *ptr = strstr(hw_iot_cmd_receive.topic, "request_id=");     // Topic 格式: $oc/devices/{device_id}/sys/commands/request_id={request_id}
            if (ptr)
            {
                ptr += strlen("request_id="); // 跳过 "request_id="
                // 找到下一个 '/' 或字符串结尾
                char *end = strchr(ptr, '/');
                int len = end ? (end - ptr) : strlen(ptr);
                if (len < sizeof(hw_iot_cmd_receive.request_id))
                {
                    memcpy(hw_iot_cmd_receive.request_id, ptr, len);
                    hw_iot_cmd_receive.request_id[len] = '\0';
                    ESP_LOGI("mqtt_hw_iot", "request_id: %s", hw_iot_cmd_receive.request_id);
                }
            }
            ESP_LOGI("mqtt_hw_iot", "topic: %s", hw_iot_cmd_receive.topic);
            /* 解析Command */
            hw_iot_cmd_receive.command_js = cJSON_Parse(receive_data->data);                                              // 解析JSON字符串
            hw_iot_cmd_receive.paras = cJSON_GetObjectItem(hw_iot_cmd_receive.command_js, "paras");                       // 获取paras字段
            hw_iot_cmd_receive.service_id = cJSON_GetObjectItem(hw_iot_cmd_receive.command_js, "service_id");             // 获取service_id字段
            hw_iot_cmd_receive.command_name = cJSON_GetObjectItem(hw_iot_cmd_receive.command_js, "command_name");         // 获取command_name字段
            hw_iot_cmd_receive.object_device_id = cJSON_GetObjectItem(hw_iot_cmd_receive.command_js, "object_device_id"); // 获取object_device_id字段
            if (hw_iot_cmd_receive.paras != NULL)
            {
                cJSON *LightControl = cJSON_GetObjectItem(hw_iot_cmd_receive.paras, "value");
                if (LightControl != NULL)
                {
                    ESP_LOGI("mqtt_hw_iot", "LightControl: %s", cJSON_GetStringValue(LightControl));
                    if (strcmp(cJSON_GetStringValue(LightControl), "ON") == 0) // 对比字符串是否相等
                    {
                        if (gpio_set_level(GPIO_NUM_1, 0) == ESP_OK) // 对比字符串是否相等
                        {
                            hw_iot_command_receive_ack(hw_iot_cmd_receive.request_id);
                        }
                    }
                    else if (strcmp(cJSON_GetStringValue(LightControl), "OFF") == 0)
                    {
                        if (gpio_set_level(GPIO_NUM_1, 1) == ESP_OK) // 对比字符串是否相等
                        {
                            hw_iot_command_receive_ack(hw_iot_cmd_receive.request_id);
                        }
                    }
                }
            }
        }
        break;
    default:
        break;
    }
}

void mqtt_hw_iot_init(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {0};
    mqtt_cfg.broker.address.uri = HW_IOT_URI;
    mqtt_cfg.broker.address.port = HW_IOT_PORT;
    mqtt_cfg.credentials.client_id = HW_IOT_CLIENT_ID;
    mqtt_cfg.credentials.username = HW_IOT_USERNAME;
    mqtt_cfg.credentials.authentication.password = HW_IOT_PASSWORD;
    mqtt_cfg.broker.verification.certificate = hw_iot_cert; // 验证证书
    mqtt_handle = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_handle, ESP_EVENT_ANY_ID, mqtt_event_callback, NULL);
    esp_mqtt_client_start(mqtt_handle);
}

const char *hw_iot_cert = "-----BEGIN CERTIFICATE-----\n"
                          "MIIDXzCCAkegAwIBAgILBAAAAAABIVhTCKIwDQYJKoZIhvcNAQELBQAwTDEgMB4G"
                          "A1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjMxEzARBgNVBAoTCkdsb2JhbFNp"
                          "Z24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDkwMzE4MTAwMDAwWhcNMjkwMzE4"
                          "MTAwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMzETMBEG"
                          "A1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQYJKoZI"
                          "hvcNAQEBBQADggEPADCCAQoCggEBAMwldpB5BngiFvXAg7aEyiie/QV2EcWtiHL8"
                          "RgJDx7KKnQRfJMsuS+FggkbhUqsMgUdwbN1k0ev1LKMPgj0MK66X17YUhhB5uzsT"
                          "gHeMCOFJ0mpiLx9e+pZo34knlTifBtc+ycsmWQ1z3rDI6SYOgxXG71uL0gRgykmm"
                          "KPZpO/bLyCiR5Z2KYVc3rHQU3HTgOu5yLy6c+9C7v/U9AOEGM+iCK65TpjoWc4zd"
                          "QQ4gOsC0p6Hpsk+QLjJg6VfLuQSSaGjlOCZgdbKfd/+RFO+uIEn8rUAVSNECMWEZ"
                          "XriX7613t2Saer9fwRPvm2L7DWzgVGkWqQPabumDk3F2xmmFghcCAwEAAaNCMEAw"
                          "DgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYEFI/wS3+o"
                          "LkUkrk1Q+mOai97i3Ru8MA0GCSqGSIb3DQEBCwUAA4IBAQBLQNvAUKr+yAzv95ZU"
                          "RUm7lgAJQayzE4aGKAczymvmdLm6AC2upArT9fHxD4q/c2dKg8dEe3jgr25sbwMp"
                          "jjM5RcOO5LlXbKr8EpbsU8Yt5CRsuZRj+9xTaGdWPoO4zzUhw8lo/s7awlOqzJCK"
                          "6fBdRoyV3XpYKBovHd7NADdBj+1EbddTKJd+82cEHhXXipa0095MJ6RMG3NzdvQX"
                          "mcIfeg7jLQitChws/zyrVQ4PkX4268NXSb7hLi18YIvDQVETI53O9zJrlAGomecs"
                          "Mx86OyXShkDOOyyGeMlhLxS67ttVb9+E7gUJTb0o2HLO02JQZR7rkpeDMdmztcpH"
                          "WD9f\n"
                          "-----END CERTIFICATE-----";