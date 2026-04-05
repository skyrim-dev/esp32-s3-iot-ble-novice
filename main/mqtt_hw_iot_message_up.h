#ifndef __MQTT_HW_IOT_MESSAGE_UP_H__
#define __MQTT_HW_IOT_MESSAGE_UP_H__

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
    cJSON *services_json;    // 根JSON对象，包含services数组
    char *mes_js_str;        // 序列化后的JSON字符串
    int mes_js_len;          // JSON字符串长度
} HW_IOT_DM_DES;

/**
 * @brief 创建一个物模型描述结构体
 * @return 新创建的描述结构体指针，失败返回NULL
 */
HW_IOT_DM_DES *hw_iot_malloc_des(void);

/**
 * @brief 向物模型添加服务属性
 * @param des 物模型描述结构体
 * @param service_id 服务ID (如 "BasicData", "Temperature")
 * @param name 属性名 (如 "luminance", "value")
 * @param val 属性值 (整数)
 */
void hw_iot_set_mes_des(HW_IOT_DM_DES *des, const char *service_id, const char *name, int val);

/**
 * @brief 向指定服务添加 event_time 字段
 * @param time_str UTC时间字符串，格式: yyyyMMddTHHmmssZ
 */
void hw_iot_set_mes_des_time(HW_IOT_DM_DES *des, const char *service_id, const char *time_str);

/**
 * @brief 向指定服务添加当前UTC时间
 */
void hw_iot_set_current_time(HW_IOT_DM_DES *des, const char *service_id);

/**
 * @brief 将物模型结构体序列化为JSON字符串
 * @param des 物模型描述结构体
 * @note 结果存储在 des->mes_js_str 中
 */
void hw_iot_mes_string(HW_IOT_DM_DES *des);

/**
 * @brief 释放物模型描述结构体
 * @param des 物模型描述结构体
 */
void hw_iot_free_des(HW_IOT_DM_DES *des);

/**
 * @brief 上报设备属性到华为云IoT平台
 * @param des 物模型描述结构体 (需先调用 hw_iot_mes_string)
 * @return 0成功, -1失败
 */
int hw_iot_report_properties(HW_IOT_DM_DES *des);

#endif
