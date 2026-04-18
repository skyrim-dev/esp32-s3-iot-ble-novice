# base-project

## 项目路径

- 项目根目录：`base-project/`
- 应用入口：`main/main.c`

## 项目简介

这是一个基于 **ESP-IDF 5.5.1** 的 **ESP32-S3** 示例工程，当前实现了以下核心能力：

- WiFi STA 联网
- SNTP 时间同步
- 华为云 IoT MQTT 连接
- 设备属性上报
- 华为云 OTA 固件升级

项目采用 `main + components` 的组织方式，`main/` 负责启动编排，`components/` 下按功能拆分为网络、协议、服务三层。

## 项目结构与分析范围

本 README 基于以下内容进行分析：

- 项目入口 `main/main.c` 的启动流程
- `components/` 目录下各组件的职责与依赖关系
- `components/network/Kconfig` 与 `components/protocol/Kconfig` 中的配置参数
- 根构建文件、组件 `CMakeLists.txt`、`sdkconfig` 与分区表配置

当前主要目录如下：

```text
base-project/
├── CMakeLists.txt
├── sdkconfig
├── custom_partitions_two_ota.csv
├── main/
│   ├── CMakeLists.txt
│   └── main.c
├── components/
│   ├── network/
│   │   ├── CMakeLists.txt
│   │   ├── Kconfig
│   │   └── wifi/
│   ├── protocol/
│   │   ├── CMakeLists.txt
│   │   ├── Kconfig
│   │   └── hw_iot_mqtt/
│   └── service/
│       ├── CMakeLists.txt
│       └── ota/
└── docs/
```

## 硬件要求

以下信息来自 `sdkconfig`：

- 芯片：`ESP32-S3` (`CONFIG_IDF_TARGET="esp32s3"`)
- Flash 大小：`16MB` (`CONFIG_ESPTOOLPY_FLASHSIZE="16MB"`)
- Flash 模式：`DIO` (`CONFIG_ESPTOOLPY_FLASHMODE="dio"`)
- Flash 频率：`80MHz` (`CONFIG_ESPTOOLPY_FLASHFREQ="80m"`)
- CPU 频率：`160MHz` (`CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ=160`)

建议硬件条件：

- 一块 ESP32-S3 开发板
- 可用的 WiFi 网络环境
- USB 数据线
- 可正常访问华为云 IoT 平台与 OTA 下载地址的网络环境

## 软件环境

- ESP-IDF 版本：`5.5.1`
  - 根构建版本来源：`CMakeLists.txt`
  - `sdkconfig` 中的初始化版本：`CONFIG_IDF_INIT_VERSION="5.5.1"`
- 项目版本：`1.0.0`
  - 来源：`CMakeLists.txt`

## 启动流程（main/main.c）

项目入口位于：

- `main/main.c`

`app_main()` 的实际启动顺序如下：

1. 打印启动日志 `Hello world!`
2. 初始化 LED（GPIO1）
3. 读取并打印当前应用版本 `get_app_version()`
4. 调用 `wifi_init()` 初始化 WiFi
5. 阻塞等待 `wifi_connected_semaphore`，直到获取 IP
6. 调用 `time_sync_init()` 执行 SNTP 时间同步
7. 调用 `hw_iot_mqtt_init()` 初始化华为云 MQTT 客户端
8. 调用 `hw_iot_mqtt_properties_publish()` 上报设备属性

这说明当前项目是一个典型的“联网后接云，再触发业务”的启动模型。

## components 组件依赖关系

### 1. main 组件

- 路径：`main/CMakeLists.txt`
- 依赖：`protocol`、`network`、`esp_driver_gpio`

职责：

- 启动顺序编排
- WiFi 连接等待
- 时间同步触发
- MQTT 初始化与首次属性上报

### 2. network 组件

- 路径：`components/network/`
- 构建文件：`components/network/CMakeLists.txt`
- 依赖：`esp_netif`、`esp_wifi`、`nvs_flash`、`lwip`

职责：

- WiFi STA 初始化与重连
- IP 获取通知
- SNTP 时间同步

核心实现：

- `wifi/wifi.c`

### 3. protocol 组件

- 路径：`components/protocol/`
- 构建文件：`components/protocol/CMakeLists.txt`
- 依赖：`esp_netif`、`mqtt`、`esp_timer`、`nvs_flash`、`json`、`esp_wifi`、`esp_driver_gpio`、`service`
- 嵌入资源：`hw_iot_mqtt/cert.pem`

职责：

- 华为云 MQTT 客户端初始化
- topic 组装与消息分类
- JSON 报文生成
- 属性上报、版本上报、OTA 状态上报
- OTA 下行消息解析与升级请求提交

核心实现：

- `hw_iot_mqtt/hw_iot_mqtt_client.c`
- `hw_iot_mqtt/hw_iot_mqtt_publish.c`
- `hw_iot_mqtt/hw_iot_mqtt_subscribe.c`
- `hw_iot_mqtt/hw_iot_mqtt_topic.c`
- `hw_iot_mqtt/hw_iot_mqtt_json.c`

### 4. service 组件

- 路径：`components/service/`
- 构建文件：`components/service/CMakeLists.txt`
- 依赖：`esp_https_ota`、`mbedtls`、`app_update`、`protocol`、`nvs_flash`

职责：

- OTA 升级任务管理
- OTA 下载与烧录
- OTA 结果回调与升级状态持久化
- 当前运行版本读取

核心实现：

- `ota/hw_iot_ota.c`
- `ota/ota_manager.c`

### 5. 当前组件关系总结

