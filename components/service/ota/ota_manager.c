#include "ota_manager.h"
#include "hw_iot_ota.h"

esp_err_t ota_manager_submit(const ota_upgrade_request_t *req)
{
    if (!req) return ESP_FAIL;
    esp_err_t err = hw_iot_ota_init(req->url, req->access_token, hw_iot_ota_callback);
    if (err != ESP_OK) return err;
    return hw_iot_ota_start();
}

