# ESP32-S3 华为云 IoT 项目

基于 ESP32-S3 的华为云物联网平台连接示例，支持 MQTT 协议通信。

## 功能特性

- WiFi 连接（可配置 SSID/密码/超时时间）
- MQTT over TLS 连接华为云 IoT 平台
- 设备属性上报
- 云端命令接收与响应
- 证书独立管理

## 硬件要求

- ESP32-S3 开发板
- USB 数据线
- 可选：传感器、外设等

## 软件依赖

- ESP-IDF v5.x
- Python 3.x

## 快速开始

### 1. 配置项目

```bash
# 配置 WiFi (SSID、密码、超时时间)
idf.py menuconfig
# 进入: Component config → WiFi Configuration

# 配置华为云连接参数
# 编辑: components/protocol/hw_iot_mqtt/hw_iot_mqtt_config.h
```

### 2. 编译烧录

```bash
# 编译
idf.py build

# 烧录
idf.py flash

# 监视串口输出
idf.py monitor
```

### 3. 查看日志

连接成功后，日志输出：
```
I (xxx) wifi: WiFi STA start
I (xxx) wifi: WiFi STA connected
I (xxx) wifi: WiFi STA got IP
I (xxx) mqtt_hw_iot: Connected to broker
```

## 项目结构

```
├── main/                       # 应用层
│   └── main.c                 # 主程序入口
│
├── components/
│   ├── network/               # 驱动层
│   │   └── wifi/             # WiFi 连接驱动
│   │
│   └── protocol/             # 协议层
│       └── hw_iot_mqtt/     # 华为云 MQTT 协议
│           ├── cert.pem      # CA 证书
│           └── ...
│
└── docs/                     # 文档
```

## 文档目录

| 文档 | 说明 |
|------|------|
| [项目层级结构.md](./项目层级结构.md) | 代码分层架构说明 |
| [Kconfig 配置指南.md](./Kconfig配置指南.md) | 如何配置 WiFi 参数 |
| [PEM 证书配置指南.md](./PEM证书配置指南.md) | 证书管理说明 |
| [cJSON 使用指南.md](./cJSON使用指南.md) | JSON 数据处理 |

## 云端配置

### 华为云 IoT 平台

1. 注册华为云账号
2. 创建 IoT 设备
3. 获取以下信息并填入 `hw_iot_mqtt_config.h`：
   - 服务器地址 (HOSTNAME)
   - 设备 ID (DEVICE_ID)
   - 设备密钥 (PASSWORD)

## 常见问题

### 编译报错

```bash
# 清理后重新编译
idf.py fullclean
idf.py build
```

### WiFi 连接失败

1. 检查 SSID 和密码是否正确
2. 检查路由器是否支持 2.4GHz（ESP32 不支持 5GHz WiFi）
3. 尝试增加 WiFi 超时时间

### MQTT 连接失败

1. 检查华为云设备配置是否正确
2. 检查证书是否有效
3. 查看串口日志定位问题

## License

MIT License
