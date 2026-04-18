#include <esp_ota_ops.h>

#include "ota_manager.h"
#include "hw_iot_ota.h"

esp_err_t ota_manager_submit(const ota_upgrade_request_t *req)
{
    if (!req)
        return ESP_FAIL;
    esp_err_t err = hw_iot_ota_init(req->url, req->access_token, hw_iot_ota_callback);
    if (err != ESP_OK)
        return err;
    return hw_iot_ota_start();
}

char *get_app_version(void)
{
    static char app_version[32] = {0};
    if (app_version[0] == 0)
    {
        const esp_partition_t *running_partition = esp_ota_get_running_partition(); // 获取当前运行的分区
        esp_app_desc_t running_desc;                                                // 应用描述
        esp_ota_get_partition_description(running_partition, &running_desc);        // 获取应用描述
        snprintf(app_version, sizeof(app_version), "%s", running_desc.version);     // 提取版本号
    }
    return app_version;
}