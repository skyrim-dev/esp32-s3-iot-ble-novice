# JSON 生成指南

## 1. 文档说明

本文档基于当前项目 `base-project/` 的实际代码编写，只说明本项目已经实现的 JSON 生成方式，不扩展到项目中不存在的通用框架或额外封装。

当前项目的 JSON 生成功能集中在：

- `components/protocol/hw_iot_mqtt/hw_iot_mqtt_json.h`
- `components/protocol/hw_iot_mqtt/hw_iot_mqtt_json.c`

这些函数主要用于生成发往华为云 IoT 的 MQTT 报文。

---

## 2. 当前项目中 JSON 生成的作用

本项目目前通过 cJSON 动态生成以下几类报文：

1. 属性上报 JSON
2. 命令响应 JSON
3. OTA 版本上报 JSON
4. OTA 升级状态上报 JSON

这些 JSON 最终会在：

- `components/protocol/hw_iot_mqtt/hw_iot_mqtt_publish.c`

中被调用，并通过 `hw_iot_mqtt_publish()` 发布到 MQTT topic。

---

## 3. 核心文件结构

### 3.1 头文件

文件：

- `components/protocol/hw_iot_mqtt/hw_iot_mqtt_json.h`

作用：

- 定义 JSON 对应的数据结构体
- 声明 JSON 生成函数

当前声明的生成函数有：

```c
char *hw_iot_mqtt_properties_report_json(hw_iot_mqtt_properties_report_json_t *json);
char *hw_iot_mqtt_command_response_json(hw_iot_mqtt_command_response_json_t *json);
char *hw_iot_mqtt_ota_version_report_json(hw_iot_mqtt_ota_version_report_json_t *json);
char *hw_iot_mqtt_ota_status_report_json(hw_iot_mqtt_ota_status_report_json_t *json);
```

### 3.2 实现文件

文件：

- `components/protocol/hw_iot_mqtt/hw_iot_mqtt_json.c`

作用：

- 使用 cJSON 创建根对象、数组、子对象
- 填充字段
- 将 cJSON 对象序列化成字符串并返回

---

## 4. 当前项目中的 JSON 数据结构

### 4.1 属性上报结构

对应结构体：

- `hw_iot_mqtt_properties_json_t`
- `hw_iot_mqtt_properties_report_json_t`

定义位置：

- `components/protocol/hw_iot_mqtt/hw_iot_mqtt_json.h`

特点：

- 支持多个 service
- 每个 service 支持多个 property
- 当前上限由宏控制：
  - `HW_IOT_MQTT_SERVICES_NUM`
  - `HW_IOT_MQTT_PROPERTIES_NUM`

### 4.2 命令响应结构

对应结构体：

- `hw_iot_mqtt_command_response_json_t`

字段包括：

- `result_code`
- `response_name`
- `result`

### 4.3 OTA 版本上报结构

对应结构体：

- `hw_iot_mqtt_ota_version_report_json_t`

字段包括：

- `object_device_id`
- `sw_version`
- `fw_version`

### 4.4 OTA 状态上报结构

对应结构体：

- `hw_iot_mqtt_ota_status_report_json_t`

字段包括：

- `object_device_id`
- `version`
- `description`
- `progress`
- `result_code`

该结构体还在头文件里写明了 `result_code` 的语义，例如：

- `0`：success
- `5`：download timeout
- `6`：upgrade package verification failure
- `9`：upgrade package installation failure
- `255`：internal exception

---

## 5. JSON 生成的通用模式

当前项目中的 4 个 JSON 生成函数虽然用途不同，但生成模式基本一致。

### 通用流程

1. 检查输入指针是否为空
2. 创建根对象 `cJSON_CreateObject()`
3. 按需创建数组 `cJSON_CreateArray()`
4. 按需创建子对象 `cJSON_CreateObject()`
5. 通过 `cJSON_AddStringToObject()` / `cJSON_AddNumberToObject()` 填充字段
6. 调用 `cJSON_PrintUnformatted()` 生成字符串
7. 删除 cJSON 树，返回生成好的字符串

当前项目中，这个“序列化并释放 cJSON 树”的公共逻辑被放在了静态函数：

```c
static char *cJSON_UnformattedFree(cJSON *root_js)
```

它的作用是：

- 调用 `cJSON_PrintUnformatted(root_js)`
- 打印生成结果日志
- `cJSON_Delete(root_js)`
- 返回最终字符串

---

## 6. event_time 的生成方式

当前项目中的属性上报、OTA 版本上报、OTA 状态上报，都需要带 `event_time`。

相关函数：

```c
static void hw_iot_mqtt_format_event_time(char *buf, size_t len)
```

位置：

- `components/protocol/hw_iot_mqtt/hw_iot_mqtt_json.c`

当前实现逻辑：

- 使用 `time(NULL)` 获取当前时间戳
- 若时间早于 2023-01-01，则返回兜底值 `19700101T000000Z`
- 使用 `gmtime_r()` 转成 UTC 时间
- 使用 `strftime()` 格式化为：

```text
YYYYMMDDTHHMMSSZ
```

这也是为什么当前项目要求在 MQTT 初始化前先完成时间同步。

---

## 7. 四种 JSON 的实际结构说明

### 7.1 属性上报 JSON

生成函数：

```c
hw_iot_mqtt_properties_report_json()
```

当前结构特点：

- 根对象包含 `services`
- `services` 是数组
- 每个 service 包含：
  - `service_id`
  - `properties`
  - `event_time`

在当前项目里，属性上报的使用位置是：

- `components/protocol/hw_iot_mqtt/hw_iot_mqtt_publish.c`

调用方式：

```c
char *json_str = hw_iot_mqtt_properties_report_json(&json);
```

### 7.2 命令响应 JSON

