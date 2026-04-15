#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <esp_log.h>
#include <mqtt_client.h>

#include "mqtt_hw_iot_message_report.h"
#include "hw_iot_mqtt.h"
#include "hw_iot_mqtt_topic.h"

static const char *TAG = "hw_iot_msg";

/**
 * @brief 创建物模型描述结构体
 *
 * 分配内存并初始化 JSON 根对象: { "services": [] }
 * 调用者负责在使用完毕后调用 hw_iot_free_des() 释放内存
 *
 * @return 成功返回 HW_IOT_DM_DES 指针，失败返回 NULL
 */
HW_IOT_DM_DES *hw_iot_malloc_des(void)
{
    /* 分配描述结构体内存 */
    HW_IOT_DM_DES *des = (HW_IOT_DM_DES *)malloc(sizeof(HW_IOT_DM_DES));
    if (!des) // 分配内存失败
    {
        ESP_LOGE(TAG, "Failed to allocate HW_IOT_DM_DES");
        return NULL;
    }
    memset(des, 0, sizeof(HW_IOT_DM_DES)); // 初始化内存

    /* 创建根JSON对象: { "services": [...] } */
    des->services_json = cJSON_CreateObject(); // 创建JSON对象
    if (!des->services_json)                   // 创建JSON对象失败
    {
        ESP_LOGE(TAG, "Failed to create cJSON object");
        free(des);
        return NULL;
    }

    /* 创建services数组并添加到根对象 */
    cJSON *services = cJSON_CreateArray(); // 创建JSON数组
    if (!services)                         // 创建JSON数组失败
    {
        ESP_LOGE(TAG, "Failed to create services array");
        cJSON_Delete(des->services_json);
        free(des);
        return NULL;
    }

    cJSON_AddItemToObject(des->services_json, "services", services); // 添加services数组到根对象
    return des;
}

/**
 * @brief 向物模型添加服务属性
 *
 * 向指定的 service_id 服务中添加一个数值属性。
 * 如果该 service_id 不存在，会自动创建新的服务对象。
 * 如果已存在，则在现有服务的 properties 中添加新属性。
 *
 * 生成的 JSON 结构:
 * {
 *   "services": [
 *     {
 *       "service_id": "BasicData",
 *       "properties": { "luminance": 30 }
 *     }
 *   ]
 * }
 *
 * @param des        物模型描述结构体（由 hw_iot_malloc_des 创建）
 * @param service_id 服务 ID，如 "BasicData"、"Temperature"
 * @param name       属性名，如 "luminance"、"value"
 * @param val        属性值（整数）
 */
void hw_iot_set_mes_des(HW_IOT_DM_DES *des, const char *service_id, const char *name, int val)
{
    if (!des || !des->services_json || !service_id || !name) // 参数为空
    {
        return;
    }

    cJSON *services = cJSON_GetObjectItem(des->services_json, "services"); // 获取services json根对象
    if (!services)                                                         // services json根对象为空
    {
        return;
    }

    // 查找是否已存在该service_id的服务对象
    cJSON *service = NULL;
    // 这段代码的意图就是：
    // 1. 查找复用：遍历 services 数组，看是否已经存在相同 service_id 的服务对象。
    // 2. 避免重复创建：如果找到了，break 跳出循环，service 指针就指向已存在的对象，后续直接往里加属性。
    // 3. 不存在则新建：如果遍历完没找到，service 为 NULL，就会进入下面的 if (!service) 分支创建一个新的服务对象。
    cJSON_ArrayForEach(service, services) // 遍历services数组中的每个服务对象
    {
        cJSON *sid = cJSON_GetObjectItem(service, "service_id");                  // 底层是 for 循环，获取service_id属性
        if (sid && sid->valuestring && strcmp(sid->valuestring, service_id) == 0) // 找到匹配的服务对象
        {
            break;
        }
    }

    // 如果服务不存在，创建新的服务对象: { "service_id": "xxx", "properties": {} }
    if (!service)
    {
        service = cJSON_CreateObject(); // 创建json对象
        if (!service)                   // 创建json对象失败
        {
            ESP_LOGE(TAG, "Failed to create service object");
            return;
        }

        cJSON_AddStringToObject(service, "service_id", service_id); // 添加service_id到json对象

        cJSON *properties = cJSON_CreateObject(); // 创建json对象
        if (!properties)                          // 创建json对象失败
        {
            cJSON_Delete(service);
            return;
        }
        cJSON_AddItemToObject(service, "properties", properties); // 添加properties到json对象
        cJSON_AddItemToArray(services, service);                  // 添加新的JSON对象到JSON根对象
    }

    // 向该服务的properties中添加属性: { "name": val }
    cJSON *properties = cJSON_GetObjectItem(service, "properties"); // 这种方式就算properties是否存在都能添加
    if (properties)                                                 // properties存在
    {
        // "properties": { "luminance": 30 }
        cJSON_AddNumberToObject(properties, name, val); // 添加属性到json对象
    }
}

