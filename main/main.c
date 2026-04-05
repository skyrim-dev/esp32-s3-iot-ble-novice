#include <stdio.h>
#include <esp_log.h>
#include <cJSON.h>

static const char* json_str = 
"{"
    "\"id\": \"123\","
    "\"version\": \"1.0\","
    "\"sys\": {"
        "\"ack\": 0"
    "},"
    "\"params\": {"
        "\"power\": {"
            "\"value\": \"on\","
            "\"time\": 1234567890"
        "}"
    "},"
    "\"method\": \"thing.event.property.post\","
    "\"array\": [\"string1\", \"string2\", \"string3\"]"
"}";

void app_main(void)
{
    ESP_LOGI("main", "Hello world!");
    /************************************************/
    /* 解析JSON字符串 */
    /*************************************************/
    cJSON* js_root = cJSON_Parse(json_str); // 解析JSON字符串
    // id
    cJSON* id_js = cJSON_GetObjectItem(js_root, "id");  // 获取id字段
    ESP_LOGI("main", "id: %s", cJSON_GetStringValue(id_js));
    // version
    cJSON* version_js = cJSON_GetObjectItem(js_root, "version");  // 获取version字段
    ESP_LOGI("main", "version: %.1lf", cJSON_GetNumberValue(version_js));
    // sys
    cJSON* sys_js = cJSON_GetObjectItem(js_root, "sys");  // 获取sys字段
    cJSON* ack_js = cJSON_GetObjectItem(sys_js, "ack");  // 获取ack字段
    ESP_LOGI("main", "ack: %.1lf", cJSON_GetNumberValue(ack_js));
    // params
    cJSON* params_js = cJSON_GetObjectItem(js_root, "params");  // 获取params字段
    cJSON* power_js = cJSON_GetObjectItem(params_js, "power");  // 获取power字段
    cJSON* value_js = cJSON_GetObjectItem(power_js, "value");  // 获取value字段
    cJSON* time_js = cJSON_GetObjectItem(power_js, "time");  // 获取time字段
    ESP_LOGI("main", "value: %s", cJSON_GetStringValue(value_js));
    ESP_LOGI("main", "time: %d", cJSON_GetNumberValue(time_js));
    // method
    cJSON* method_js = cJSON_GetObjectItem(js_root, "method");  // 获取method字段
    ESP_LOGI("main", "method: %s", cJSON_GetStringValue(method_js));
    // array
    cJSON* array_js = cJSON_GetObjectItem(js_root, "array");  // 获取array字段
    cJSON* sub_js = array_js->child;
    int num = 0;
    while(sub_js)
    {
        ESP_LOGI("main", "array[%d]: %s", num, cJSON_GetStringValue(sub_js));
        sub_js = sub_js->next;
        num++;       
    }
    cJSON_Delete(js_root);
    /************************************************/
    /* 生成JSON字符串 */
    /*************************************************/
    cJSON* js_root_new = cJSON_CreateObject();
    // id
    cJSON_AddStringToObject(js_root_new, "id", "123");
    // version
    cJSON_AddStringToObject(js_root_new, "version", "1.0");
    // sys
    cJSON* sys_js_new = cJSON_CreateObject();
    cJSON_AddNumberToObject(sys_js_new, "ack", 1.0);
    cJSON_AddItemToObject(sys_js_new, "sys", sys_js_new);
    // param
    cJSON* param_js_new = cJSON_CreateObject();
    cJSON* power_js_new = cJSON_CreateObject();
    cJSON_AddStringToObject(power_js_new, "value", "on");
    cJSON_AddNumberToObject(power_js_new, "time", 1234567890);
    cJSON_AddItemToObject(param_js_new, "Power", power_js_new);
    cJSON_AddItemToObject(js_root_new, "Params", power_js_new);
    // method
    cJSON_AddStringToObject(js_root_new, "method", "thing.event.property.post");
    // array
    cJSON* array_js_new = cJSON_CreateArray();
    cJSON_AddItemToArray(array_js_new, cJSON_CreateString("string2"));
    cJSON_AddItemToArray(array_js_new, cJSON_CreateString("string1"));
    cJSON_AddItemToArray(array_js_new, cJSON_CreateString("string3"));
    cJSON_AddItemToObject(js_root_new, "array", array_js_new);

    char* js_str_new = cJSON_Print(js_root_new);

    ESP_LOGI("main", "js: \r\n: %s", js_str_new);
    cJSON_free(js_str_new);
    cJSON_Delete(js_root_new);
}
