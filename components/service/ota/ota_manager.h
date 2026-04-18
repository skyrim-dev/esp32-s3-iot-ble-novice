#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include "esp_err.h"

typedef struct
{
    char *url;
    char *access_token;
} ota_upgrade_request_t;

esp_err_t ota_manager_submit(const ota_upgrade_request_t *req); // 提交升级请求

#endif