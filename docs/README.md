# ESP32-S3 华为云 IoT 项目

基于 ESP32-S3 的华为云物联网平台连接示例，当前仓库已实现 MQTT over TLS 通信，以及由华为云 IoT 下发升级任务触发的 HTTPS OTA。

## 功能特性

- WiFi 连接（可配置 SSID/密码/超时时间）
- MQTT over TLS 连接华为云 IoT 平台
- 设备属性上报
- 云端命令接收与响应
- OTA 版本上报（`version_query -> version_report`）
- 华为云 `firmware_upgrade` 事件触发的 HTTPS OTA
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
# 打开 menuconfig
idf.py menuconfig

# 配置 WiFi
# 路径: Component config -> WiFi Configuration

# 配置华为云 MQTT 连接参数
# 路径: Component config -> Huawei Cloud IoT Configuration
```

当前项目中的华为云连接参数不再通过头文件手工编辑，而是来自 `components/protocol/Kconfig` 对应的 `CONFIG_HW_IOT_*` 配置项，实际编译结果写入 `sdkconfig`。

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

连接成功后，典型日志输出如下：

```
I (xxx) wifi: WiFi STA start
I (xxx) wifi: WiFi STA connected
I (xxx) wifi: WiFi STA got IP
I (xxx) mqtt_event_callback: Connected to broker
```

如果需要验证 OTA，请继续参考《[华为云IoT OTA指南](./华为云IoT%20OTA指南.md)》。

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
│   └── service/              # 服务层
│       └── ota/              # 华为云 HTTPS OTA 实现
│
└── docs/                     # 文档
```

## 文档目录

| 文档 | 说明 |
|------|------|
| [项目层级结构.md](./项目层级结构.md) | 代码分层架构说明 |
| [Kconfig 配置指南.md](./Kconfig%20配置指南.md) | Kconfig/menuconfig 配置说明 |
| [PEM 证书配置指南.md](./PEM%20证书配置指南.md) | MQTT 证书管理说明 |
| [cJSON 使用指南.md](./cJSON%20使用指南.md) | JSON 数据处理 |
| [华为云IoT OTA指南.md](./华为云IoT%20OTA指南.md) | 基于当前仓库实现的 OTA 发布、升级与验证指南 |

## 云端配置

### 华为云 IoT 平台

1. 注册华为云账号
2. 创建 IoT 设备
3. 在 `idf.py menuconfig` 中填写以下配置项：
   - `HW_IOT_HOSTNAME`
   - `HW_IOT_PORT`
   - `HW_IOT_DEVICE_ID`
   - `HW_IOT_CLIENT_ID`
   - `HW_IOT_USERNAME`
   - `HW_IOT_PASSWORD`
4. 如果要执行 OTA，维护者还需要在华为云 IoT 平台上传 `build/base-project.bin` 并创建升级任务。当前仓库未提供云侧自动化脚本，详情见《[华为云IoT OTA指南](./华为云IoT%20OTA指南.md)》。

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

### OTA 升级问题

1. 确认平台是否已下发 `version_query` 与 `firmware_upgrade`
2. 确认设备日志中是否出现 `OTA URL`、`OTA access_token`、`esp_https_ota: Starting OTA...`
3. 确认当前工程使用的是 `custom_partitions_two_ota.csv`
4. 详细排查请参考《[华为云IoT OTA指南](./华为云IoT%20OTA指南.md)》

## License

MIT License
