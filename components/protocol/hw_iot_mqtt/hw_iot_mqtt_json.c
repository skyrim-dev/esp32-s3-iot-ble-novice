#include <cJSON.h>
#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <time.h>

#include "hw_iot_mqtt_json.h"

// 生成属性报告 JSON 字符串
char *hw_iot_mqtt_properties_report_json(hw_iot_mqtt_properties_report_json_t *json)
{
    const char *TAG = "hw_iot_mqtt_properties_report_json";
    if (!json)
    {
        ESP_LOGE(TAG, "Input json pointer is NULL");
        return NULL;
    }

    /* 创建根 json 对象 */
    cJSON *root_js = cJSON_CreateObject(); // 创建根 json 对象
    if (!root_js)
    {
        ESP_LOGE(TAG, "cJSON_CreateObject failed");
        return NULL;
    }

    /* 创建 services 数组对象 */
    cJSON *services_js = cJSON_CreateArray(); // 创建 services 数组对象
    if (!services_js)
    {
        ESP_LOGE(TAG, "cJSON_CreateArray failed");
        cJSON_Delete(root_js);
        return NULL;
    }
    cJSON_AddItemToObject(root_js, "services", services_js); // 添加 services 对象到根 json 对象

    /* 遍历服务结构体数组 */
    uint8_t i = 0;
    while (i < HW_IOT_MQTT_SERVICES_NUM)
    {
        if (json->json[i].service_id == NULL) // 如果 service_id 为空，说明没有更多服务了，退出循环
        {
            ESP_LOGW(TAG, "Configuration allows up to 10 services, but only %d services found", i);
            break;
        }
        /* 重要 */
        cJSON *service_obj_js = cJSON_CreateObject(); // 创建 services 数组对象里的单个服务对象
        if (!service_obj_js)
        {
            ESP_LOGE(TAG, "cJSON_CreateObject failed");
            cJSON_Delete(root_js);
            return NULL;
        }
        cJSON_AddItemToArray(services_js, service_obj_js); // 把对象加入数组

        cJSON_AddStringToObject(service_obj_js, "service_id", json->json[i].service_id); // 添加 service_id 到服务对象

        cJSON *properties_js = cJSON_CreateObject(); // 创建 properties 对象
        if (!properties_js)
        {
            ESP_LOGE(TAG, "cJSON_CreateObject failed");
            cJSON_Delete(root_js);
            return NULL;
        }
        cJSON_AddItemToObject(service_obj_js, "properties", properties_js); // 添加 properties 对象到服务对象

        int j = 0;
        while (j < HW_IOT_MQTT_PROPERTIES_NUM)
        {
            if (json->json[i].properties_id[j] == NULL) // 如果 property_id 为空，说明没有更多属性了，退出循环
            {
                ESP_LOGW(TAG, "Configuration allows up to 10 properties per service, but only %d properties found for service_id: %s", j, json->json[i].service_id);
                break;
            }
            cJSON_AddNumberToObject(properties_js, json->json[i].properties_id[j], json->json[i].properties_value[j]); // 添加 property_id 到 properties 对象
            j++;
        }

        /* 获取当前UTC时间并格式化为华为云要求的格式 */
        time_t now = time(NULL);                                         // 获取当前时间戳
        struct tm *tm_info = gmtime(&now);                               // 获取UTC时间结构体
        char time_buf[32];                                               // 用于存储格式化后的UTC时间字符串
        strftime(time_buf, sizeof(time_buf), "%Y%m%dT%H%M%SZ", tm_info); // 格式化UTC时间字符串
        cJSON_AddStringToObject(service_obj_js, "event_time", time_buf); // 添加 time 到服务对象

        i++;
    }

    /* 将cJSON对象序列化为无格式JSON字符串 */
    char *js_str = cJSON_UnformattedFree(root_js); // 生成 JSON 字符串，不包含格式化字符

    return js_str;
}

