#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <mqtt_client.h>
#include <driver/gpio.h>
#include <cJSON.h>

#include "hw_iot_mqtt.h"
#include "mqtt_hw_iot_command_receive.h"
#include "hw_iot_mqtt_topic.h"

static const char *TAG = "hw_iot_cmd";

hw_iot_cmd_receive_t hw_iot_cmd_receive;

void hw_iot_command_parse(esp_mqtt_event_handle_t receive_data)
{
    if (!receive_data)
    {
        return;
    }

    ESP_LOGI(TAG, "receive command, topic: %.*s", receive_data->topic_len, receive_data->topic);

    int copy_len = (receive_data->topic_len < sizeof(hw_iot_cmd_receive.topic) - 1)
                       ? receive_data->topic_len
                       : sizeof(hw_iot_cmd_receive.topic) - 1;
    memcpy(hw_iot_cmd_receive.topic, receive_data->topic, copy_len);
    hw_iot_cmd_receive.topic[copy_len] = '\0';

    char *ptr = strstr(hw_iot_cmd_receive.topic, "request_id=");
    if (ptr)
    {
        ptr += strlen("request_id=");
        char *end = strchr(ptr, '/');
        int len = end ? (end - ptr) : strlen(ptr);
        if (len < sizeof(hw_iot_cmd_receive.request_id))
        {
            memcpy(hw_iot_cmd_receive.request_id, ptr, len);
            hw_iot_cmd_receive.request_id[len] = '\0';
            ESP_LOGI(TAG, "request_id: %s", hw_iot_cmd_receive.request_id);
        }
    }

    hw_iot_cmd_receive.command_js = cJSON_Parse(receive_data->data);
    if (!hw_iot_cmd_receive.command_js)
    {
        ESP_LOGE(TAG, "Failed to parse command JSON");
        return;
    }

    hw_iot_cmd_receive.paras = cJSON_GetObjectItem(hw_iot_cmd_receive.command_js, "paras");
    hw_iot_cmd_receive.service_id = cJSON_GetObjectItem(hw_iot_cmd_receive.command_js, "service_id");
    hw_iot_cmd_receive.command_name = cJSON_GetObjectItem(hw_iot_cmd_receive.command_js, "command_name");
    hw_iot_cmd_receive.object_device_id = cJSON_GetObjectItem(hw_iot_cmd_receive.command_js, "object_device_id");

    if (hw_iot_cmd_receive.paras)
    {
        cJSON *value = cJSON_GetObjectItem(hw_iot_cmd_receive.paras, "value");
        if (value)
        {
            const char *cmd_value = cJSON_GetStringValue(value);
            ESP_LOGI(TAG, "command value: %s", cmd_value);

            if (strcmp(cmd_value, "ON") == 0)
            {
                gpio_set_level(GPIO_NUM_1, 0);
                hw_iot_command_receive_ack(hw_iot_cmd_receive.request_id);
            }
            else if (strcmp(cmd_value, "OFF") == 0)
            {
                gpio_set_level(GPIO_NUM_1, 1);
                hw_iot_command_receive_ack(hw_iot_cmd_receive.request_id);
            }
        }
    }

    if (hw_iot_cmd_receive.command_js)
    {
        cJSON_Delete(hw_iot_cmd_receive.command_js);
        hw_iot_cmd_receive.command_js = NULL;
    }
}

void hw_iot_command_receive_ack(char *request_id)
{
    if (!request_id || strlen(request_id) == 0)
    {
        ESP_LOGE(TAG, "request_id is NULL or empty");
        return;
    }

    if (!mqtt_handle)
    {
        ESP_LOGE(TAG, "MQTT client not initialized");
        return;
    }

    ESP_LOGI(TAG, "request_id: %s", request_id);

    cJSON *response_json = cJSON_CreateObject();
    if (!response_json)
    {
        ESP_LOGE(TAG, "Failed to create response JSON");
        return;
    }

    cJSON_AddNumberToObject(response_json, "result_code", 0);
    cJSON_AddStringToObject(response_json, "response_name", "COMMAND_RESPONSE");

    cJSON *paras = cJSON_CreateObject();
    if (paras)
    {
        cJSON_AddStringToObject(paras, "result", "success");
        cJSON_AddItemToObject(response_json, "paras", paras);
    }

    char *json_str = cJSON_PrintUnformatted(response_json);
    if (!json_str)
    {
        ESP_LOGE(TAG, "Failed to stringify response JSON");
        cJSON_Delete(response_json);
        return;
    }

    char *topic = hw_iot_mqtt_topic_get(HW_IOT_TOPIC_COMMAND_RESPONSE, HW_IOT_USERNAME, request_id);
    if (!topic)
    {
        ESP_LOGE(TAG, "Failed to get command response topic");
        cJSON_free(json_str);
        cJSON_Delete(response_json);
        return;
    }

    ESP_LOGI(TAG, "Response topic: %s", topic);
    ESP_LOGI(TAG, "Response payload: %s", json_str);

    int msg_id = esp_mqtt_client_publish(mqtt_handle, topic, json_str, strlen(json_str), 0, 0);
    if (msg_id < 0)
    {
        ESP_LOGE(TAG, "Failed to publish ACK, msg_id=%d", msg_id);
    }
    else
    {
        ESP_LOGI(TAG, "ACK published successfully, msg_id=%d", msg_id);
    }

    cJSON_free(json_str);
    cJSON_Delete(response_json);
}
