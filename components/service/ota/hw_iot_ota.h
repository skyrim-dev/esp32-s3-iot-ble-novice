#ifndef HW_IOT_OTA_H
#define HW_IOT_OTA_H

#include "esp_err.h"

typedef void (*hw_iot_ota_finish_callback_t)(int code); // OTA完成回调函数

esp_err_t hw_iot_ota_init(const char *url, hw_iot_ota_finish_callback_t cb); // 初始化OTA：设置OTA服务器URL
esp_err_t hw_iot_ota_start(void);                                            // 启动OTA

#endif