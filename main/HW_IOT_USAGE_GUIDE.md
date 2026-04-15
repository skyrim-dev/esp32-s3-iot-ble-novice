# 华为云 IoT 物模型属性上报 - 指南

## 概述

本模块用于 ESP32 设备通过 MQTT 协议向华为云 IoTDA 平台上报设备属性数据。遵循华为云 IoT 平台定义的物模型（Device Model）通信规范。

**参考文档**：
- [华为云 IoT 设备属性上报 API](https://support.huaweicloud.com/api-iothub/iot_06_v5_3010.html)
- [华为云 IoT 快速入门](https://support.huaweicloud.com/qs-iothub/iot_05_00121.html)

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
├── mqtt_hw_iot_message_up.h    # 头文件：结构体定义和 API 声明
├── mqtt_hw_iot_message_up.c    # 源文件：函数实现
├── mqtt_hw_iot.h               # MQTT 配置（设备 ID、密钥等）
├── mqtt_hw_iot.c               # MQTT 客户端初始化
├── main.c                      # 主程序入口
└── CMakeLists.txt              # 构建配置
```
