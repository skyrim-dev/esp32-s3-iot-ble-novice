#include <cJSON.h>
#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <time.h>

#include "hw_iot_mqtt_json.h"

char *hw_iot_mqtt_properties_report_json(hw_iot_mqtt_properties_report_json_t *json)
{
    /* 创建根 json 对象 */
    cJSON *root_js = cJSON_CreateObject(); // 创建根 json 对象
    if (!root_js)
    {
        ESP_LOGE("hw_iot_mqtt_properties_report_json", "cJSON_CreateObject failed");
        return NULL;
    }

    /* 创建 services 数组对象 */
    cJSON *services_js = cJSON_CreateArray(); // 创建 services 数组对象
    if (!services_js)
    {
        ESP_LOGE("hw_iot_mqtt_properties_report_json", "cJSON_CreateArray failed");
        cJSON_Delete(root_js);
        return NULL;
    }
    cJSON_AddItemToObject(root_js, "services", services_js); // 添加 services 对象到根 json 对象

    /* 遍历服务结构体数组 */
    uint8_t i = 0;
    while (i < HW_IOT_MQTT_SERVICES_NUM)
    {
        /* 重要 */
        cJSON *service_obj_js = cJSON_CreateObject();      // 创建 services 数组对象里的单个服务对象
        cJSON_AddItemToArray(services_js, service_obj_js); // 把对象加入数组

        cJSON_AddStringToObject(service_obj_js, "service_id", json->json[i].service_id); // 添加 service_id 到服务对象

        cJSON *properties_js = cJSON_CreateObject();                        // 创建 properties 对象
        cJSON_AddItemToObject(service_obj_js, "properties", properties_js); // 添加 properties 对象到服务对象

        int j = 0;
        while (j < HW_IOT_MQTT_PROPERTIES_NUM)
        {
            cJSON_AddNumberToObject(properties_js, json->json[i].properties_id[j], json->json[i].properties_value[j]); // 添加 property_id 到 properties 对象
            j++;
        }

        /* 获取当前UTC时间并格式化为华为云要求的格式 */
        time_t now = time(NULL);                                         // 获取当前时间戳
        struct tm *tm_info = gmtime(&now);                               // 获取UTC时间结构体
        char time_buf[32];                                               // 用于存储格式化后的UTC时间字符串
        strftime(time_buf, sizeof(time_buf), "%Y%m%dT%H%M%SZ", tm_info); // 格式化UTC时间字符串
        cJSON_AddStringToObject(service_obj_js, "event_time", time_buf);       // 添加 time 到服务对象

        i++;
    }

    /* 将cJSON对象序列化为无格式JSON字符串 */
    char *js_str = cJSON_PrintUnformatted(root_js); // 生成 JSON 字符串，不包含格式化字符
    if (!js_str)
    {
        ESP_LOGE("hw_iot_mqtt_properties_report_json", "cJSON_PrintUnformatted failed");
        cJSON_Delete(root_js);
        return NULL;
    }
    ESP_LOGI("hw_iot_mqtt_properties_report_json", "js:\r\n:%s", js_str);

    cJSON_Delete(root_js);  // 释放根 json 对象

    return js_str;
}

