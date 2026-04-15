#ifndef __MQTT_HW_IOT_MESSAGE_REPORT_H__
#define __MQTT_HW_IOT_MESSAGE_REPORT_H__

#include <cJSON.h>

/**
 * @brief 华为云IoT设备属性上报结构体
 *
 * 对应华为云IoT平台 ServiceProperty 结构:
 * {
 *   "services": [{
 *     "service_id": "BasicData",
 *     "properties": { "luminance": 30 },
 *     "event_time": "20161219T114920Z"
 *   }]
 * }
 */
typedef struct
{
    cJSON *services_json; // 根JSON对象，包含services数组
    char *mes_js_str;     // 序列化后的JSON字符串
    int mes_js_len;       // JSON字符串长度
} HW_IOT_DM_DES;

HW_IOT_DM_DES *hw_iot_malloc_des(void);                                                         // 创建一个物模型描述结构体
void hw_iot_set_mes_des(HW_IOT_DM_DES *des, const char *service_id, const char *name, int val); // 向物模型添加服务属性
void hw_iot_set_mes_des_time(HW_IOT_DM_DES *des, const char *service_id, const char *time_str); // 向指定服务添加时间
void hw_iot_set_current_time(HW_IOT_DM_DES *des, const char *service_id);                       // 向指定服务添加当前UTC时间
void hw_iot_mes_string(HW_IOT_DM_DES *des);                                                     // 将物模型结构体序列化为JSON字符串
void hw_iot_free_des(HW_IOT_DM_DES *des);                                                       // 释放物模型描述结构体
int hw_iot_report_properties(HW_IOT_DM_DES *des);                                               // 上报设备属性到华为云IoT平台
/* OTA */
void hw_iot_set_ota_version(HW_IOT_DM_DES *des, const char *version);                           // 设置OTA版本号（固件/软件）

#endif
