# cJSON 使用指南

cJSON 是 ESP-IDF 内置的轻量级 JSON 解析库。

## 包含头文件

```c
#include <cJSON.h>
```

---

## 示例 JSON 结构

```json
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
```

---

## 解析 JSON 字符串

### 1. 解析根节点

```c
cJSON *js_root = cJSON_Parse(json_str);
if (js_root == NULL) {
    ESP_LOGE(TAG, "JSON parse error");
    return;
}
```

### 2. 获取字符串值

```c
cJSON *id_js = cJSON_GetObjectItem(js_root, "id");
printf("id: %s\n", cJSON_GetStringValue(id_js));

cJSON *version_js = cJSON_GetObjectItem(js_root, "version");
printf("version: %s\n", cJSON_GetStringValue(version_js));
```

### 3. 获取嵌套对象的值

```c
// 获取 sys.ack 数值
cJSON *sys_js = cJSON_GetObjectItem(js_root, "sys");
cJSON *ack_js = cJSON_GetObjectItem(sys_js, "ack");
printf("ack: %d\n", (int)cJSON_GetNumberValue(ack_js));

// 获取 params.Power.value 字符串
cJSON *params_js = cJSON_GetObjectItem(js_root, "params");
cJSON *power_js = cJSON_GetObjectItem(params_js, "Power");
cJSON *value_js = cJSON_GetObjectItem(power_js, "value");
printf("value: %s\n", cJSON_GetStringValue(value_js));

// 获取 params.Power.time 数值
cJSON *time_js = cJSON_GetObjectItem(power_js, "time");
printf("time: %lld\n", (int64_t)cJSON_GetNumberValue(time_js));
```

### 4. 遍历数组

```c
cJSON *array_js = cJSON_GetObjectItem(js_root, "array");
int size = cJSON_GetArraySize(array_js);

for (int i = 0; i < size; i++) {
    cJSON *item = cJSON_GetArrayItem(array_js, i);
    printf("array[%d]: %s\n", i, cJSON_GetStringValue(item));
}
```

### 5. 释放内存

```c
cJSON_Delete(js_root);
```

---

## 生成 JSON 字符串

### 1. 创建根对象

```c
cJSON *js_root = cJSON_CreateObject();
```

### 2. 添加字符串

```c
cJSON_AddStringToObject(js_root, "id", "123");
cJSON_AddStringToObject(js_root, "version", "1.0");
cJSON_AddStringToObject(js_root, "method", "thing.event.property.post");
```

### 3. 添加数值

```c
cJSON_AddNumberToObject(js_root, "ack", 0);
cJSON_AddNumberToObject(js_root, "time", 1524448722000);
```

### 4. 添加嵌套对象

```c
// 添加 sys 对象
cJSON *sys_js = cJSON_CreateObject();
cJSON_AddNumberToObject(sys_js, "ack", 0);
cJSON_AddObjectToObject(js_root, "sys", sys_js);

// 添加 params.Power 对象
cJSON *params_js = cJSON_CreateObject();
cJSON *power_js = cJSON_CreateObject();
cJSON_AddStringToObject(power_js, "value", "on");
cJSON_AddNumberToObject(power_js, "time", 1524448722000);
cJSON_AddObjectToObject(params_js, "Power", power_js);
cJSON_AddObjectToObject(js_root, "params", params_js);
```

### 5. 添加数组

```c
cJSON *array_js = cJSON_CreateArray();
cJSON_AddItemToArray(array_js, cJSON_CreateString("string1"));
cJSON_AddItemToArray(array_js, cJSON_CreateString("string2"));
cJSON_AddItemToArray(array_js, cJSON_CreateString("string3"));
cJSON_AddArrayToObject(js_root, "array", array_js);
```

### 6. 生成字符串并释放

```c
char *js_str = cJSON_PrintUnformatted(js_root);  // 无格式（无缩进）
// char *js_str = cJSON_Print(js_root);           // 有格式（有缩进）

ESP_LOGI(TAG, "JSON: %s", js_str);
free(js_str);
cJSON_Delete(js_root);
```

---

## API 速查表

### 创建对象/数组

| 函数 | 说明 |
|------|------|
| `cJSON_CreateObject()` | 创建空对象 `{}` |
| `cJSON_CreateArray()` | 创建空数组 `[]` |
| `cJSON_CreateString("xxx")` | 创建字符串 |
| `cJSON_CreateNumber(123)` | 创建数值 |
| `cJSON_CreateBool(1)` | 创建布尔值 |
| `cJSON_CreateNull()` | 创建 null |

### 添加数据

| 函数 | 说明 |
|------|------|
| `cJSON_AddStringToObject(obj, "key", "value")` | 添加字符串 |
| `cJSON_AddNumberToObject(obj, "key", 123)` | 添加数值 |
| `cJSON_AddBoolToObject(obj, "key", 1)` | 添加布尔值 |
| `cJSON_AddObjectToObject(obj, "key", child)` | 添加子对象 |
| `cJSON_AddArrayToObject(obj, "key", arr)` | 添加数组 |
| `cJSON_AddItemToArray(arr, item)` | 数组添加元素 |

### 读取数据

| 函数 | 说明 |
|------|------|
| `cJSON_Parse(str)` | 解析 JSON 字符串 |
| `cJSON_GetObjectItem(obj, "key")` | 获取对象属性 |
| `cJSON_GetArraySize(arr)` | 获取数组长度 |
| `cJSON_GetArrayItem(arr, index)` | 获取数组元素 |
| `cJSON_GetStringValue(item)` | 获取字符串值 |
| `cJSON_GetNumberValue(item)` | 获取数值 |
| `cJSON_GetArrayObject(obj, "key")` | 获取数组 |
| `cJSON_GetObjectCaseSens(obj, "key")` | 区分大小写获取 |

### 内存管理

| 函数 | 说明 |
|------|------|
| `cJSON_Delete(obj)` | 释放整个对象树 |
| `free(str)` | 释放 JSON 字符串 |

---

## 注意事项

1. **`cJSON_Parse` 返回的指针需要 `cJSON_Delete` 释放**
2. **`cJSON_Print`/`cJSON_PrintUnformatted` 返回的字符串需要 `free` 释放**
3. **遍历数组**：子节点通过 `child` 和 `next` 链接
4. **嵌套对象**：需要逐层获取后再访问子属性
