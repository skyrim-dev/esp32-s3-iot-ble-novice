#include <stdio.h>
#include <string.h>

#include <esp_err.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <freertos/semphr.h>
#include <nvs_flash.h>

#include "wifi.h"


SemaphoreHandle_t wifi_connected_semaphore = NULL;

/**
 * @brief WiFi事件回调函数
 *
 * 该函数是ESP-IDF WiFi驱动的事件回调函数，用于处理WiFi和IP相关的事件。
 * 支持的事件类型包括：WiFi启动、连接成功、连接断开、获取IP地址等。
 *
 * @param event_handler_arg 事件处理器的用户参数，当前未使用
 * @param event_base 事件基础类型，用于区分不同的事件源
 *                   - WIFI_EVENT: WiFi相关事件
 *                   - IP_EVENT: IP相关事件
 * @param event_id 事件ID，标识具体的事件类型
 *                   - WIFI_EVENT_STA_START: WiFi STA启动
 *                   - WIFI_EVENT_STA_CONNECTED: WiFi STA连接成功
 *                   - WIFI_EVENT_STA_DISCONNECTED: WiFi STA连接断开
 *                   - IP_EVENT_STA_GOT_IP: 获取到IP地址
 * @param event_data 事件数据指针，包含事件的详细信息，当前未使用
 *
 * @note
 *       1. 该函数由ESP-IDF WiFi驱动在发生WiFi或IP事件时自动调用
 *       2. 在WIFI_EVENT_STA_START事件中，会自动调用esp_wifi_connect()开始连接
 *       3. 在WIFI_EVENT_STA_DISCONNECTED事件中，会自动重新连接WiFi
 *       4. 在IP_EVENT_STA_GOT_IP事件中，会释放wifi_connected_semaphore信号量
 *       5. 信号量用于通知其他任务WiFi已连接并获取到IP地址
 *       6. 日志标签使用"wifi"
 *       7. 对于未知的事件ID，会记录警告日志
 */
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

/**
 * @brief 初始化WiFi模块并连接到WiFi网络
 *
 * 该函数用于初始化ESP32的WiFi模块，配置为Station模式，并连接到指定的WiFi网络。
 * 函数会创建信号量用于同步WiFi连接状态，并注册事件回调函数处理WiFi事件。
 *
 * @note
 *       1. 函数会从配置宏中读取WiFi连接参数：
 *          - WIFI_SSID: WiFi网络的SSID
 *          - WIFI_PWD: WiFi网络的密码
 *       2. 函数会创建二进制信号量wifi_connected_semaphore，用于同步WiFi连接状态
 *       3. 初始化步骤包括：
 *          - 初始化NVS Flash（用于存储WiFi配置）
 *          - 初始化网络接口（esp_netif）
 *          - 创建默认事件循环
 *          - 创建默认WiFi STA网络接口
 *          - 初始化WiFi驱动
 *          - 注册WiFi和IP事件回调函数
 *       4. WiFi配置包括：
 *          - 认证模式：WPA2-PSK
 *          - 启用保护管理帧（PMF）
 *          - 不强制要求PMF
 *       5. 函数会自动启动WiFi服务，开始连接到指定的WiFi网络
 *       6. WiFi连接成功并获取IP地址后，会释放wifi_connected_semaphore信号量
 *       7. 其他任务可以通过xSemaphoreTake等待WiFi连接完成
 *       8. 如果WiFi连接断开，会自动重新连接
 *       9. 该函数应该在应用程序启动时调用一次
 *       10. 如果多次调用，可能会导致资源泄漏或异常
 *       11. 日志标签使用"wifi"
 */
void wifi_init(void)
{
    wifi_connected_semaphore = xSemaphoreCreateBinary(); // 创建二进制信号量

    ESP_ERROR_CHECK(nvs_flash_init());                // 初始化NVS flash
    ESP_ERROR_CHECK(esp_netif_init());                // 初始化网络接口
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // 创建默认事件循环
    esp_netif_create_default_wifi_sta();              // 创建默认WiFi STA网络
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));                                                 // 初始化WiFi
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_callback, NULL);  // 注册WiFi事件处理函数
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_callback, NULL); // 注册IP事件处理函数
    
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