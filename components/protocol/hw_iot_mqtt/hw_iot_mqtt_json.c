#include <cJSON.h>
#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <time.h>

#include "hw_iot_mqtt_json.h"

/**
 * @brief 生成华为云IoT平台属性上报的JSON字符串
 *
 * 该函数根据输入的服务和属性数据结构，生成符合华为云IoT平台要求的JSON格式字符串。
 * JSON结构为：{"services": [{"service_id": "xxx", "properties": {"key": value}, "event_time": "UTC时间"}]}
 *
 * @param json 指向hw_iot_mqtt_properties_report_json_t结构体的指针，包含需要上报的服务和属性数据
 *               - json[].service_id: 服务ID，为NULL时表示服务结束
 *               - json[].properties_id[]: 属性ID数组，为NULL时表示属性结束
 *               - json[].properties_value[]: 属性值数组，与properties_id一一对应
 *
 * @return char* 成功返回动态分配的JSON字符串指针，失败返回NULL
 *               注意：调用者需要在使用完毕后调用free()释放返回的字符串内存
 *
 * @note
 *       1. 函数内部会自动释放cJSON对象树，调用者只需负责释放返回的JSON字符串
 *       2. 时间格式使用华为云要求的UTC格式：YYYYMMDDThhmmssZ
 *       3. 生成的JSON字符串为无格式化版本，减少传输数据量
 *       4. 如果cJSON创建或序列化失败，函数会返回NULL并记录错误日志
 *       5. 支持动态服务数量，通过service_id为NULL判断结束
 *       6. 支持动态属性数量，通过properties_id为NULL判断结束
 */
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

/**
 * @brief 生成华为云IoT平台命令响应的JSON字符串
 *
 * 该函数根据输入的命令响应数据结构，生成符合华为云IoT平台要求的命令响应JSON格式字符串。
 * JSON结构为：{"result_code": xxx, "response_name": "xxx", "paras": {"result": "xxx"}}
 *
 * @param json 指向hw_iot_mqtt_command_response_json_t结构体的指针，包含命令响应数据
 *               - result_code: 响应结果码，0表示成功，非0表示失败
 *               - response_name: 响应名称字符串
 *               - result: 响应结果字符串
 *
 * @return char* 成功返回动态分配的JSON字符串指针，失败返回NULL
 *               注意：调用者需要在使用完毕后调用free()释放返回的字符串内存
 *
 * @note
 *       1. 函数内部会自动释放cJSON对象树，调用者只需负责释放返回的JSON字符串
 *       2. 生成的JSON字符串为无格式化版本，减少传输数据量
 *       3. 如果cJSON创建或序列化失败，函数会返回NULL并记录错误日志
 *       4. result_code通常使用0表示成功，其他值表示具体错误码
 */
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

char *hw_iot_mqtt_ota_version_report_json(hw_iot_mqtt_firmware_version_json_t *json)
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
    cJSON_AddItemToObject(service_obj_js, "paras", paras_js); // 添加 paras 对象到服务对象
    cJSON_AddStringToObject(paras_js, "sw_version", json->sw_version); // 添加 sw_version 到 paras 对象
    cJSON_AddStringToObject(paras_js, "fw_version", json->fw_version); // 添加 fw_version 到 paras 对象

    /* 将cJSON对象序列化为无格式JSON字符串 */
    char *js_str = cJSON_UnformattedFree(root_js); // 生成 JSON 字符串，不包含格式化字符

    return js_str;
}

/**
 * @brief 将cJSON对象序列化为无格式JSON字符串并释放对象
 *
 * 该函数将cJSON对象树序列化为紧凑的JSON字符串（不包含空格和换行符），
 * 然后自动释放cJSON对象树的所有内存。
 *
 * @param root_js 指向要序列化的cJSON根对象的指针
 *
 * @return char* 成功返回动态分配的JSON字符串指针，失败返回NULL
 *               注意：调用者需要在使用完毕后调用free()释放返回的字符串内存
 *
 * @note
 *       1. 函数会自动释放cJSON对象树，调用者无需手动释放
 *       2. 生成的JSON字符串为无格式化版本，减少传输数据量
 *       3. 序列化成功后会记录生成的JSON字符串到日志
 *       4. 如果序列化失败，函数会释放cJSON对象并返回NULL
 *       5. 返回的字符串由cJSON库分配，必须由调用者释放
 */
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