// 生成命令响应 JSON 字符串
char *hw_iot_mqtt_command_response_json(hw_iot_mqtt_command_response_json_t *json)
{
    char *TAG = "hw_iot_mqtt_command_response_json";
    if (!json)
    {
        ESP_LOGE(TAG, "Input json pointer is NULL");
        return NULL;
    }

    /* 创建根 json 对象 */
    cJSON *root_js = cJSON_CreateObject(); // 创建根 json 对象
    if (!root_js)
    {
        ESP_LOGE(TAG, "cJSON_CreateObject failed");
        return NULL;
    }
    cJSON_AddNumberToObject(root_js, "result_code", json->result_code);     // 添加 result_code 到根 json 对象
    cJSON_AddStringToObject(root_js, "response_name", json->response_name); // 添加 response_name 到根 json 对象

    cJSON *paras_js = cJSON_CreateObject(); // 创建 paras 对象
    if (!paras_js)
    {
        ESP_LOGE(TAG, "cJSON_CreateObject failed");
        cJSON_Delete(root_js);
        return NULL;
    }
    cJSON_AddItemToObject(root_js, "paras", paras_js);         // 添加 paras 对象到根 json 对象
    cJSON_AddStringToObject(paras_js, "result", json->result); // 添加 result 到 paras 对象

    /* 将cJSON对象序列化为无格式JSON字符串 */
    char *js_str = cJSON_UnformattedFree(root_js); // 生成 JSON 字符串，不包含格式化字符

    return js_str;
}

// 生成固件版本报告 JSON 字符串
char *hw_iot_mqtt_ota_version_report_json(hw_iot_mqtt_ota_response_version_json_t *json)
{
    char *TAG = "hw_iot_mqtt_ota_version_report_json";
    if (!json)
    {
        ESP_LOGE(TAG, "Input json pointer is NULL");
        return NULL;
    }

    /* 创建根 json 对象 */
    cJSON *root_js = cJSON_CreateObject(); // 创建根 json 对象
    if (!root_js)
    {
        ESP_LOGE(TAG, "cJSON_CreateObject failed");
        return NULL;
    }

    /* 创建 services 数组对象 */
    cJSON *services_js = cJSON_CreateArray(); // 创建 services 数组对象
    if (!services_js)
    {
        ESP_LOGE(TAG, "cJSON_CreateArray failed");
        cJSON_Delete(root_js);
        return NULL;
    }
    cJSON_AddItemToObject(root_js, "services", services_js); // 添加 services 对象到根 json 对象

    cJSON *service_obj_js = cJSON_CreateObject(); // 创建 services 数组对象里的单个服务对象
    if (!service_obj_js)
    {
        ESP_LOGE(TAG, "cJSON_CreateObject failed");
        cJSON_Delete(root_js);
        return NULL;
    }
    cJSON_AddItemToArray(services_js, service_obj_js); // 把对象加入数组

    cJSON_AddStringToObject(service_obj_js, "service_id", "$ota");           // 添加 service_id 到服务对象，这里固定为$ota
    cJSON_AddStringToObject(service_obj_js, "event_type", "version_report"); // 添加 event_type 到服务对象，这里固定为version_report

    /* 获取当前UTC时间并格式化为华为云要求的格式 */
    time_t now = time(NULL);                                         // 获取当前时间戳
    struct tm *tm_info = gmtime(&now);                               // 获取UTC时间结构体
    char time_buf[32];                                               // 用于存储格式化后的UTC时间字符串
    strftime(time_buf, sizeof(time_buf), "%Y%m%dT%H%M%SZ", tm_info); // 格式化UTC时间字符串
    cJSON_AddStringToObject(service_obj_js, "event_time", time_buf); // 添加 time 到服务对象

    cJSON *paras_js = cJSON_CreateObject(); // 创建 paras 对象
    if (!paras_js)
    {
        ESP_LOGE(TAG, "cJSON_CreateObject failed");
        cJSON_Delete(root_js);
        return NULL;
    }
    cJSON_AddItemToObject(service_obj_js, "paras", paras_js);          // 添加 paras 对象到服务对象
    cJSON_AddStringToObject(paras_js, "sw_version", json->sw_version); // 添加 sw_version 到 paras 对象
    cJSON_AddStringToObject(paras_js, "fw_version", json->fw_version); // 添加 fw_version 到 paras 对象

    /* 将cJSON对象序列化为无格式JSON字符串 */
    char *js_str = cJSON_UnformattedFree(root_js); // 生成 JSON 字符串，不包含格式化字符

    return js_str;
}

