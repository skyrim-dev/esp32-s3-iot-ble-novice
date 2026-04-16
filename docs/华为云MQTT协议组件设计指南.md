# 华为云 MQTT 协议组件设计指南

## 组件概述

`hw_iot_mqtt` 是一个封装华为云 IoT 平台 MQTT 协议通信的组件，提供 Topic 生成、JSON 编解码、MQTT 连接管理等功能。

## 设计架构

```
┌─────────────────────────────────────────────────────────────┐
│                      应用层 (main.c)                         │
│                                                             │
│   ┌─────────────────────────────────────────────────────┐   │
│   │              hw_iot_mqtt (组件)                     │   │
│   │                                                      │   │
│   │  ┌──────────────┐  ┌──────────────┐  ┌────────────┐  │   │
│   │  │ hw_iot_mqtt  │  │ hw_iot_mqtt │  │ hw_iot_mqtt│  │   │
│   │  │  _config     │  │   _topic     │  │   _json    │  │   │
│   │  │  (连接管理)  │  │  (Topic工具) │  │ (JSON工具) │  │   │
│   │  └──────────────┘  └──────────────┘  └────────────┘  │   │
│   │          │                 │                │         │   │
│   │          └─────────────────┴────────────────┘         │   │
│   │                         │                             │   │
│   │                         ▼                             │   │
│   │              ESP-IDF mqtt_client                      │   │
│   └─────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

## 模块职责

| 模块 | 文件 | 职责 |
|------|------|------|
| **配置模块** | `hw_iot_mqtt_config.c/h` | MQTT 连接初始化、事件回调、消息发布 |
| **Topic 模块** | `hw_iot_mqtt_topic.c/h` | 生成华为云规范的 Topic、解析命令请求 |
| **JSON 模块** | `hw_iot_mqtt_json.c/h` | JSON 结构体定义、序列化/反序列化 |

## 核心设计思想

### 1. 分层设计

```
┌─────────────────┐
│   应用层        │  调用 hw_iot_mqtt_report()
├─────────────────┤
│   Topic 工具    │  生成 $oc/devices/xxx/sys/properties/report
├─────────────────┤
│   JSON 工具     │  生成 {"services": [...]}
├─────────────────┤
│   MQTT 连接     │  封装 ESP-IDF mqtt_client
└─────────────────┘
```

**优点**：
- 各层职责单一
- 便于测试和替换
- 上层无需关心底层细节

### 2. 静态缓冲区设计

Topic 模块使用静态缓冲区避免动态内存分配：

```c
static char topic_str[128] = {0};

char *hw_iot_mqtt_topic_get(hw_iot_topic_type_t type, char *device_id, char *request_id)
{
    // 写入静态缓冲区
    snprintf(topic_str, sizeof(topic_str), "...", device_id);
    return topic_str;  // 返回静态缓冲区指针
}
```

**注意**：返回值在下次调用时会被覆盖，调用者需要立即使用或复制。

### 3. 枚举驱动设计

使用枚举定义支持的 Topic 类型：

```c
typedef enum {
    HW_IOT_TOPIC_PROPERTIES_REPORT = 0,  // 属性上报
    HW_IOT_TOPIC_COMMAND_RESPONSE,       // 命令响应
    HW_IOT_TOPIC_VERSION_REPORT,         // 版本上报
} hw_iot_topic_type_t;
```

**优点**：
- 类型安全
- 代码可读性好
- 便于扩展新 Topic

### 4. 结构体抽象

JSON 模块使用结构体封装数据：

```c
typedef struct {
    char *service_id;
    char *properties_id[10];
    double properties_value[10];
} hw_iot_mqtt_properties_json_t;
```

**优点**：
- 数据结构清晰
- 便于传参和赋值
- 内存布局明确

## 数据流

### 属性上报流程

```
应用层                    hw_iot_mqtt                  华为云
   │                          │                          │
   │  1. 填充结构体            │                          │
   │  ────────────────────────>│                          │
   │                          │  2. 生成 JSON             │
   │                          │  ────────────────────────>│
   │                          │                          │
   │                          │  3. 生成 Topic            │
   │                          │  ────────────────────────>│
   │                          │                          │
   │                          │  4. 发布 MQTT 消息        │
   │                          │  ────────────────────────>│
```

### 命令响应流程

```
华为云                    hw_iot_mqtt                  应用层
   │                          │                          │
   │  1. 下发命令             │                          │
   │  ────────────────────────>│  2. 回调通知             │
   │                          │  ────────────────────────>│
   │                          │                          │
   │  5. 接收响应             │  4. 发布响应             │
   │  <────────────────────────│  <───────────────────────│
```

## 使用示例

### 1. 初始化连接

```c
#include "hw_iot_mqtt_config.h"

void app_main(void)
{
    // WiFi 连接后初始化 MQTT
    wifi_init();
    xSemaphoreTake(wifi_connected_semaphore, portMAX_DELAY);
    
    // 连接华为云
    hw_iot_mqtt_init();
}
```

### 2. 上报属性

```c
// 填充数据结构
hw_iot_mqtt_properties_report_json_t data = {0};
data.json[0].service_id = "temperature";
data.json[0].properties_id[0] = "temp";
data.json[0].properties_value[0] = 25.5;
data.json[0].properties_id[1] = NULL;  // 结束标记

// 生成 JSON
char *json_str = hw_iot_mqtt_properties_report_json(&data);

// 生成 Topic
char *topic = hw_iot_mqtt_topic_get(HW_IOT_TOPIC_PROPERTIES_REPORT, HW_IOT_DEVICE_ID, NULL);

// 发布
hw_iot_mqtt_report(topic, json_str);

// 释放内存
free(json_str);
```

### 3. 接收命令

命令响应在 `mqtt_event_callback` 中自动处理：

```c
// 收到命令后自动：
// 1. 解析 request_id
// 2. 生成响应 JSON
// 3. 发布响应
```

## 扩展指南

### 添加新的 Topic 类型

1. 在 `hw_iot_mqtt_topic.h` 枚举中添加：
```c
typedef enum {
    // ... 现有类型 ...
    HW_IOT_TOPIC_NEW_TYPE,  // 新类型
} hw_iot_topic_type_t;
```

2. 在 `hw_iot_mqtt_topic.c` 中添加处理逻辑：
```c
case HW_IOT_TOPIC_NEW_TYPE:
    snprintf(topic_str, sizeof(topic_str), 
             "$oc/devices/%s/sys/new/path", device_id);
    break;
```

### 添加新的 JSON 格式

1. 在 `hw_iot_mqtt_json.h` 中定义结构体
2. 在 `hw_iot_mqtt_json.c` 中实现序列化函数

## 注意事项

| 事项 | 说明 |
|------|------|
| **内存管理** | `hw_iot_mqtt_topic_get()` 返回静态缓冲区，需立即使用 |
| **JSON 内存** | JSON 函数返回的字符串需调用 `free()` 释放 |
| **线程安全** | MQTT 回调在独立任务中执行，注意共享资源访问 |
| **错误处理** | 所有函数都有错误返回值，需检查处理 |

## 相关文件

```
components/protocol/hw_iot_mqtt/
├── CMakeLists.txt           # 组件构建配置
├── cert.pem                # CA 证书
├── hw_iot_mqtt_config.c/h  # 连接管理
├── hw_iot_mqtt_topic.c/h   # Topic 工具
└── hw_iot_mqtt_json.c/h    # JSON 工具
```
