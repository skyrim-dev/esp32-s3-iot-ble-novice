#include <stdio.h>
#include <string.h>

#include <esp_log.h>
#include <mqtt_client.h>
#include <mbedtls/md5.h>
#include <mbedtls/md.h>

#include "mqtt_hw_iot.h"

esp_mqtt_client_handle_t mqtt_handle = NULL;

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
        ESP_LOGI("mqtt_hw_iot", "topic: %.*s", receive_data->topic_len, receive_data->topic);
        ESP_LOGI("mqtt_hw_iot", "data length: %d", receive_data->data_len);
        ESP_LOGI("mqtt_hw_iot", "data: %.*s", receive_data->data_len, receive_data->data);
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
    mqtt_handle = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_handle, ESP_EVENT_ANY_ID, mqtt_event_callback, NULL);
    esp_mqtt_client_start(mqtt_handle);
}

void mqtt_publish_message(const char *topic, const char *message)
{
    if (mqtt_handle != NULL)
    {
        esp_mqtt_client_publish(mqtt_handle, topic, message, strlen(message), 1, 0);
    }
}