#include <esp_log.h>
#include <esp_https_ota.h>
#include <esp_http_client.h>
#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_crt_bundle.h>
#include <esp_err.h>

#include "hw_iot_ota.h"

static char hw_iot_url[256];
static char hw_iot_access_token[256];

static hw_iot_ota_finish_callback_t hw_iot_ota_finish_cb = NULL;

static bool is_current_ota_task_running = false;

void hw_iot_ota_callback(int code)
{
}

static esp_err_t hw_iot_http_client_init_cb(esp_http_client_handle_t client)
{
    char auth_header[192] = {0};
    if (snprintf(auth_header, sizeof(auth_header), "Bearer %s", hw_iot_access_token) >= sizeof(auth_header))
    {
        return ESP_FAIL;
    }
    esp_http_client_set_header(client, "Authorization", auth_header);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    return ESP_OK;
}

void hw_iot_ota_task(void *param)
{
    const char *TAG = "hw_iot_ota_task";
    ESP_LOGI(TAG, "OTA task started");
    // http客户端初始化
    esp_http_client_config_t config = {
        .url = hw_iot_url,                          // OTA URL
        .timeout_ms = 10000 * 10,                   // 10秒超时
        .crt_bundle_attach = esp_crt_bundle_attach, // 附加证书包
        .keep_alive_enable = true,                  // 启用长连接
        .skip_cert_common_name_check = true,        // 跳过证书通用名称检查，用于测试环境
    };

    // ota配置初始化
    esp_https_ota_config_t ota_config = {
        .http_config = &config,                            // HTTP客户端配置
        .http_client_init_cb = hw_iot_http_client_init_cb, // HTTP客户端初始化函数
    };
    // 启动OTA任务（核心）
    // 分配HTTPS OTA固件升级上下文，建立HTTPS连接,
    // 从HTTP流中读取图像数据并将其写入OTA分区和完成HTTPS OTA固件升级操作
    esp_err_t ota_finish_err = esp_https_ota(&ota_config); // 此函数本身带有阻塞
    if (ota_finish_err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start OTA task, err=%d", ota_finish_err);
        if (hw_iot_ota_finish_cb)
        {
            hw_iot_ota_finish_cb(ESP_FAIL);
        }
        is_current_ota_task_running = false;
        vTaskDelete(NULL); // 需要先删除任务，不能直接返回
        return;
    }
    else
    {
        if (hw_iot_ota_finish_cb)
        {
            ESP_LOGI(TAG, "OTA task finished");
            if (hw_iot_ota_finish_cb)
            {
                hw_iot_ota_finish_cb(ESP_OK);
            }
            vTaskDelay(pdMS_TO_TICKS(1000)); // 等待1秒，确保OTA任务完成
            esp_restart();
        }
    }
    is_current_ota_task_running = false;
    vTaskDelete(NULL);
}

esp_err_t hw_iot_ota_start(void)
{
    const char *TAG = "hw_iot_ota_start";
    if (is_current_ota_task_running)
    {
        ESP_LOGE(TAG, "OTA task is running");
        return ESP_FAIL;
    }
    is_current_ota_task_running = true;
    xTaskCreatePinnedToCore(hw_iot_ota_task, "hw_iot_ota_task", 8192, NULL, 4, NULL, 1); // 创建OTA任务，优先级为4，分配在第一个内核
    return ESP_OK;
}

esp_err_t hw_iot_ota_init(const char *url, const char *access_token, hw_iot_ota_finish_callback_t cb)
{
    char *TAG = "hw_iot_ota_init";
    if (url == NULL || url[0] == '\0')
    {
        ESP_LOGI(TAG, "OTA URL is empty");
        return ESP_FAIL;
    }
    if (access_token == NULL || access_token[0] == '\0')
    {
        ESP_LOGI(TAG, "OTA access_token is empty");
        return ESP_FAIL;
    }
    if (snprintf(hw_iot_url, sizeof(hw_iot_url), "%s", url) >= sizeof(hw_iot_url)) // 初始化URL
    {
        ESP_LOGI(TAG, "OTA URL is too long");
        return ESP_FAIL;
    }
    if (snprintf(hw_iot_access_token, sizeof(hw_iot_access_token), "%s", access_token) >= sizeof(hw_iot_access_token)) // 初始化access_token
    {
        ESP_LOGI(TAG, "OTA access_token is too long");
        return ESP_FAIL;
    }
    hw_iot_ota_finish_cb = cb; // 初始化回调函数
    return ESP_OK;
}
