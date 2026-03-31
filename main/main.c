#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_err.h"
#include <string.h>

#define WIFI_SSID "天际"
#define WIFI_PWD "sususususu"

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
            break;
        default:
            ESP_LOGI("wifi", "Unknown event ID %d", event_id);
            break;
        }
    }
}

void app_main(void)
{
    ESP_LOGI("main", "Hello world!");

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
}
