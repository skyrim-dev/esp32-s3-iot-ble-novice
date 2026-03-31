#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_netif.h" //  网络接口
#include "esp_smartconfig.h"
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
            break;
        }
    }
    else if (event_base == SC_EVENT)
    {
        switch (event_id)
        {
        case SC_EVENT_SCAN_DONE: //  扫描完成
            ESP_LOGI("wifi", "SmartConfig scan done");
            break;
        case SC_EVENT_GOT_SSID_PSWD: //  获取到SSID和密码
            ESP_LOGI("wifi", "SmartConfig got ssid and password");
            smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
            wifi_config_t wifi_config = {0};
            /* 复制SSID和密码到wifi_config */
            snprintf((char *)wifi_config.sta.ssid, sizeof(wifi_config.sta.ssid), "%s", (char *)evt->ssid);  
            snprintf((char *)wifi_config.sta.password, sizeof(wifi_config.sta.password), "%s", (char *)evt->password);
            wifi_config.sta.bssid_set = evt->bssid_set; //  是否设置BSSID
            memset(wifi_config.sta.bssid, 0, sizeof(wifi_config.sta.bssid));     //  清空BSSID
            if (wifi_config.sta.bssid_set)  //  如果设置BSSID
            {
                memcpy(wifi_config.sta.bssid, evt->bssid, 6); //  复制BSSID到wifi_config.sta.bssid
            }
            esp_wifi_disconnect(); //  断开WiFi连接
            esp_wifi_set_config(WIFI_IF_STA, &wifi_config); //  设置WiFi配置
            esp_wifi_connect(); //  连接到WiFi网络
            break;
        case SC_EVENT_SEND_ACK_DONE: //  发送ACK完成
            ESP_LOGI("wifi", "SmartConfig send ack done");
            esp_smartconfig_stop(); //  停止智能配置
            break;
        default:
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
    esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, wifi_event_callback, NULL);    // 注册智能配置事件处理函数
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));                                    // 设置WiFi模式为STA
    ESP_ERROR_CHECK(esp_wifi_start());                                                    // 启动WiFi服务
    
    esp_smartconfig_set_type(SC_TYPE_ESPTOUCH); // 设置智能配置类型为ESP-Touch
    smartconfig_start_config_t sc_cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    esp_smartconfig_start(&sc_cfg); // 启动智能配置
}
