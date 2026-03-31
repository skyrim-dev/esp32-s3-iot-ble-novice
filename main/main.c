#include <stdio.h>
#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_err.h"
#include "freertos/semphr.h"
#include <string.h>

#define WIFI_SSID "天际"
#define WIFI_PWD "sususususu"

#define MQTT_ADDRESS "mqtt://broker-cn.emqx.io"
#define MQTT_CLIENT_ID "mqttx_esp32_20260331"
#define MQTT_USERNAME "public"
#define MQTT_PASSWORD "public"

#define ESP32_PUBLISH_TOPIC "/topic/esp32_0331" // ESP32发布主题，MQTTX订阅
#define MQTTX_PUBLISH_TOPIC "/topic/mqttx_0331" // MQTTX发布主题，ESP32订阅

static SemaphoreHandle_t wifi_connected_semaphore = NULL;

void wifi_event_callback(void *event_handler_arg,
                         esp_event_base_t event_base,
                         int32_t event_id,
                         void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_STA_START: //  启动WiFi STA
            ESP_LOGI("wifi", "WiFi STA start");
            esp_wifi_connect(); // 连接到WiFi网络
            break;
        case WIFI_EVENT_STA_CONNECTED: //  连接成功
            ESP_LOGI("wifi", "WiFi STA connected");
            break;
        case WIFI_EVENT_STA_DISCONNECTED: //  连接断开
            ESP_LOGI("wifi", "WiFi STA disconnected");
            esp_wifi_connect(); // 重新连接WiFi网络
            break;
        default:
            ESP_LOGW("wifi", "Unknown event ID %d", event_id);
            break;
        }
    }
    else if (event_base == IP_EVENT)
    {
        switch (event_id)
        {
        case IP_EVENT_STA_GOT_IP: //  获取到IP地址
            ESP_LOGI("wifi", "WiFi STA got IP");
            xSemaphoreGive(wifi_connected_semaphore); // 释放信号量
            break;
        default:
            ESP_LOGI("wifi", "Unknown event ID %d", event_id);
            break;
        }
    }
}

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

    wifi_connected_semaphore = xSemaphoreCreateBinary(); // 创建二进制信号量

    ESP_ERROR_CHECK(nvs_flash_init());                // 初始化NVS flash
    ESP_ERROR_CHECK(esp_netif_init());                // 初始化网络接口
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // 创建默认事件循环
    esp_netif_create_default_wifi_sta();              // 创建默认WiFi STA网络
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));                                                 // 初始化WiFi
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_callback, NULL);  // 注册WiFi事件处理函数
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_callback, NULL); // 注册IP事件处理函数}
    /* 配置WiFi */
    wifi_config_t wifi_config = {
        .sta.threshold.authmode = WIFI_AUTH_WPA2_PSK, // 设置WiFi认证模式为WPA2-PSK
        .sta.pmf_cfg.capable = true,                  // 启用保护管理帧
        .sta.pmf_cfg.required = false,                // 不要求只和有保护管理帧的设备通讯
    };
    memset(wifi_config.sta.ssid, 0, sizeof(wifi_config.sta.ssid));         // 清空SSID
    memset(wifi_config.sta.password, 0, sizeof(wifi_config.sta.password)); // 清空密码
    memcpy(wifi_config.sta.ssid, WIFI_SSID, strlen(WIFI_SSID));            // 设置SSID
    memcpy(wifi_config.sta.password, WIFI_PWD, strlen(WIFI_PWD));          // 设置密码
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));                     // 设置WiFi模式为STA
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));       // 设置WiFi配置
    ESP_ERROR_CHECK(esp_wifi_start());                                     // 启动WiFi服务
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