/**
 * @brief 向指定服务添加自定义 event_time 字段
 *
 * event_time 是可选字段，用于标识设备采集数据的时间。
 * 如果不添加此字段，平台会以收到数据的时间为准。
 *
 * @param des        物模型描述结构体
 * @param service_id 服务 ID
 * @param time_str   UTC 时间字符串，格式必须为 yyyyMMddTHHmmssZ
 *                   例如: "20161219T114920Z"
 */
void hw_iot_set_mes_des_time(HW_IOT_DM_DES *des, const char *service_id, const char *time_str)
{
    if (!des || !des->services_json || !service_id || !time_str) // 参数为空
    {
        return;
    }

    cJSON *services = cJSON_GetObjectItem(des->services_json, "services"); // 获取services数组
    if (!services)                                                         // services数组不存在
    {
        return;
    }

    /* 查找对应的service_id的服务对象 */
    cJSON *service = NULL;
    cJSON_ArrayForEach(service, services)
    {
        cJSON *sid = cJSON_GetObjectItem(service, "service_id");
        if (sid && sid->valuestring && strcmp(sid->valuestring, service_id) == 0)
        {
            break;
        }
    }

    // 添加event_time字段 (可选字段，格式: yyyyMMddTHHmmssZ)
    if (service)
    {
        cJSON_AddStringToObject(service, "event_time", time_str);
    }
}

/**
 * @brief 向指定服务添加当前 UTC 时间
 *
 * 自动获取系统当前 UTC 时间，格式化为华为云要求的格式后添加到服务中。
 * 等价于 hw_iot_set_mes_des_time(des, service_id, "当前UTC时间")
 *
 * @param des        物模型描述结构体
 * @param service_id 服务 ID
 */
void hw_iot_set_current_time(HW_IOT_DM_DES *des, const char *service_id)
{
    if (!des || !service_id) // 参数检查
    {
        return;
    }

    /* 获取当前UTC时间并格式化为华为云要求的格式 */
    time_t now = time(NULL);                                         // 获取当前时间戳
    struct tm *tm_info = gmtime(&now);                               // 获取UTC时间结构体
    char time_buf[32];                                               // 用于存储格式化后的UTC时间字符串
    strftime(time_buf, sizeof(time_buf), "%Y%m%dT%H%M%SZ", tm_info); // 格式化UTC时间字符串

    hw_iot_set_mes_des_time(des, service_id, time_buf); // 添加UTC时间到服务中
}

/**
 * @brief 将物模型结构体序列化为 JSON 字符串
 *
 * 将 cJSON 对象转换为无格式的 JSON 字符串，保存到 des->mes_js_str 中。
 * 必须在调用 hw_iot_report_properties() 之前调用此函数。
 * 支持重复调用，每次调用会释放旧的字符串并重新生成。
 *
 * 序列化后:
 *   des->mes_js_str 指向 JSON 字符串
 *   des->mes_js_len  存储字符串长度
 *
 * @param des 物模型描述结构体
 */
