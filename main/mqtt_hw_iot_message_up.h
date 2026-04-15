#ifndef __MQTT_HW_IOT_MESSAGE_UP_H__
#define __MQTT_HW_IOT_MESSAGE_UP_H__

#include <cJSON.h>

typedef struct
{
    cJSON *services_json;    // services数组的JSON对象
    char *mes_js_str;        // 生成的JSON字符串
    int mes_js_len;          // JSON字符串长度
} HW_IOT_MES_DES;

// 生成一个物模型
HW_IOT_MES_DES *hw_iot_malloc_des(void);
// 往物模型描述结构体添加属性 (service_id: 服务ID, name: 属性名, val: 属性值)
void hw_iot_set_mes_des(HW_IOT_MES_DES *des, const char *service_id, const char *name, int val);
// 生成cJSON字符串，保存到mes_js_str中
void hw_iot_mes_string(HW_IOT_MES_DES *des);
// 释放物模型描述结构体
void hw_iot_free_des(HW_IOT_MES_DES *des);

#endif