```text
main
 ├─ network
 ├─ protocol
 └─ esp_driver_gpio

protocol
 └─ service

service
 └─ protocol
```

说明：

- 当前 `protocol` 与 `service` 之间存在耦合
- `protocol` 负责接收华为云 OTA 指令
- `service` 负责真正执行 OTA 下载与升级

## Kconfig 配置参数

### 1. WiFi 配置

定义文件：

- `components/network/Kconfig`

参数如下：

| 参数名 | 默认值 | 说明 |
| --- | --- | --- |
| `CONFIG_WIFI_SSID` | `MyWiFi` | WiFi 名称 |
| `CONFIG_WIFI_PASSWORD` | `password` | WiFi 密码 |
| `CONFIG_WIFI_CONNECT_TIMEOUT_MS` | `30000` | WiFi 连接超时（ms） |
| `CONFIG_WIFI_RECONNECT_INTERVAL_MS` | `5000` | WiFi 重连间隔（ms） |

这些配置最终会在 `wifi.h` 中映射为：

- `WIFI_SSID`
- `WIFI_PWD`

### 2. 华为云 IoT 配置

定义文件：

- `components/protocol/Kconfig`

参数如下：

| 参数名 | 默认值 | 说明 |
| --- | --- | --- |
| `CONFIG_HW_IOT_HOSTNAME` | `f1614c4895.iotda-device.cn-south-4.myhuaweicloud.com` | MQTT Broker 地址 |
| `CONFIG_HW_IOT_PORT` | `8883` | MQTT TLS 端口 |
| `CONFIG_HW_IOT_PRODUCTKEY` | `69cc7e9c6b6c4d5f8d58bd94` | 产品标识 |
| `CONFIG_HW_IOT_DEVICE_ID` | `69cc7e9c6b6c4d5f8d58bd94_3c-84-27-c0-2e-6c` | 设备 ID |
| `CONFIG_HW_IOT_CLIENT_ID` | `69cc7e9c6b6c4d5f8d58bd94_3c-84-27-c0-2e-6c_0_0_2026040109` | MQTT Client ID |
| `CONFIG_HW_IOT_USERNAME` | `69cc7e9c6b6c4d5f8d58bd94_3c-84-27-c0-2e-6c` | MQTT 用户名 |
| `CONFIG_HW_IOT_PASSWORD` | 空字符串 | MQTT 密码 / Device Secret |

### 3. 分区表配置

来自：

- `sdkconfig`
- `custom_partitions_two_ota.csv`

当前分区表文件：

- `CONFIG_PARTITION_TABLE_FILENAME="custom_partitions_two_ota.csv"`

实际分区包括：

- `nvs`
- `otadata`
- `phy_init`
- `factory`
- `ota_0`
- `ota_1`

说明：

- 当前工程已经具备双 OTA 分区能力
- OTA 升级会写入 `ota_0` / `ota_1` 之一，而不是覆盖 factory 分区

## 配置说明

### WiFi 配置

修改方式：

- 通过 `idf.py menuconfig`
- 进入 `WiFi Configuration`

建议至少配置：

- `WiFi SSID`
- `WiFi Password`

### 华为云 IoT 配置

修改方式：

- 通过 `idf.py menuconfig`
- 进入 `Huawei Cloud IoT Configuration`

建议至少配置：

- MQTT Broker Hostname
- MQTT Broker Port
- Device ID
- Client ID
- Username
- Password

### 证书配置

协议层证书文件位于：

- `components/protocol/hw_iot_mqtt/cert.pem`

证书通过 `components/protocol/CMakeLists.txt` 中的：

```cmake
EMBED_FILES "hw_iot_mqtt/cert.pem"
```

嵌入到固件。

若修改证书，建议重新执行：

```bash
idf.py fullclean && idf.py build
```

## 构建和烧录命令

以下命令均在项目根目录执行：

- `base-project/`

### 1. 编译项目

```bash
idf.py build
```

### 2. 烧录固件

```bash
idf.py flash
```

或指定串口：

```bash
idf.py -p PORT flash
```

### 3. 配置参数

```bash
idf.py menuconfig
```

### 4. Kconfig 变更后重新配置

```bash
idf.py reconfigure
```

### 5. 清理后完整重编译

```bash
idf.py fullclean && idf.py build
```

适用场景：

- 证书文件变更
- Kconfig / sdkconfig 相关缓存问题
- OTA 嵌入资源更新后需要强制刷新构建结果

## 功能说明

当前项目已实现以下功能：

### 1. 设备启动与版本打印

- 启动后初始化 LED
- 读取当前运行分区的应用版本

### 2. WiFi 联网

- STA 模式连接指定 WiFi
- 获取 IP 后再继续后续流程
- 断线后自动重连

### 3. 时间同步

- 使用 SNTP 同步系统时间
- 当前服务器配置为 `ntp.aliyun.com`
- 用于保证 MQTT 上报中的 `event_time` 正确

### 4. 华为云 MQTT 接入

- 通过 TLS 连接华为云 IoT 平台
- 支持属性上报
- 支持 OTA 版本查询与升级消息处理

### 5. OTA 升级

- 响应华为云 OTA `version_query`
- 上报当前软固件版本
- 接收 `firmware_upgrade` 指令
- 使用 `esp_https_ota()` 下载并烧录新固件
- 上报 OTA 升级状态
- 重启进入新分区运行新版本

## 说明

- 本项目 README 基于当前仓库实际代码与配置生成，不基于假设。
- 如果后续你修改了 Kconfig、分区表、证书文件或 OTA 逻辑，README 也应该同步更新。