// 生成升级状态报告 JSON 字符串
char *hw_iot_mqtt_upgrade_status_report_json(hw_iot_mqtt_ota_upgrade_status_json_t *json)
{
    char *TAG = "hw_iot_mqtt_upgrade_status_report_json";
    if (!json)
    {
        ESP_LOGE(TAG, "Input json pointer is NULL");
        return NULL;
    }

    /* 创建根 json 对象 */
    cJSON *root_js = cJSON_CreateObject(); // 创建根 json 对象
    if (!root_js)
    {
        ESP_LOGE(TAG, "cJSON_CreateObject failed");
        return NULL;
    }

    /* 创建 services 数组对象 */
    cJSON *services_js = cJSON_CreateArray(); // 创建 services 数组对象
    if (!services_js)
    {
        ESP_LOGE(TAG, "cJSON_CreateArray failed");
        cJSON_Delete(root_js);
        return NULL;
    }
    cJSON_AddItemToObject(root_js, "services", services_js); // 添加 services 对象到根 json 对象

    cJSON *service_obj_js = cJSON_CreateObject(); // 创建 services 数组对象里的单个服务对象
    if (!service_obj_js)
    {
        ESP_LOGE(TAG, "cJSON_CreateObject failed");
        cJSON_Delete(root_js);
        return NULL;
    }
    cJSON_AddItemToArray(services_js, service_obj_js); // 把对象加入数组

    cJSON_AddStringToObject(service_obj_js, "service_id", "$ota");                    // 添加 service_id 到服务对象，这里固定为$ota
    cJSON_AddStringToObject(service_obj_js, "event_type", "upgrade_progress_report"); // 添加 event_type 到服务对象，这里固定为upgrade_progress_report

    /* 获取当前UTC时间并格式化为华为云要求的格式 */
    time_t now = time(NULL);                                         // 获取当前时间戳
    struct tm *tm_info = gmtime(&now);                               // 获取UTC时间结构体
    char time_buf[32];                                               // 用于存储格式化后的UTC时间字符串
    strftime(time_buf, sizeof(time_buf), "%Y%m%dT%H%M%SZ", tm_info); // 格式化UTC时间字符串
    cJSON_AddStringToObject(service_obj_js, "event_time", time_buf); // 添加 time 到服务对象

    cJSON *paras_js = cJSON_CreateObject(); // 创建 paras 对象
    if (!paras_js)
    {
        ESP_LOGE(TAG, "cJSON_CreateObject failed");
        cJSON_Delete(root_js);
        return NULL;
    }
    cJSON_AddItemToObject(service_obj_js, "paras", paras_js);            // 添加 paras 对象到服务对象
    cJSON_AddNumberToObject(paras_js, "result_code", json->result_code); // 添加 result_code 到 paras 对象
    cJSON_AddNumberToObject(paras_js, "progress", json->progress);       // 添加 progress 到 paras 对象
    cJSON_AddStringToObject(paras_js, "version", json->version);         // 添加 version 到 paras 对象
    cJSON_AddStringToObject(paras_js, "description", json->description); // 添加 description 到 paras 对象

    /* 将cJSON对象序列化为无格式JSON字符串 */
    char *js_str = cJSON_UnformattedFree(root_js); // 生成 JSON 字符串，不包含格式化字符

    return js_str;
}

// 将cJSON对象序列化为无格式 JSON 字符串
char *cJSON_UnformattedFree(cJSON *root_js)
{
    char *TAG = "cJSON_UnformattedFree";
    /* 将cJSON对象序列化为无格式JSON字符串 */
    char *js_str = cJSON_PrintUnformatted(root_js); // 生成 JSON 字符串，不包含格式化字符
    if (!js_str)
    {
        ESP_LOGE(TAG, "cJSON_PrintUnformatted failed");
        cJSON_Delete(root_js);
        return NULL;
    }
    ESP_LOGI(TAG, "Generated JSON for service(s): %s", js_str);

    cJSON_Delete(root_js); // 释放根 json 对象

    return js_str;
}