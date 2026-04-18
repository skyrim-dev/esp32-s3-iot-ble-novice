#include <stdio.h>
#include <string.h>

#include <esp_log.h>
#include <mqtt_client.h>
#include <cJSON.h>
#include <esp_ota_ops.h>
#include <esp_err.h>

#include "hw_iot_mqtt_client.h"
#include "hw_iot_mqtt_json.h"
#include "hw_iot_mqtt_topic.h"
#include "hw_iot_mqtt_subscribe.h"
#include "hw_iot_mqtt_publish.h"
#include "ota_manager.h"

extern const char _binary_cert_pem_start[] asm("_binary_cert_pem_start");
extern const char _binary_cert_pem_end[] asm("_binary_cert_pem_end");

esp_mqtt_client_handle_t mqtt_handle = NULL;

void mqtt_event_callback(void *event_handler_arg,
                         esp_event_base_t event_base,
                         int32_t event_id,
                         void *event_data)
{
    const char *TAG = "mqtt_event_callback";
    esp_mqtt_event_handle_t receive_data = event_data;
    switch (event_id)
    {
    case MQTT_EVENT_CONNECTED: // 连接确认
        ESP_LOGI(TAG, "Connected to broker");
        break;
    case MQTT_EVENT_DISCONNECTED: // 断开连接确认
        ESP_LOGI(TAG, "Disconnected from broker");
        break;
    case MQTT_EVENT_PUBLISHED: // 消息发布确认
        ESP_LOGI(TAG, "mqtt publish ack");
        break;
    case MQTT_EVENT_SUBSCRIBED: // 订阅确认
        ESP_LOGI(TAG, "ESP32 Subscribed ack");
        break;
    case MQTT_EVENT_UNSUBSCRIBED: // 取消订阅确认
        ESP_LOGI(TAG, "ESP32 Unsubscribed ack");
        break;
    case MQTT_EVENT_DATA: // 接收到MQTT消息数据
        ESP_LOGI(TAG, "topic length: %d", receive_data->topic_len);
        ESP_LOGI(TAG, "data length: %d", receive_data->data_len);
        ESP_LOGI(TAG, "topic: %.*s", receive_data->topic_len, receive_data->topic);
        ESP_LOGI(TAG, "data: %.*s", receive_data->data_len, receive_data->data);
        hw_iot_mqtt_subscribe_type_t subscribe_type = hw_iot_mqtt_subscribe_type(receive_data); // 获取订阅类型
        ESP_LOGI(TAG, "subscribe_type: %d", subscribe_type);
        if (hw_iot_mqtt_subscribe_ack(subscribe_type, receive_data) != ESP_OK) // 处理订阅确认
        {
            ESP_LOGE(TAG, "Failed to subscribe ack");
        }
        break;
    default:
        break;
    }
}

esp_err_t hw_iot_mqtt_subscribe_ack(hw_iot_mqtt_subscribe_type_t subscribe_type, esp_mqtt_event_handle_t receive_data)
{
    char *TAG = "hw_iot_mqtt_subscribe_ack";
    if (!receive_data)
    {
        ESP_LOGE(TAG, "Invalid event handle");
        return ESP_FAIL;
    }
    if (subscribe_type == HW_IOT_MQTT_SUBSCRIBE_TYPE_INVALID)
    {
        ESP_LOGE(TAG, "Invalid subscribe type");
        return ESP_FAIL;
    }
    switch (subscribe_type)
    {
    case HW_IOT_MQTT_MESSAGE_SUBSCRIBE: // 消息订阅
        ESP_LOGI(TAG, "Message subscribe ack");
        break;
    case HW_IOT_MQTT_COMMAND_SUBSCRIBE: // 命令订阅
        ESP_LOGI(TAG, "Command subscribe ack");
        char request_id[128] = {0};
        hw_iot_mqtt_topic_get_command_request_id(receive_data, request_id); // 从topic中提取request_id
        if (hw_iot_mqtt_command_report(request_id) != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to report command");
            return ESP_FAIL;
        }
        break;
    case HW_IOT_MQTT_OTA_VERSION_QUERY_SUBSCRIBE: // OTA 版本查询订阅
        ESP_LOGI(TAG, "Version query subscribe ack");
        if (hw_iot_mqtt_ota_version_publish() != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to report version");
            return ESP_FAIL;
        }
        break;
    case HW_IOT_MQTT_OTA_SFW_UPGRADE_SUBSCRIBE: // OTA 固件升级订阅
        ESP_LOGI(TAG, "Firmware upgrade subscribe ack");
        cJSON *ota_js = cJSON_Parse(receive_data->data); // 解析平台下发的JSON数据
        if (!ota_js)
        {
            ESP_LOGE(TAG, "cJSON_Parse failed");
            return ESP_FAIL;
        }
        cJSON *services_js = cJSON_GetObjectItem(ota_js, "services"); // 提取services数组
        if (!services_js || !cJSON_IsArray(services_js) || cJSON_GetArraySize(services_js) == 0)
        {
            ESP_LOGE(TAG, "services invalid");
            cJSON_Delete(ota_js);
            return ESP_FAIL;
        }
        cJSON *service_obj = cJSON_GetArrayItem(services_js, 0); // 提取services数组的第一个元素
        if (!service_obj || !cJSON_IsObject(service_obj))
        {
            ESP_LOGE(TAG, "service[0] invalid");
            cJSON_Delete(ota_js);
            return ESP_FAIL;
        }
        cJSON *paras_js = cJSON_GetObjectItem(service_obj, "paras"); // 提取paras对象
        if (!paras_js || !cJSON_IsObject(paras_js))
        {
            ESP_LOGE(TAG, "paras invalid");
            cJSON_Delete(ota_js);
            return ESP_FAIL;
        }
        cJSON *url_js = cJSON_GetObjectItem(paras_js, "url"); // 提取url字符串
        if (!url_js || !cJSON_IsString(url_js) || url_js->valuestring == NULL)
        {
            ESP_LOGE(TAG, "url invalid");
            cJSON_Delete(ota_js);
            return ESP_FAIL;
        }
        cJSON *access_token_js = cJSON_GetObjectItem(paras_js, "access_token"); // 提取access_token字符串
        if (!access_token_js || !cJSON_IsString(access_token_js) || access_token_js->valuestring == NULL)
        {
            ESP_LOGE(TAG, "access_token invalid");
            cJSON_Delete(ota_js);
            return ESP_FAIL;
        }
        ESP_LOGI(TAG, "OTA URL: %s", url_js->valuestring);
        ESP_LOGI(TAG, "OTA access_token: %s", access_token_js->valuestring);

        ota_upgrade_request_t req = {
            .url = url_js->valuestring,
            .access_token = access_token_js->valuestring,
        };
        if (ota_manager_submit(&req) != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to submit upgrade request");
            return ESP_FAIL;
        }

        cJSON_Delete(ota_js);
        break;
    default:
        break;
    }
    return ESP_OK;
}

void hw_iot_mqtt_init(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {0};
    mqtt_cfg.broker.address.uri = HW_IOT_URI;
    mqtt_cfg.broker.address.port = HW_IOT_PORT;
    mqtt_cfg.credentials.client_id = HW_IOT_CLIENT_ID;
    mqtt_cfg.credentials.username = HW_IOT_USERNAME;
    mqtt_cfg.credentials.authentication.password = HW_IOT_PASSWORD;
    mqtt_cfg.broker.verification.certificate = _binary_cert_pem_start; // 验证证书
    mqtt_handle = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_handle, ESP_EVENT_ANY_ID, mqtt_event_callback, NULL);
    esp_mqtt_client_start(mqtt_handle);
}