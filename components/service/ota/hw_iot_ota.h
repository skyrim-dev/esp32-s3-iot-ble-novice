#ifndef HW_IOT_OTA_H
#define HW_IOT_OTA_H

#include "esp_err.h"

/**
 * @brief 定义了一个名为 hw_iot_ota_finish_callback_t 的类型，它是一个函数指针
 * @param code OTA任务完成状态码（该函数接收一个 int code 参数）
 * @return void
 * @note 凡是长这样签名的函数：void xxx(int code)，都可以被当作 hw_iot_ota_finish_callback_t 使用。
 */
typedef void (*hw_iot_ota_finish_callback_t)(int code); 

void hw_iot_ota_callback(int code);                                                                    // OTA完成回调函数
esp_err_t hw_iot_ota_init(const char *url, const char *access_token, hw_iot_ota_finish_callback_t cb); // 初始化OTA：设置OTA服务器URL
esp_err_t hw_iot_ota_start(void);                                                                      // 启动OTA

#endif