void hw_iot_mes_string(HW_IOT_DM_DES *des)
{
    if (!des || !des->services_json) // 参数为空
    {
        return;
    }

    /* 释放旧的JSON字符串 (支持重复调用) */
    if (des->mes_js_str)
    {
        free(des->mes_js_str);
        des->mes_js_str = NULL;
        des->mes_js_len = 0;
    }

    /* 将cJSON对象序列化为无格式JSON字符串 */
    char *json_str = cJSON_PrintUnformatted(des->services_json);
    if (json_str)
    {
        des->mes_js_len = strlen(json_str);
        des->mes_js_str = (char *)malloc(des->mes_js_len + 1);
        if (des->mes_js_str) // 内存分配成功
        {
            strcpy(des->mes_js_str, json_str);
        }
        cJSON_free(json_str);
    }
}

/**
 * @brief 释放物模型描述结构体
 *
 * 释放结构体及其内部所有动态分配的内存，包括:
 * 1. cJSON 对象（cJSON_Delete 会自动释放所有子节点）
 * 2. JSON 字符串（mes_js_str）
 * 3. 结构体本身
 *
 * 调用此函数后，des 指针不可再使用。
 *
 * @param des 物模型描述结构体
 */
void hw_iot_free_des(HW_IOT_DM_DES *des)
{
    if (!des) // 参数为空
    {
        return;
    }

    /* 释放cJSON对象 (会自动释放其子节点) */
    if (des->services_json)
    {
        cJSON_Delete(des->services_json);
    }

    /* 释放JSON字符串 */
    if (des->mes_js_str)
    {
        free(des->mes_js_str);
    }

    /* 释放结构体本身 */
    free(des);
}

/**
 * @brief 上报设备属性到华为云 IoT 平台
 *
 * 将序列化后的 JSON 数据通过 MQTT 发布到华为云 IoTDA 平台。
 *
 * Topic 格式: $oc/devices/{device_id}/sys/properties/report
 * QoS: 0
 *
 * 调用前必须确保:
 * 1. 已调用 hw_iot_mes_string() 完成序列化
 * 2. MQTT 客户端已初始化并连接成功
 *
 * @param des 物模型描述结构体
 * @return 0 表示成功，-1 表示失败
 */
int hw_iot_report_properties(HW_IOT_DM_DES *des)
{
    if (!des || !des->mes_js_str || des->mes_js_len <= 0) // 参数为空
    {
        ESP_LOGE(TAG, "Invalid message data");
        return -1;
    }

    if (!mqtt_handle) // 因为 mqtt_handle 是全局变量，所以可以检测 mqtt 客户端是否初始化
    {
        ESP_LOGE(TAG, "MQTT client not initialized");
        return -1;
    }

    /* 华为云IoT属性上报Topic: $oc/devices/{device_id}/sys/properties/report */
    const char *topic = hw_iot_mqtt_topic_get(HW_IOT_TOPIC_PROPERTIES_REPORT, HW_IOT_USERNAME, NULL);

    /* 发布MQTT消息 (QoS=0) */
    int msg_id = esp_mqtt_client_publish(mqtt_handle, topic, des->mes_js_str, des->mes_js_len, 0, 0);
    if (msg_id < 0) // 发布消息失败
    {
        ESP_LOGE(TAG, "Failed to publish message, msg_id=%d", msg_id);
        return -1;
    }

    ESP_LOGI(TAG, "Properties reported, topic: %s, msg_id: %d", topic, msg_id);
    ESP_LOGI(TAG, "Payload: %s", des->mes_js_str);

    hw_iot_free_des(des); // 释放物模型描述结构体
    des = NULL;
    ESP_LOGI(TAG, "Properties reported, des freed");

    return 0;
}
