#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <mqtt_client.h>
#include <cJSON.h>

#include "mqtt_hw_iot.h"
#include "mqtt_hw_iot_command_receive.h"

static const char *TAG = "hw_iot_cmd_ack";

hw_iot_cmd_receive_t hw_iot_cmd_receive;

/**
 * @brief 发送命令响应ACK到华为云IoT平台
 *
 * 当设备接收到平台下发的命令后，需要发送响应告知平台命令执行结果。
 * 根据华为云IoTDA平台API规范，响应Topic格式为：
 *   $oc/devices/{device_id}/sys/commands/response/request_id={request_id}
 *
 * 响应JSON格式：
 *   {
 *       "result_code": 0,           // 0表示成功，其他表示失败
 *       "response_name": "COMMAND_RESPONSE",
 *       "paras": { ... }            // 响应参数（可选）
 *   }
 *
 * @param request_id 平台下发命令时携带的request_id（cJSON对象）
 */
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

    char topic[128];
    snprintf(topic, sizeof(topic), "$oc/devices/%s/sys/commands/response/request_id=%s",
             HW_IOT_USERNAME, request_id);

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
