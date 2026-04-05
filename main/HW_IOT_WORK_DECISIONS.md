# 华为云 IoT 物模型属性上报 - 工作记录

> 项目：ESP32-S3 基础项目  
> 日期：2026-04-05  
> 分支：`mqtts_hw_iot_messages_up`

---

## 一、背景

在 ESP32-S3 上实现华为云 IoTDA 平台的物模型（Device Model）属性上报功能。  
设备通过 MQTT 协议将传感器数据按照华为云定义的 JSON 格式上报到云端。

### 参考文档

- [华为云 IoT 设备属性上报 API](https://support.huaweicloud.com/api-iothub/iot_06_v5_3010.html)
- [华为云 IoT 快速入门 — 属性上报与命令接收](https://support.huaweicloud.com/qs-iothub/iot_05_00121.html)

---

## 二、华为云 IoT 数据规范

### 上报 Topic

```
$oc/devices/{device_id}/sys/properties/report
```

### JSON 数据格式

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
| `service_id` | 必选 | String | 服务 ID，由产品模型定义 |
| `properties` | 必选 | Object | 属性键值对 |
| `event_time` | 可选 | String | UTC 时间，格式：`yyyyMMddTHHmmssZ` |

---

## 三、命名决策

### 结构体命名：`HW_IOT_DM_DES`

| 选项 | 含义 | 选择 |
|------|------|------|
| `HW_IOT_DM_DES` | Device Model Description | ✅ 准确描述物模型结构 |
| `HW_IOT_DM_TOPIC` | Device Model Topic | ❌ 这是 MQTT 主题，不是结构体职责 |

### 命名规范对应

| 缩写 | 全称 | 说明 |
|------|------|------|
| `HW_IOT` | Huawei IoT | 华为云物联网 |
| `DM` | Device Model | 设备模型（物模型） |
| `DES` | Description | 描述结构体 |

---

## 四、API 设计

### 数据结构

```c
typedef struct {
    cJSON *services_json;    // 根 JSON 对象，包含 services 数组
    char  *mes_js_str;       // 序列化后的 JSON 字符串
    int    mes_js_len;       // JSON 字符串长度
} HW_IOT_DM_DES;
```

### 函数列表

| 函数 | 功能 |
|------|------|
| `hw_iot_malloc_des()` | 创建物模型描述结构体，初始化 `{ "services": [] }` |
| `hw_iot_set_mes_des(des, service_id, name, val)` | 向服务添加数值属性 |
| `hw_iot_set_mes_des_time(des, service_id, time_str)` | 添加自定义 UTC 时间 |
| `hw_iot_set_current_time(des, service_id)` | 自动添加当前 UTC 时间 |
| `hw_iot_mes_string(des)` | 序列化为 JSON 字符串 |
| `hw_iot_report_properties(des)` | 通过 MQTT 上报到华为云平台 |
| `hw_iot_free_des(des)` | 释放所有内存 |

---

## 五、调用顺序

```
hw_iot_malloc_des()           ← 创建结构体
    ↓
hw_iot_set_mes_des()          ← 添加属性（可多次调用）
hw_iot_set_current_time()     ← 添加时间（可选）
    ↓
hw_iot_mes_string()           ← 序列化为 JSON（必须在上报前调用）
    ↓
hw_iot_report_properties()    ← 通过 MQTT 发布
    ↓
hw_iot_free_des()             ← 释放内存（必须调用，防止泄漏）
```

---

## 六、使用示例

### 基本用法

```c
#include "mqtt_hw_iot_message_up.h"

HW_IOT_DM_DES *des = hw_iot_malloc_des();
if (des) {
    hw_iot_set_mes_des(des, "BasicData", "luminance", 30);
    hw_iot_set_current_time(des, "BasicData");
    hw_iot_mes_string(des);
    hw_iot_report_properties(des);
    hw_iot_free_des(des);
}
```

### 多服务多属性

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

### 定时上报（FreeRTOS 任务）

```c
void property_report_task(void *pvParameters)
{
    while (1) {
        HW_IOT_DM_DES *des = hw_iot_malloc_des();
        if (des) {
            int luminance = read_luminance_sensor();
            int temperature = read_temperature_sensor();

            hw_iot_set_mes_des(des, "BasicData", "luminance", luminance);
            hw_iot_set_mes_des(des, "Temperature", "value", temperature);
            hw_iot_set_current_time(des, "BasicData");

            hw_iot_mes_string(des);
            hw_iot_report_properties(des);
            hw_iot_free_des(des);
        }
        vTaskDelay(pdMS_TO_TICKS(30000));  // 每 30 秒上报一次
    }
}
```

---

## 七、关键 Bug 修复

### cJSON_ArrayForEach 遍历陷阱

**问题代码：**
```c
cJSON *service = NULL;
cJSON_ArrayForEach(service, services)
{
    cJSON *sid = cJSON_GetObjectItem(service, "service_id");
    if (sid && sid->valuestring && strcmp(sid->valuestring, service_id) == 0)
    {
        break;
    }
    service = NULL;  // ← 错误！这会破坏循环，只能检查第一个元素
}
```

**原因：** `cJSON_ArrayForEach` 底层是 `for` 循环，循环体内 `service = NULL` 会导致下一次循环条件判断直接终止。

**修复：** 去掉循环内的 `service = NULL;`，循环正常结束时 `service` 自然为 `NULL`。

```c
cJSON *service = NULL;
cJSON_ArrayForEach(service, services)
{
    cJSON *sid = cJSON_GetObjectItem(service, "service_id");
    if (sid && sid->valuestring && strcmp(sid->valuestring, service_id) == 0)
    {
        break;
    }
    // 不需要 service = NULL;
}
```

---

## 八、文件结构

```
main/
├── mqtt_hw_iot_message_up.h     # 头文件：结构体定义和 API 声明
├── mqtt_hw_iot_message_up.c     # 源文件：函数实现
├── mqtt_hw_iot_message_up.md    # 使用说明文档
├── mqtt_hw_iot.h                # MQTT 配置（设备 ID、密钥等）
├── mqtt_hw_iot.c                # MQTT 客户端初始化
├── main.c                       # 主程序入口
└── CMakeLists.txt               # 构建配置
```

---

## 九、注意事项

### 内存管理
- `hw_iot_malloc_des()` 和 `hw_iot_free_des()` 必须**配对调用**
- 上报完成后立即释放，防止内存泄漏

### 前提条件
- WiFi 已连接
- MQTT 客户端已初始化（`mqtt_hw_iot_init()` 已调用）
- 设备已在华为云平台注册

### 属性值类型限制
- 当前实现仅支持**整数类型**属性值
- 如需浮点数或字符串，需添加新的 API

### 错误处理

| 场景 | 返回值 |
|------|--------|
| 内存分配失败 | `hw_iot_malloc_des()` 返回 `NULL` |
| 未序列化就上报 | `hw_iot_report_properties()` 返回 `-1` |
| MQTT 未初始化 | `hw_iot_report_properties()` 返回 `-1` |
| 发布失败 | `hw_iot_report_properties()` 返回 `-1` |

---

## 十、Git 提交记录

```
d8417d5 feat(iot): 添加华为云IoT物模型消息上传功能
```

涉及文件：
- `main/mqtt_hw_iot_message_up.c` — 新增
- `main/mqtt_hw_iot_message_up.h` — 新增
- `main/main.c` — 修改（引入头文件 + 测试调用）
- `main/CMakeLists.txt` — 修改（添加新源文件）
