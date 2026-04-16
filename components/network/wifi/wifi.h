#ifndef WIFI_H
#define WIFI_H

#include "freertos/semphr.h"

// WiFi配置 (从 Kconfig 读取)
#define WIFI_SSID      CONFIG_WIFI_SSID
#define WIFI_PWD       CONFIG_WIFI_PASSWORD
#define WIFI_TIMEOUT   CONFIG_WIFI_CONNECT_TIMEOUT_MS
#define WIFI_RECONNECT CONFIG_WIFI_RECONNECT_INTERVAL_MS

// 外部声明的信号量
extern SemaphoreHandle_t wifi_connected_semaphore;

// WiFi初始化函数声明
void wifi_init(void);

#endif // WIFI_H