#ifndef WIFI_H
#define WIFI_H

#include "freertos/semphr.h"

// WiFi配置
#define WIFI_SSID "天际"
#define WIFI_PWD "sususususu"

// 外部声明的信号量
extern SemaphoreHandle_t wifi_connected_semaphore;

// WiFi初始化函数声明
void wifi_init(void);

#endif // WIFI_H