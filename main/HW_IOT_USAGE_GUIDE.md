# 华为云 IoT - 指南

## 概述

本模块用于 ESP32 设备通过 MQTT 协议与华为云 IoTDA 平台通信：
1. **属性上报** - 向华为云上报设备属性数据
2. **命令接收** - 接收华为云下发的命令并返回响应

**参考文档**：
- [华为云 IoT 设备属性上报 API](https://support.huaweicloud.com/api-iothub/iot_06_v5_3010.html)
- [华为云 IoT 平台命令下发 API](https://support.huaweicloud.com/api-iothub/iot_06_v5_3014.html)
- [华为云 IoT 设备消息上报 API](https://support.huaweicloud.com/api-iothub/iot_06_v5_3016.html)
- [华为云 IoT 平台消息下发 API](https://support.huaweicloud.com/api-iothub/iot_06_v5_3017.html)

## 数据格式

设备上报的 JSON 数据必须符合以下格式：

```json
{
    "services": [
        {
            "service_id": "BasicData",
            "properties": {
                "luminance": 30
            },
            "event_time": "20161219T114920Z"
        },
        {
            "service_id": "Temperature",
            "properties": {
                "value": 25,
                "humidity": 60
            }
        }
    ]
}
```

### 字段说明

| 字段 | 必选 | 类型 | 说明 |
|------|------|------|------|
| `services` | 必选 | Array | 设备服务数据列表 |
| `service_id` | 必选 | String | 服务 ID，由产品模型定义（如 `BasicData`、`Temperature`） |
| `properties` | 必选 | Object | 属性键值对，具体字段在产品模型中定义 |
| `event_time` | 可选 | String | UTC 时间，格式：`yyyyMMddTHHmmssZ`（如 `20161219T114920Z`） |

## MQTT Topic

```
$oc/devices/{device_id}/sys/properties/report
```

其中 `{device_id}` 由 `mqtt_hw_iot.h` 中的 `HW_IOT_USERNAME` 定义。

## API 参考

### 数据结构

```c
typedef struct {
    cJSON *services_json;    // 根 JSON 对象
    char  *mes_js_str;       // 序列化后的 JSON 字符串
    int    mes_js_len;       // JSON 字符串长度
} HW_IOT_DM_DES;
```

### 函数列表

| 函数 | 说明 |
|------|------|
| `hw_iot_malloc_des()` | 创建物模型描述结构体 |
| `hw_iot_set_mes_des(des, service_id, name, val)` | 向服务添加数值属性 |
| `hw_iot_set_mes_des_time(des, service_id, time_str)` | 向服务添加自定义时间 |
| `hw_iot_set_current_time(des, service_id)` | 向服务添加当前 UTC 时间 |
| `hw_iot_mes_string(des)` | 将结构体序列化为 JSON 字符串 |
| `hw_iot_report_properties(des)` | 通过 MQTT 上报到华为云平台 |
| `hw_iot_free_des(des)` | 释放结构体内存 |

## 使用步骤

### 1. 引入头文件

```c
#include "mqtt_hw_iot_message_up.h"
```

### 2. 基本使用流程

```c
// 1. 创建描述结构体
HW_IOT_DM_DES *des = hw_iot_malloc_des();
if (!des) {
    // 内存分配失败处理
    return;
}

// 2. 添加属性数据
hw_iot_set_mes_des(des, "BasicData", "luminance", 30);
hw_iot_set_mes_des(des, "BasicData", "temperature", 25);
hw_iot_set_mes_des(des, "Temperature", "value", 25);
hw_iot_set_mes_des(des, "Temperature", "humidity", 60);

// 3. 添加时间戳（可选）
hw_iot_set_current_time(des, "BasicData");

// 4. 序列化为 JSON
hw_iot_mes_string(des);

// 5. 上报到华为云平台
int ret = hw_iot_report_properties(des);
if (ret == 0) {
    // 上报成功
}

// 6. 释放内存
hw_iot_free_des(des);
```

### 3. 多服务多属性示例

```c
HW_IOT_DM_DES *des = hw_iot_malloc_des();
if (des) {
    // 基础数据服务
    hw_iot_set_mes_des(des, "BasicData", "luminance", 30);
    hw_iot_set_current_time(des, "BasicData");

    // 温度服务
    hw_iot_set_mes_des(des, "Temperature", "value", 25);
    hw_iot_set_mes_des(des, "Temperature", "humidity", 60);
    hw_iot_set_current_time(des, "Temperature");

    // 电池服务
    hw_iot_set_mes_des(des, "Battery", "level", 80);

    hw_iot_mes_string(des);
    hw_iot_report_properties(des);
    hw_iot_free_des(des);
}
```

生成的 JSON：
```json
{
    "services": [
        {
            "service_id": "BasicData",
            "properties": { "luminance": 30 },
            "event_time": "20260405T120000Z"
        },
        {
            "service_id": "Temperature",
            "properties": { "value": 25, "humidity": 60 },
            "event_time": "20260405T120000Z"
        },
        {
            "service_id": "Battery",
            "properties": { "level": 80 }
        }
    ]
}
```

### 4. 自定义时间戳

```c
HW_IOT_DM_DES *des = hw_iot_malloc_des();
if (des) {
    hw_iot_set_mes_des(des, "BasicData", "luminance", 30);

    // 使用自定义时间（格式必须为 yyyyMMddTHHmmssZ）
    hw_iot_set_mes_des_time(des, "BasicData", "20260405T120000Z");

    hw_iot_mes_string(des);
    hw_iot_report_properties(des);
    hw_iot_free_des(des);
}
```

### 5. 定时上报（FreeRTOS 任务）

```c
void property_report_task(void *pvParameters)
{
    while (1) {
        HW_IOT_DM_DES *des = hw_iot_malloc_des();
        if (des) {
            // 读取传感器数据
            int luminance = read_luminance_sensor();
            int temperature = read_temperature_sensor();

            hw_iot_set_mes_des(des, "BasicData", "luminance", luminance);
            hw_iot_set_mes_des(des, "Temperature", "value", temperature);
            hw_iot_set_current_time(des, "BasicData");

            hw_iot_mes_string(des);
            hw_iot_report_properties(des);
            hw_iot_free_des(des);
        }

        // 每 30 秒上报一次
        vTaskDelay(pdMS_TO_TICKS(30000));
    }
}
```

## 注意事项

### 内存管理

- **必须配对调用**：`hw_iot_malloc_des()` 和 `hw_iot_free_des()` 必须配对使用，否则会造成内存泄漏
- **使用完立即释放**：上报完成后应立即调用 `hw_iot_free_des()`

```c
// 正确用法
HW_IOT_DM_DES *des = hw_iot_malloc_des();
if (des) {
    // ... 添加属性、上报
    hw_iot_free_des(des);  // 必须调用
}
```

### 调用顺序

```
hw_iot_malloc_des()
    ↓
hw_iot_set_mes_des() / hw_iot_set_current_time()
    ↓
hw_iot_mes_string()          ← 必须先序列化才能上报
    ↓
hw_iot_report_properties()
    ↓
hw_iot_free_des()
```

### 前提条件

- WiFi 已连接
- MQTT 客户端已初始化（`mqtt_hw_iot_init()` 已调用）
- 设备已在华为云平台注册

### 属性值类型限制

当前实现仅支持 **整数类型** 属性值。如需支持浮点数或字符串，需修改 `hw_iot_set_mes_des()` 函数或添加新的 API。

### 错误处理

| 场景 | 返回值/行为 |
|------|-------------|
| 内存分配失败 | `hw_iot_malloc_des()` 返回 `NULL` |
| 未序列化就上报 | `hw_iot_report_properties()` 返回 `-1` |
| MQTT 未初始化 | `hw_iot_report_properties()` 返回 `-1` |
| 发布失败 | `hw_iot_report_properties()` 返回 `-1` |

## 日志输出

模块使用 `hw_iot_msg` 标签输出日志：

```
I (xxx) hw_iot_msg: Properties reported, topic: $oc/devices/xxx/sys/properties/report, msg_id: 1
I (xxx) hw_iot_msg: Payload: {"services":[{"service_id":"BasicData","properties":{"luminance":30}}]}
```

## 文件结构

```
main/
├── mqtt_hw_iot_message_up.h    # 头文件：属性上报 API 声明
├── mqtt_hw_iot_message_up.c    # 源文件：属性上报实现
├── mqtt_hw_iot_command_receive.h   # 头文件：命令接收 API 声明
├── mqtt_hw_iot_command_receive.c   # 源文件：命令接收实现
├── mqtt_hw_iot.h               # MQTT 配置（设备 ID、密钥等）
├── mqtt_hw_iot.c               # MQTT 客户端初始化和命令接收回调
├── main.c                      # 主程序入口
└── CMakeLists.txt              # 构建配置
```

---

## 命令接收

### Topic 规范

| 方向 | Topic |
|------|-------|
| 下行（接收） | `$oc/devices/{device_id}/sys/commands/request_id={request_id}` |
| 上行（响应） | `$oc/devices/{device_id}/sys/commands/response/request_id={request_id}` |

### 下行数据格式

```json
{
    "object_device_id": "{子设备ID}",
    "command_name": "ON_OFF",
    "service_id": "WaterMeter",
    "paras": {
        "value": "1"
    }
}
```

### 上行响应格式

```json
{
    "result_code": 0,
    "response_name": "COMMAND_RESPONSE",
    "paras": {
        "result": "success"
    }
}
```

### 数据结构

```c
typedef struct {
    char topic[128];         // MQTT Topic
    char request_id[128];   // 从 Topic 解析的 request_id
    cJSON *command_js;      // 命令 JSON 对象
    cJSON *object_device_id; // 设备ID字段
    cJSON *service_id;      // 服务ID字段
    cJSON *command_name;    // 命令名称字段
    cJSON *paras;           // 参数字段
} hw_iot_cmd_receive_t;

extern hw_iot_cmd_receive_t hw_iot_cmd_receive;
```

### 函数

| 函数 | 说明 |
|------|------|
| `hw_iot_command_receive_ack(char *request_id)` | 发送命令响应 ACK 到华为云平台 |

### 使用示例

```c
// 在 MQTT 回调中处理命令接收
void mqtt_event_callback(void *event_handler_arg, esp_event_base_t event_base,
                         int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t receive_data = event_data;
    if (event_id == MQTT_EVENT_DATA) {
        if (strstr(receive_data->topic, "/sys/commands/")) {
            // 复制 topic 并添加 null terminator
            int copy_len = (receive_data->topic_len < sizeof(hw_iot_cmd_receive.topic) - 1) 
                           ? receive_data->topic_len : sizeof(hw_iot_cmd_receive.topic) - 1;
            memcpy(hw_iot_cmd_receive.topic, receive_data->topic, copy_len);
            hw_iot_cmd_receive.topic[copy_len] = '\0';

            // 从 topic 中解析 request_id
            char *ptr = strstr(hw_iot_cmd_receive.topic, "request_id=");
            if (ptr) {
                ptr += strlen("request_id=");
                char *end = strchr(ptr, '/');
                int len = end ? (end - ptr) : strlen(ptr);
                if (len < sizeof(hw_iot_cmd_receive.request_id)) {
                    memcpy(hw_iot_cmd_receive.request_id, ptr, len);
                    hw_iot_cmd_receive.request_id[len] = '\0';
                }
            }

            // 解析命令 JSON
            hw_iot_cmd_receive.command_js = cJSON_Parse(receive_data->data);
            hw_iot_cmd_receive.paras = cJSON_GetObjectItem(hw_iot_cmd_receive.command_js, "paras");
            hw_iot_cmd_receive.command_name = cJSON_GetObjectItem(hw_iot_cmd_receive.command_js, "command_name");

            // 执行命令处理...
            // ...

            // 发送响应 ACK
            hw_iot_command_receive_ack(hw_iot_cmd_receive.request_id);
        }
    }
}
```

### 日志输出

模块使用 `hw_iot_cmd_ack` 标签输出日志：

```
I (xxx) hw_iot_cmd_ack: request_id: xxx-xxx-xxx
I (xxx) hw_iot_cmd_ack: Response topic: $oc/devices/xxx/sys/commands/response/request_id=xxx-xxx-xxx
I (xxx) hw_iot_cmd_ack: Response payload: {"result_code":0,"response_name":"COMMAND_RESPONSE","paras":{"result":"success"}}
I (xxx) hw_iot_cmd_ack: ACK published successfully, msg_id: 1
```

### Topic 解析要点

MQTT 回调中的 `topic` 是非 NULL 结尾的缓冲区，必须使用 `topic_len` 来正确解析：

```c
// 错误做法
char *topic = receive_data->topic;
printf("%s", topic);  // 可能崩溃或输出乱码

// 正确做法
ESP_LOGI("mqtt", "topic: %.*s", receive_data->topic_len, receive_data->topic);
```

从 Topic 解析 request_id 的核心逻辑：
1. 先复制到本地缓冲区并添加 `\0` 结尾
2. 使用 `strstr()` 查找 "request_id=" 
3. 使用 `strchr()` 找到下一个 '/' 作为结束位置

---

## 设备消息上报

### 功能说明

当设备无法按照产品模型中定义的属性格式进行数据上报时，可使用设备消息上报接口将设备的自定义数据格式上报给平台。平台对该消息不进行解析，可转发给应用服务器或华为云其他云服务进行存储和处理。

**与属性上报的区别**：
- **属性上报**：需符合产品模型定义的属性格式，平台会解析处理
- **消息上报**：平台不解析数据内容，仅透传，适合自定义格式或二进制数据

### Topic

```
$oc/devices/{device_id}/sys/messages/up
```

### 数据格式

设备消息上报对数据内容不做固定要求，可自定义格式：

**方式1：仅发送消息内容**
```
hello!
```

**方式2：系统格式上报**
```json
{
    "object_device_id": "{子设备ID}",
    "name": "message_name",
    "id": "aca6a906-c74c-4302-a2ce-b17ba2ce630c",
    "content": "hello!"
}
```

### 参数说明

| 字段 | 必选 | 类型 | 说明 |
|------|------|------|------|
| `object_device_id` | 可选 | String | 网关子设备上报时填写子设备ID |
| `name` | 可选 | String | 消息名称，可不填写 |
| `id` | 可选 | String | 消息唯一标识，如不填写平台自动生成 |
| `content` | 必选 | String/Object | 消息内容 |

### 使用示例

```c
#include <mqtt_client.h>

extern esp_mqtt_client_handle_t mqtt_handle;

// 上报简单字符串
const char *msg = "hello!";
esp_mqtt_client_publish(mqtt_handle, 
    "$oc/devices/" HW_IOT_USERNAME "/sys/messages/up",
    msg, strlen(msg), 0, 0);

// 上报系统格式 JSON
char json_buf[256];
snprintf(json_buf, sizeof(json_buf),
    "{\"name\":\"sensor_data\",\"content\":\"%s\"}", data);
esp_mqtt_client_publish(mqtt_handle,
    "$oc/devices/" HW_IOT_USERNAME "/sys/messages/up",
    json_buf, strlen(json_buf), 0, 0);
```

---

## 平台消息下发

### 功能说明

平台可以向设备下发自定义格式的消息，设备接收后自行解析处理。

### Topic

```
$oc/devices/{device_id}/sys/messages/down
```

### 数据格式

**系统格式**
```json
{
    "object_device_id": "{子设备ID}",
    "name": "message_name",
    "id": "message_id",
    "content": "hello"
}
```

**自定义格式**
```
arbitrary content
```

### 参数说明

| 字段 | 必选 | 类型 | 说明 |
|------|------|------|------|
| `object_device_id` | 可选 | String | 网关子设备下发时填写子设备ID |
| `name` | 可选 | String | 消息名称 |
| `id` | 可选 | String | 消息唯一标识 |
| `content` | 必选 | String | 消息内容，可为 base64 编码 |

### 接收处理示例

```c
void mqtt_event_callback(void *event_handler_arg, esp_event_base_t event_base,
                         int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t receive_data = event_data;
    if (event_id == MQTT_EVENT_DATA) {
        if (strstr(receive_data->topic, "/sys/messages/down")) {
            // 处理平台下发的消息
            ESP_LOGI("mqtt", "Received message: %.*s", 
                receive_data->data_len, receive_data->data);
        }
    }
}
```
