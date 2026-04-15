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






