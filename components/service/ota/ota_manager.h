#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include "esp_err.h"

#define OTA_URL_MAX_LEN 256
#define OTA_TOKEN_MAX_LEN 256

typedef struct
{
    char url[OTA_URL_MAX_LEN];
    char access_token[OTA_TOKEN_MAX_LEN];
} ota_upgrade_request_t;

esp_err_t ota_manager_submit(const ota_upgrade_request_t *req); // 提交升级请求
char *get_app_version(void);                                    // 获取应用版本号

#endif