生成函数：

```c
hw_iot_mqtt_command_response_json()
```

当前结构特点：

- 根对象直接包含：
  - `result_code`
  - `response_name`
- 同时包含子对象：
  - `paras`
    - `result`

使用位置：

- `components/protocol/hw_iot_mqtt/hw_iot_mqtt_publish.c`

### 7.3 OTA 版本上报 JSON

生成函数：

```c
hw_iot_mqtt_ota_version_report_json()
```

当前结构特点：

- `service_id` 固定为 `$ota`
- `event_type` 固定为 `version_report`
- `paras` 中包含：
  - `sw_version`
  - `fw_version`

使用位置：

- `components/protocol/hw_iot_mqtt/hw_iot_mqtt_publish.c`

### 7.4 OTA 状态上报 JSON

生成函数：

```c
hw_iot_mqtt_ota_status_report_json()
```

当前结构特点：

- `service_id` 固定为 `$ota`
- `event_type` 固定为 `upgrade_progress_report`
- `paras` 中包含：
  - `result_code`
  - `progress`
  - `version`
  - `description`

使用位置：

- `components/protocol/hw_iot_mqtt/hw_iot_mqtt_publish.c`
- OTA 成功 / 失败后由 `components/service/ota/hw_iot_ota.c` 间接触发

---

## 8. 当前项目里 JSON 是如何被调用的

### 8.1 属性上报

位置：

- `components/protocol/hw_iot_mqtt/hw_iot_mqtt_publish.c`

流程：

1. 构造 `hw_iot_mqtt_properties_report_json_t`
2. 调用 `hw_iot_mqtt_properties_report_json()` 生成 JSON 字符串
3. 调用 `hw_iot_mqtt_publish()` 发布
4. `free(json_str)` 释放字符串

### 8.2 命令响应

位置：

- `components/protocol/hw_iot_mqtt/hw_iot_mqtt_publish.c`

流程：

1. 构造 `hw_iot_mqtt_command_response_json_t`
2. 调用 `hw_iot_mqtt_command_response_json()`
3. 调用 `hw_iot_mqtt_publish()`
4. 释放返回字符串

### 8.3 OTA 版本上报

位置：

- `components/protocol/hw_iot_mqtt/hw_iot_mqtt_publish.c`

流程：

1. 构造 `hw_iot_mqtt_ota_version_report_json_t`
2. 版本号来自 `get_app_version()`
3. 调用 `hw_iot_mqtt_ota_version_report_json()`
4. 发布并释放字符串

### 8.4 OTA 状态上报

位置：

- `components/protocol/hw_iot_mqtt/hw_iot_mqtt_publish.c`
- `components/service/ota/hw_iot_ota.c`

流程：

1. OTA 回调中构造 `hw_iot_mqtt_ota_status_report_json_t`
2. 调用 `hw_iot_mqtt_ota_status_report_publish()`
3. 在 publish 函数里进一步调用 `hw_iot_mqtt_ota_status_report_json()`
4. 发布并释放字符串

---

## 9. 编写新 JSON 生成函数时应遵循的当前项目风格

如果你后续要在本项目里增加新的 JSON 生成函数，建议沿用现在的写法：

### 9.1 先在头文件定义结构体

位置：

- `components/protocol/hw_iot_mqtt/hw_iot_mqtt_json.h`

做法：

- 先定义和 JSON 对应的数据结构
- 再声明 `xxx_json()` 函数

### 9.2 在实现文件中完成字段拼装

位置：

- `components/protocol/hw_iot_mqtt/hw_iot_mqtt_json.c`

做法：

- 按字段层级创建对象
- 尽量保持和华为云报文格式一致
- 时间字段统一走 `hw_iot_mqtt_format_event_time()`

### 9.3 返回值保持一致

当前项目所有 JSON 生成函数都返回：

```c
char *
```

也就是：

- 成功时返回 `cJSON_PrintUnformatted()` 生成的字符串
- 失败时返回 `NULL`

调用方负责：

- 检查返回值
- 在发布完成后 `free()`

---

## 10. 注意事项

### 10.1 返回的 JSON 字符串需要释放

当前项目的 JSON 生成函数会返回动态分配的字符串，调用方使用完成后必须：

```c
free(json_str);
```

否则会造成内存泄漏。

### 10.2 输入结构体不能为空

当前所有 JSON 生成函数都会先检查输入是否为 `NULL`。如果传入空指针，会直接返回 `NULL`。

### 10.3 event_time 依赖系统时间

如果设备未完成时间同步，`event_time` 会回退为：

```text
19700101T000000Z
```

因此要保证：

- WiFi 已连接
- `time_sync_init()` 已成功

### 10.4 固定字段不要随意修改

当前项目中部分 JSON 字段是固定协议字段，例如：

- `service_id = "$ota"`
- `event_type = "version_report"`
- `event_type = "upgrade_progress_report"`

这些字段应与当前华为云协议保持一致，不建议随意改名。

### 10.5 当前属性上报使用固定测试数据

在 `hw_iot_mqtt_publish.c` 中，属性上报目前使用的是固定测试数据：

```c
{"BasicData", {"luminance"}, {100}}
```

这说明当前项目的 JSON 生成框架已经支持通用结构，但业务内容仍然是测试示例。

---

## 11. 一句话总结

当前项目的 JSON 生成逻辑集中在 `components/protocol/hw_iot_mqtt/hw_iot_mqtt_json.c/.h`，使用 cJSON 根据结构体生成华为云 MQTT 报文字符串，并由调用方在发布后负责释放内存；如果后续要扩展新的 JSON 类型，最稳妥的方式就是继续沿用“先定义结构体、再实现 `xxx_json()`、最后由 publish 层调用并释放”的现有模式。
