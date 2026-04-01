#include <stdio.h>

#include <esp_err.h>
#include <esp_log.h>
#include <mqtt_client.h>

#include "wifi.h"

#define MQTT_ADDRESS "mqtt://broker-cn.emqx.io"
#define MQTT_CLIENT_ID "mqttx_esp32_20260331"
#define MQTT_USERNAME "public"
#define MQTT_PASSWORD "public"

#define ESP32_PUBLISH_TOPIC "/topic/esp32_0331" // ESP32发布主题，MQTTX订阅
#define MQTTX_PUBLISH_TOPIC "/topic/mqttx_0331" // MQTTX发布主题，ESP32订阅

static esp_mqtt_client_handle_t mqtt_handle = NULL; // MQTT客户端句柄
void mqtt_event_callback(void *event_handler_arg,
                         esp_event_base_t event_base,
                         int32_t event_id,
                         void *event_data)
{
    esp_mqtt_event_handle_t receive_data = event_data;
    switch (event_id)
    {
    case MQTT_EVENT_CONNECTED: // 连接成功
        ESP_LOGI("mqtt", "Connected to broker");
        esp_mqtt_client_subscribe_single(mqtt_handle, MQTTX_PUBLISH_TOPIC, 1);
        break;
    case MQTT_EVENT_DISCONNECTED: // 断开连接
        ESP_LOGI("mqtt", "Disconnected from broker");
        break;
    case MQTT_EVENT_PUBLISHED: // 发布消息成功
        ESP_LOGI("mqtt", "mqtt publish ack");
        break;
    case MQTT_EVENT_SUBSCRIBED: // 订阅成功成功
        ESP_LOGI("mqtt", "ESP32 Subscribed ack");
        break;
    case MQTT_EVENT_UNSUBSCRIBED: // 收到解订阅消息
        break;
    case MQTT_EVENT_DATA: // ESP32收到消息
        /* 注意，需要日志打印出指定长度的字符串，否则容易接收到乱码 */
        ESP_LOGI("mqtt", "topic length: %d", receive_data->topic_len);
        ESP_LOGI("mqtt", "topic: %.*s", receive_data->topic_len, receive_data->topic);
        ESP_LOGI("mqtt", "data length: %d", receive_data->data_len);
        ESP_LOGI("mqtt", "data: %.*s", receive_data->data_len, receive_data->data);
        break;
    default:
        break;
    }
}

void mqtt_init(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {0};                      // 初始化配置结构体
    mqtt_cfg.broker.address.uri = MQTT_ADDRESS;                   // 设置MQTT地址
    mqtt_cfg.broker.address.port = 1883;                          // 设置MQTT端口
    mqtt_cfg.credentials.client_id = MQTT_CLIENT_ID;              // 设置客户端ID
    mqtt_cfg.credentials.username = MQTT_USERNAME;                // 设置用户名
    mqtt_cfg.credentials.authentication.password = MQTT_PASSWORD; // 设置密码
    mqtt_handle = esp_mqtt_client_init(&mqtt_cfg);                // 初始化MQTT客户端句柄
    /* 注册事件回调函数 */
    esp_mqtt_client_register_event(mqtt_handle, ESP_EVENT_ANY_ID, mqtt_event_callback, NULL);
    esp_mqtt_client_start(mqtt_handle); // 启动MQTT客户端
}

void app_main(void)
{
    ESP_LOGI("main", "Hello world!");

    // 初始化WiFi
    wifi_init();

    /* 等待WiFi连接成功，获取到IP地址后，初始化MQTT客户端 */
    xSemaphoreTake(wifi_connected_semaphore, portMAX_DELAY); // 等待WiFi连接成功
    ESP_LOGI("main", "WiFi connected");
    mqtt_init();
    /* ESP32发布消息 */
    uint8_t count = 0;
    while (1)
    {
        char count_str[32];
        snprintf(count_str, sizeof(count_str), "{\"count\": %d}", count);
        esp_mqtt_client_publish(mqtt_handle, ESP32_PUBLISH_TOPIC, count_str, strlen(count_str), 1, 0);
        count++;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}