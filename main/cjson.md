# cJSON 知识点

**存在一段 JSON：**

    {
        "id": "123",
        "version": "1.0",
        "sys": {
            "ack": 0
        },
        "params": {
            "Power": {
                "value": "on",
                "time": 1524448722000
            }
        },
        "method": "thing.event.property.post",
        "array": ["string1", "string2", "string3"]
    }

## 解析 JSON 字符串

    cJSON *js_root = cJSON_Parse(json_str);  // 解析整个 json_str 字符串，获取 js_root 对象


    /* 解析 id 字段的字符串值 */
    cJSON *id_js = cJSON_GetObjectItem(js_root, "id");  // 获取 js_root 对象中的 id 字段
    printf("id: %s\n", cJSON_GetValueString(id_js));    // 提取 id 字段的字符串值


    /* 解析 version 字段的字符串值 */
    cJSON *version_js = cJSON_GetObjectItem(js_root, "version");    // 获取 js_root 对象中的 version 字段
    printf("version: %s\n", cJSON_GetValueString(version_js));      // 提取 version 字段的字符串值


    /* 取得对象 sys 中的 ack 数值 */
    cJSON* sys_js = cJSON_GetObjectItem(js_root, "sys"); // 获取 js_root 对象中的 sys 字段
    cJSON* ack_js = cJSON_GetObjectItem(sys_js, "ack");  // 获取 sys 对象中的 ack 字段的值
    printf("ack: %.1lf\n", cJSON_GetNumberValue(ack_js));


    /* 取得对象 params 中的对象 Power 中的 value 字段的字符串值和 time 字段的数值 */
    cJSON* params_js = cJSON_GetObjectItem(js_root, "params"); // 获取 js_root 对象中的 params 字段
    cJSON* Power_js = cJSON_GetObjectItem(params_js, "Power");  // 获取 params 对象中的 Power 字段的值
    cJSON* value_js = cJSON_GetObjectItem(Power_js, "value");  // 获取 Power 对象中的 value 字段的字符串值
    cJSON* time_js = cJSON_GetObjectItem(Power_js, "time");  // 获取 Power 对象中的 time 字段的数值
    printf("value: %s\n", cJSON_GetValueString(value_js));
    printf("time: %.1lf\n", cJSON_GetNumberValue(time_js));


    /* 取得 method 字段的字符串值 */
    cJSON *method_js = cJSON_GetObjectItem(js_root, "method");    // 获取 js_root 对象中的 method 字段
    printf("method: %s\n", cJSON_GetValueString(method_js));      // 提取 method 字段的字符串值


    /* 取得 array 字段中的所有元素 */
    cJSON *array_js = cJSON_GetObjectItem(js_root, "array");    // 获取 js_root 对象中的 array 字段
    cJSON *sub_js = array_js->child;   // 获取 array 字段中的第一个元素
    int loop = 0;
    while (sub_js != NULL)  // 循环打印 array 字段中的所有元素，直到 sub_js 为 NULL 时停止
    {
        printf("array[%d]: %s\n", loop, cJSON_GetValueString(sub_js));
        sub_js = sub_js->next;  // 获取 array 字段中的下一个元素
        loop++;
    }


    /* 释放 cJSON 对象 */
    cJSON_Delete(js_root);

## 使用 cJSON 生成 JSON 字符串

    cJSON *js_root = cJSON_CreateObject();    // 创建一个 cJSON 对象

    /* id 字符串 */
    cJSON_AddStringToObject(js_root, "id", "123");  // 添加 id 字段，值为 "123"

    /* version 字符串 */
    cJSON_AddStringToObject(js_root, "version", "1.0");  // 添加 version 字段，值为 "1.0"

    /* sys 对象 ack 数值 */
    cJSON *sys_js = cJSON_CreateObject();  // 创建一个 cJSON 对象 sys_js
    cJSON_AddNumberToObject(sys_js, "ack", 0);  // 添加 ack 字段，值为 0
    cJSON_AddObjectToObject(js_root, "sys", sys_js);  // 将 sys_js 对象添加到 js_root 对象中，作为 sys 字段

    /* params 对象 Power 对象 value 字符串 time 数值 1524448722000 */
    cJSON *params_js = cJSON_CreateObject();  // 创建一个 cJSON 对象 params_js
    cJSON *Power_js = cJSON_CreateObject();  // 创建一个 cJSON 对象 Power_js
    cJSON_AddStringToObject(Power_js, "value", "on");  // 添加 value 字段，值为 "on"
    cJSON_AddNumberToObject(Power_js, "time", 1524448722000);  // 添加 time 字段，值为 1524448722000
    cJSON_AddObjectToObject(params_js, "Power", Power_js);  // 将 Power_js 对象添加到 params_js 对象中，作为 Power 字段
    cJSON_AddObjectToObject(js_root, "params", params_js);  // 将 params_js 对象添加到 js_root 对象中，作为 params 字段

    /* method 字符串 */
    cJSON_AddStringToObject(js_root, "method", "thing.event.property.post");  // 添加 method 字段，值为 "thing.event.property.post"

    /* array 数组对象 */
    cJSON *array_js = cJSON_CreateArray();  // 创建一个 cJSON 数组对象 array_js
    cJSON_AddStringToObject(array_js, cJSON_CreateString("string1"));  // 添加 array 字段，值为 "string1"
    cJSON_AddStringToObject(array_js, cJSON_CreateString("string2"));  // 添加 array 字段，值为 "string2"
    cJSON_AddStringToObject(array_js, cJSON_CreateString("string3"));  // 添加 array 字段，值为 "string3"
    cJSON_AddArrayToObject(js_root, "array", array_js);  // 将 array_js 数组对象添加到 js_root 对象中，作为 array 字段

    /* 生成 JSON 字符串 */
    char *js_str = cJSON_PrintUnformatted(js_root);  // 生成 JSON 字符串，不包含格式化字符
    ESP_LOGI(TAG, "js:\r\n:%s", js_str);
    free(js_str);


