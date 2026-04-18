# 华为云IoT OTA指南

## 1. 文档目的与适用范围

本文面向当前仓库维护者，说明本项目中**已经真实落地**的华为云 IoT OTA 相关实现、配置、构建产物、平台人工操作和验证方式。

本文只以当前仓库中的代码、配置和构建产物为依据，主要覆盖以下范围：

- 设备：ESP32-S3
- 构建系统：ESP-IDF 5.5.1
- 云侧接入：华为云 IoT MQTT over TLS
- OTA 方式：华为云下发 `firmware_upgrade` 事件，设备端使用 `esp_https_ota()` 进行 HTTPS OTA

本文**不是**华为云 OTA 通用教程。仓库中未实现的能力会明确标注为“当前项目未实现该能力”或“当前仓库未发现对应实现”。

## 2. 当前项目 OTA 实现概览

当前项目已经实现了一条可运行的 OTA 主链路：设备上电后连接 Wi-Fi 和华为云 MQTT，收到 `$ota` 的 `version_query` 时上报当前版本，收到 `$ota` 的 `firmware_upgrade` 时从下行 JSON 中提取 `url` 和 `access_token`，再通过 `esp_https_ota()` 下载并写入 OTA 分区，成功后调用 `esp_restart()` 重启到新固件。

当前项目中的角色关系如下：

- **设备端**：`main/main.c` 启动 Wi-Fi 与 MQTT，`components/protocol/hw_iot_mqtt/` 处理华为云下行消息，`components/service/ota/hw_iot_ota.c` 执行 OTA 下载与重启。
- **固件产物**：构建后生成 `build/base-project.bin`，该文件是当前仓库可直接用于 OTA 上传的固件包。
- **华为云 IoT 平台**：负责下发 `version_query` 和 `firmware_upgrade` 事件；当前仓库未发现自动化调用华为云 OTA 管理 API 的脚本，平台升级包上传与任务创建依赖人工在控制台操作。

本项目当前 OTA 主流程如下：

1. 设备启动，输出当前应用版本，连接 Wi-Fi。
2. Wi-Fi 获取 IP 后初始化华为云 MQTT。
3. 平台下发 `version_query`，设备通过 MQTT 上报 `sw_version` 和 `fw_version`。
4. 平台下发 `firmware_upgrade`，设备解析 `url` 与 `access_token`。
5. 设备创建 OTA 任务，通过 `Authorization: Bearer <token>` 请求头调用 `esp_https_ota()`。
6. `esp_https_ota()` 将新镜像写入被动 OTA 分区。
7. OTA 成功后设备调用 `esp_restart()` 重启，启动到新的 OTA 分区。

## 3. 代码与配置位置总览

| 路径 | 作用 | 是否关键 |
|---|---|---|
| `CMakeLists.txt` | 定义项目名 `base-project` 和 `PROJECT_VER` | 是 |
| `main/main.c` | 应用入口；启动 Wi-Fi、MQTT，打印当前版本 | 是 |
| `sdkconfig` | 当前实际编译配置；包含华为云参数、分区表、TLS、Flash 大小 | 是 |
| `custom_partitions_two_ota.csv` | 当前项目 OTA 分区表 | 是 |
| `components/protocol/CMakeLists.txt` | 注册华为云 MQTT 组件并嵌入 `cert.pem` | 是 |
| `components/service/CMakeLists.txt` | 注册 OTA 组件并依赖 `esp_https_ota` | 是 |
| `components/protocol/Kconfig` | 华为云 MQTT Host、Port、Device ID、Password 等 menuconfig 配置项 | 是 |
| `components/network/Kconfig` | Wi-Fi SSID/密码配置项 | 否 |
| `components/network/wifi/wifi.c` | Wi-Fi 初始化、事件处理、联网后释放信号量 | 是 |
| `components/protocol/hw_iot_mqtt/hw_iot_mqtt_client.c` | MQTT 事件入口；处理 `version_query` 和 `firmware_upgrade` | 是 |
| `components/protocol/hw_iot_mqtt/hw_iot_mqtt_subscribe.c` | 识别华为云下行消息类型 | 是 |
| `components/protocol/hw_iot_mqtt/hw_iot_mqtt_publish.c` | 版本上报、属性上报、命令响应 | 是 |
| `components/protocol/hw_iot_mqtt/hw_iot_mqtt_json.c` | 构造 `version_report`、属性上报、命令响应 JSON | 是 |
| `components/protocol/hw_iot_mqtt/hw_iot_mqtt_topic.c` | 生成华为云 MQTT Topic | 是 |
| `components/service/ota/hw_iot_ota.c` | 保存 OTA URL/Token，配置 HTTPS OTA，执行下载、写分区、重启 | 是 |
| `components/service/ota/hw_iot_ota.h` | OTA 对外接口声明 | 是 |
| `build/base-project.bin` | 当前构建产物，可作为 OTA 固件包上传 | 是 |
| `build/ota_data_initial.bin` | 构建生成的 OTA 数据镜像 | 否 |
| `docs/README.md` | 项目已有编译/烧录/monitor 基础命令说明 | 否 |

当前仓库未发现以下文件：

- `sdkconfig.defaults`
- 专用 OTA 打包脚本
- 华为云 OTA 任务创建脚本
- 专用烧录脚本（仅发现标准 `idf.py` 工作流）

## 4. OTA 前置条件

### 4.1 平台侧条件

当前项目依赖华为云 IoT 平台下发 OTA 事件，平台侧至少需要：

- 设备已在华为云 IoT 平台创建，且仓库中的 `CONFIG_HW_IOT_*` 参数与之匹配，见 `sdkconfig` 和 `components/protocol/Kconfig`。
- 平台能向设备下发 `$ota` 服务的 `version_query` 与 `firmware_upgrade` 事件；代码依据见 `components/protocol/hw_iot_mqtt/hw_iot_mqtt_subscribe.c`。
- 维护者需要在华为云控制台上传固件包并创建升级任务。当前仓库未发现自动调用华为云 OTA API 的脚本。

### 4.2 设备侧条件

- 设备必须能成功连接 Wi-Fi，见 `components/network/wifi/wifi.c`。
- 设备必须能成功连接华为云 MQTT over TLS，见 `components/protocol/hw_iot_mqtt/hw_iot_mqtt_client.c`。
- 设备必须已经烧录为带 OTA 分区表的镜像。

### 4.3 编译/烧录前提

- 当前项目使用 ESP-IDF 5.5.1，见 `sdkconfig` 第 3 行。
- 当前仓库编译命令沿用标准 IDF 工作流：`idf.py build`、`idf.py flash`、`idf.py monitor`，见 `docs/README.md`。
- 当前项目名是 `base-project`，固件输出文件名随之为 `build/base-project.bin`，依据：`CMakeLists.txt` 和 `build/` 目录。

### 4.4 分区表要求

当前项目已启用自定义分区表：

- `CONFIG_PARTITION_TABLE_CUSTOM=y`
- `CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="custom_partitions_two_ota.csv"`

当前分区表文件为 `custom_partitions_two_ota.csv`，包含：

- `nvs`
- `otadata`
- `phy_init`
- `factory`
- `ota_0`
- `ota_1`

其中三个应用分区 `factory`、`ota_0`、`ota_1` 大小均为 `1500k`。当前项目 OTA 依赖 `otadata + ota_0 + ota_1`，因此该分区表是 OTA 能够运行的前提。

### 4.5 网络要求

- 设备必须能访问华为云 MQTT 地址 `CONFIG_HW_IOT_HOSTNAME:CONFIG_HW_IOT_PORT`。
- 设备必须能访问 `firmware_upgrade` 事件中下发的 HTTPS 下载地址。

### 4.6 鉴权/证书要求

#### MQTT 连接

- MQTT 连接使用 `components/protocol/hw_iot_mqtt/cert.pem`，由 `components/protocol/CMakeLists.txt` 的 `EMBED_FILES` 嵌入。
- MQTT 客户端通过 `mqtt_cfg.broker.verification.certificate = _binary_cert_pem_start` 使用该证书。

#### OTA HTTPS 下载

- OTA 下载使用 `esp_crt_bundle_attach` 进行证书链校验，见 `components/service/ota/hw_iot_ota.c`。
- 当前实现额外设置了 `.skip_cert_common_name_check = true`，即当前项目跳过了服务端证书通用名称检查。代码注释明确说明“用于测试环境”。当前项目已这样实现，但这不是生产环境推荐配置。
- `sdkconfig` 中未启用：
  - `CONFIG_ESP_TLS_INSECURE`
  - `CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY`

## 5. 本项目 OTA 实现机制详解

### 5.1 升级任务如何触发

当前项目通过华为云下行 MQTT 消息触发 OTA，不是本地定时轮询，不是 HTTP 轮询，不是脚本触发。

入口文件：`components/protocol/hw_iot_mqtt/hw_iot_mqtt_client.c`

关键流程：

1. `mqtt_event_callback()` 接收 `MQTT_EVENT_DATA`。
2. `hw_iot_mqtt_subscribe_type(receive_data)` 判断消息类型。
3. 若识别为 OTA，则进入 `hw_iot_mqtt_subscribe_ack()`。

消息类型判断逻辑在 `components/protocol/hw_iot_mqtt/hw_iot_mqtt_subscribe.c`：

- 包含 `$ota` 且 Topic 含 `sys/events/down`：视为 OTA 下行消息。
- 包含 `version_query`：返回 `HW_IOT_MQTT_VERSION_QUERY_SUBSCRIBE`
- 包含 `firmware_upgrade`：返回 `HW_IOT_MQTT_SFW_UPGRADE_SUBSCRIBE`

### 5.2 设备如何感知新版本

当前设备端**不做本地版本比较**，也不主动检查云端版本。

当前实现方式是：

- 平台先下发 `version_query`
- 设备调用 `hw_iot_mqtt_ota_version_publish()` 上报当前版本
- 平台再决定是否下发 `firmware_upgrade`

也就是说，版本比较由平台流程驱动。当前仓库未发现设备端自定义版本比较逻辑。

### 5.3 版本号如何定义与比较

#### 版本定义

当前仓库的版本定义来自项目级 `PROJECT_VER`：

- 文件：`CMakeLists.txt`
- 当前默认值：`set(PROJECT_VER "1.0.0")`

设备运行时的版本读取逻辑在 `get_app_version()`：

- 文件：`components/protocol/hw_iot_mqtt/hw_iot_mqtt_client.c`
- 函数：`get_app_version()`
- 实现：通过 `esp_ota_get_running_partition()` 和 `esp_ota_get_partition_description()` 读取当前运行分区中的 `esp_app_desc_t.version`

#### 版本比较

当前仓库未发现设备端对版本号进行字符串比较、语义化版本比较或数值比较的实现。

### 5.4 固件下载地址如何获取

`firmware_upgrade` 事件的 JSON 由 `hw_iot_mqtt_subscribe_ack()` 解析：

- 文件：`components/protocol/hw_iot_mqtt/hw_iot_mqtt_client.c`

当前代码实际提取的字段只有：

- `services[0].paras.url`
- `services[0].paras.access_token`

当前仓库未发现对以下字段的实际使用：

- `version`
- `file_size`
- `file_name`
- `expires`
- `sign`

### 5.5 下载流程如何实现

下载执行模块：`components/service/ota/hw_iot_ota.c`

执行流程如下：

1. `hw_iot_ota_init(url, access_token, cb)` 保存 URL、Token 和回调。
2. `hw_iot_ota_start()` 防止重复启动 OTA 任务，然后创建 `hw_iot_ota_task`。
3. `hw_iot_ota_task()` 构造 `esp_http_client_config_t`：
   - `.url = hw_iot_url`
   - `.timeout_ms = 100000`
   - `.crt_bundle_attach = esp_crt_bundle_attach`
   - `.keep_alive_enable = true`
   - `.skip_cert_common_name_check = true`
4. `hw_iot_http_client_init_cb()` 给 HTTP 客户端设置：
   - `Authorization: Bearer <access_token>`
   - `Content-Type: application/json`
5. `esp_https_ota(&ota_config)` 阻塞执行下载与写入。

当前项目是**华为云 MQTT 下发 URL + access_token，设备端用 ESP-IDF `esp_https_ota()` 直接下载**。

### 5.6 实际存在的校验机制

当前仓库中能够确认的校验机制只有：

- OTA 下载使用 TLS 证书链校验（`esp_crt_bundle_attach`）
- `esp_https_ota()` 的标准镜像处理流程

当前仓库未发现以下自定义实现：

- 使用 `sign` 字段做签名校验
- 使用 `file_size` 做下载完成后的文件大小校验
- CRC 校验
- SHA256 校验
- MD5 校验
- 自定义镜像 hash 校验

因此，本项目文档不能把这些能力写成“已实现”。

### 5.7 OTA 写入分区流程

当前项目没有手写 `esp_ota_begin()` / `esp_ota_write()` / `esp_ota_end()` 流程，而是直接调用 `esp_https_ota()`。

因此：

- OTA 写入由 ESP-IDF `esp_https_ota()` 内部完成
- 当前仓库未发现自定义选择 OTA 分区的实现
- 当前仓库未发现显式调用 `esp_ota_set_boot_partition()` 的代码

当前仓库实际依赖的是 ESP-IDF 原生 OTA 机制和当前分区表中的 `ota_0 / ota_1`。

### 5.8 启动分区切换逻辑

当前仓库未发现自定义启动分区切换逻辑。

设备端成功 OTA 后直接执行：

- `esp_restart()`

启动分区切换由 `esp_https_ota()` 与 ESP-IDF OTA 机制处理，不是本项目自定义实现。

### 5.9 首次启动确认机制

当前仓库未发现以下实现：

- `esp_ota_mark_app_valid_cancel_rollback()`
- `esp_ota_mark_app_invalid_rollback_and_reboot()`
- 首次启动自检确认流程

因此，当前项目未实现“首次启动确认”机制。

### 5.10 升级失败处理与回滚逻辑

当前代码在 `esp_https_ota()` 失败时：

- 记录错误日志
- 如果设置了回调，则回调 `ESP_FAIL`
- 将 `is_current_ota_task_running` 置为 `false`
- 删除 OTA 任务

当前仓库未发现自动回滚逻辑。

配置层面也未启用回滚：

- `# CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE is not set`
- `# CONFIG_APP_ROLLBACK_ENABLE is not set`

因此，当前项目文档不能写“已支持失败自动回滚”。

### 5.11 升级状态/结果如何上报

当前项目**只实现了版本上报**，未实现升级状态上报链路。

已实现：

- `version_query` 响应时调用 `hw_iot_mqtt_ota_version_publish()`
- 发布 Topic：`$oc/devices/{device_id}/sys/events/up`
- 负载由 `hw_iot_mqtt_ota_version_report_json()` 生成，固定 `service_id="$ota"`、`event_type="version_report"`

未实现：

- OTA 进度上报
- OTA 成功/失败状态上报
- 使用 `hw_iot_ota_callback()` 发布升级结果

虽然 `components/protocol/hw_iot_mqtt/hw_iot_mqtt_json.h` 声明了 `hw_iot_mqtt_ota_upgrade_status_json_t` 和 `hw_iot_mqtt_upgrade_status_report_json()`，但当前仓库未发现对应实现与调用。

## 6. 一次完整 OTA 发布流程（基于当前项目）

以下流程只写当前仓库中真实存在的步骤与人工操作。

### 6.1 修改固件版本

发布新 OTA 包前，先修改顶层版本号：

文件：`CMakeLists.txt`

```cmake
set(PROJECT_VER "1.0.1")
```

当前项目运行时版本直接来自该字段，因此每次发布新 OTA 包前都应同步更新 `PROJECT_VER`，否则设备上报给华为云的平台版本不会变化。

### 6.2 编译固件

在仓库根目录执行：

```bash
idf.py build
```

当前仓库中实际可见的关键产物位于 `build/` 目录：

- `build/base-project.bin`
- `build/base-project.elf`
- `build/partition_table/partition-table.bin`
- `build/ota_data_initial.bin`

其中用于 OTA 上传的固件包是：

```text
build/base-project.bin
```

### 6.3 首次烧录前提（设备端）

如果设备还没有烧录当前工程，需要先烧录完整工程，而不是只上传 OTA 包：

```bash
idf.py flash
idf.py monitor
```

原因：当前项目依赖自定义 OTA 分区表 `custom_partitions_two_ota.csv`，设备必须先运行在带 OTA 分区表的固件上，后续 OTA 才能切到 `ota_0/ota_1`。

### 6.4 华为云平台准备 OTA 包

当前仓库未发现上传升级包或创建升级任务的脚本，因此该步骤依赖华为云控制台人工操作。

维护者需要在华为云 IoT 平台完成：

1. 上传 `build/base-project.bin`
2. 将平台中的目标版本设置为与 `PROJECT_VER` 一致
3. 创建升级任务并下发到目标设备

本文只确认“平台需要下发 `version_query` 和 `firmware_upgrade`”，当前仓库未发现更细粒度的云侧自动化工具。

### 6.5 设备执行升级

设备在线后，按当前代码实际行为，执行顺序如下：

1. 平台下发 `version_query`
2. 设备上报 `version_report`
3. 平台下发 `firmware_upgrade`
4. 设备解析 `url` 和 `access_token`
5. 设备通过 `esp_https_ota()` 下载并写入被动 OTA 分区
6. 成功后设备 `esp_restart()`

### 6.6 如何验证升级成功

建议使用：

```bash
idf.py monitor
```

在当前项目中，至少应确认以下三类日志：

1. **收到 OTA 下行消息**
   - `Firmware upgrade subscribe ack`
   - `OTA URL: ...`

2. **开始 OTA 写入**
   - `hw_iot_ota_task: OTA task started`
   - `esp_https_ota: Starting OTA...`
   - `esp_https_ota: Writing to <ota_x> partition ...`

3. **重启后进入新固件**
   - Boot log 显示从新的 OTA 分区启动
   - `main: app_version: <新版本>`

如果新固件能启动并打印新的 `app_version`，则设备侧 OTA 已完成。

## 7. 关键调用链说明

### 7.1 启动联网与 MQTT 初始化

1. `app_main()`  
   文件：`main/main.c`
2. `wifi_init()`  
   文件：`components/network/wifi/wifi.c`
3. `xSemaphoreTake(wifi_connected_semaphore, portMAX_DELAY)` 等待网络连通  
   文件：`main/main.c`
4. `hw_iot_mqtt_init()` 初始化 MQTTS 连接  
   文件：`components/protocol/hw_iot_mqtt/hw_iot_mqtt_client.c`

### 7.2 OTA 版本查询链路

1. `mqtt_event_callback()` 收到 `MQTT_EVENT_DATA`  
   文件：`components/protocol/hw_iot_mqtt/hw_iot_mqtt_client.c`
2. `hw_iot_mqtt_subscribe_type(receive_data)` 判断为 `HW_IOT_MQTT_VERSION_QUERY_SUBSCRIBE`  
   文件：`components/protocol/hw_iot_mqtt/hw_iot_mqtt_subscribe.c`
3. `hw_iot_mqtt_subscribe_ack()` 调用 `hw_iot_mqtt_ota_version_publish()`  
   文件：`components/protocol/hw_iot_mqtt/hw_iot_mqtt_client.c`
4. `hw_iot_mqtt_ota_version_report_json()` 生成 `version_report` JSON  
   文件：`components/protocol/hw_iot_mqtt/hw_iot_mqtt_json.c`
5. `hw_iot_mqtt_publish()` 发布到 `$oc/devices/{device_id}/sys/events/up`  
   文件：`components/protocol/hw_iot_mqtt/hw_iot_mqtt_publish.c`、`hw_iot_mqtt_topic.c`

### 7.3 固件升级链路

1. `mqtt_event_callback()` 收到 `MQTT_EVENT_DATA`  
   文件：`components/protocol/hw_iot_mqtt/hw_iot_mqtt_client.c`
2. `hw_iot_mqtt_subscribe_type(receive_data)` 判断为 `HW_IOT_MQTT_SFW_UPGRADE_SUBSCRIBE`  
   文件：`components/protocol/hw_iot_mqtt/hw_iot_mqtt_subscribe.c`
3. `hw_iot_mqtt_subscribe_ack()` 解析 `services[0].paras.url` 和 `services[0].paras.access_token`  
   文件：`components/protocol/hw_iot_mqtt/hw_iot_mqtt_client.c`
4. `hw_iot_ota_init(url, access_token, hw_iot_ota_callback)` 保存 OTA 参数  
   文件：`components/service/ota/hw_iot_ota.c`
5. `hw_iot_ota_start()` 创建 `hw_iot_ota_task`  
   文件：`components/service/ota/hw_iot_ota.c`
6. `hw_iot_http_client_init_cb()` 设置 HTTP Header  
   文件：`components/service/ota/hw_iot_ota.c`
7. `esp_https_ota(&ota_config)` 下载并写分区  
   文件：`components/service/ota/hw_iot_ota.c`
8. OTA 成功后 `esp_restart()`  
   文件：`components/service/ota/hw_iot_ota.c`

## 8. 常见问题与排查

### 8.1 为什么设备收不到 OTA 任务

排查顺序：

1. 确认设备是否已经联网并输出 `WiFi connected`
2. 确认 MQTT 是否已连接并输出 `Connected to broker`
3. 检查 `sdkconfig` 中 `CONFIG_HW_IOT_HOSTNAME`、`CONFIG_HW_IOT_PORT`、`CONFIG_HW_IOT_DEVICE_ID`、`CONFIG_HW_IOT_PASSWORD` 是否与平台一致
4. 查看 `mqtt_event_callback()` 是否收到 `MQTT_EVENT_DATA`

当前仓库未发现显式调用 `esp_mqtt_client_subscribe()` 的实现。如果设备收不到 OTA 下行消息，需要首先确认平台当前是否仍能把 `$oc/devices/{device_id}/sys/events/down` 推送到当前 MQTT 会话。

### 8.2 为什么下载失败

当前项目中最直接的下载路径位于 `components/service/ota/hw_iot_ota.c`。排查应重点看：

- `url` 是否为空或过长
- `access_token` 是否为空或过长
- HTTPS 证书链校验是否通过
- Bearer Token 是否有效
- 设备是否能访问 OTA URL

如果日志停在 `Failed to start OTA task, err=...`，说明 `esp_https_ota()` 返回错误，需要结合前面的 TLS/HTTP 日志继续定位。

### 8.3 为什么校验失败

当前仓库未实现自定义签名、CRC、SHA256、MD5 或 `sign` 字段校验。

因此这里的“校验失败”只能优先从以下方向排查：

- `esp_https_ota()` 内部镜像校验失败
- 固件镜像与芯片/分区不匹配
- 固件超出 OTA 分区大小

当前项目不能把平台 JSON 里的 `sign` 字段作为排查入口，因为代码没有使用该字段。

### 8.4 为什么升级后没有切换到新固件

先检查：

- 分区表是否确实使用 `custom_partitions_two_ota.csv`
- `sdkconfig` 是否仍为 `CONFIG_PARTITION_TABLE_CUSTOM=y`
- OTA 日志是否出现 `esp_https_ota: Starting OTA...`
- OTA 成功后是否执行了 `esp_restart()`

当前仓库未发现自定义 `esp_ota_set_boot_partition()` 调用，因此如果切换失败，应优先从 `esp_https_ota()` 的执行结果和分区表本身排查。

### 8.5 为什么重启后回滚

当前项目未启用自动回滚：

- `CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE` 未启用
- `CONFIG_APP_ROLLBACK_ENABLE` 未启用

因此当前项目不存在代码层面的“自动回滚机制”。如果出现启动后仍回到旧固件，应先怀疑：

- OTA 实际没有成功写入
- 设备并未切换到新的 OTA 分区
- 启动的是旧分区而非新分区

### 8.6 为什么平台状态与设备实际状态不一致

这是当前项目最需要维护者注意的边界之一。

当前项目已实现：

- `version_query -> version_report`

当前项目未实现：

- OTA 进度上报
- OTA 成功/失败状态上报

因此平台侧若依赖显式升级结果回报，当前项目会出现“设备已升级，但平台状态没有完整闭环”的情况。

## 9. 当前实现的限制与注意事项

以下内容为当前仓库已确认的实现边界：

1. **当前项目已实现华为云 MQTT 触发的 HTTPS OTA 主链路。**
2. **当前项目未实现设备端版本比较逻辑。**
3. **当前项目未实现 OTA 进度上报。**
4. **当前项目未实现 OTA 成功/失败结果上报。**
5. **当前项目未实现基于 `sign/file_size/expires/version` 的自定义校验逻辑。**
6. **当前项目未实现回滚确认与失败自动回滚。**
7. **当前项目未发现上传 OTA 包、创建 OTA 任务的脚本；平台操作依赖人工。**
8. **当前项目未发现 OTA 打包脚本；当前固件包直接来自 `idf.py build` 生成的 `build/base-project.bin`。**
9. **当前 OTA 下载代码显式设置了 `skip_cert_common_name_check = true`，这属于当前实现的测试性妥协，不是完整安全闭环。**
10. **当前仓库未发现显式 MQTT 订阅调用，维护者应把这一点视为现有实现边界。**

## 10. 附录

### 10.1 当前项目常用命令

```bash
idf.py build
idf.py flash
idf.py monitor
```

### 10.2 当前项目关键配置项

来自 `sdkconfig`：

- `CONFIG_PARTITION_TABLE_CUSTOM=y`
- `CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="custom_partitions_two_ota.csv"`
- `CONFIG_ESPTOOLPY_FLASHSIZE="16MB"`
- `CONFIG_ESP_HTTP_CLIENT_ENABLE_HTTPS=y`
- `CONFIG_MBEDTLS_CERTIFICATE_BUNDLE=y`
- `CONFIG_HW_IOT_HOSTNAME="f1614c4895.iotda-device.cn-south-4.myhuaweicloud.com"`
- `CONFIG_HW_IOT_PORT=8883`
- `# CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE is not set`
- `# CONFIG_APP_ROLLBACK_ENABLE is not set`

### 10.3 当前仓库中与 OTA 直接相关的构建产物

来自 `build/` 目录：

- `base-project.bin`
- `base-project.elf`
- `ota_data_initial.bin`
- `partition_table/`
- `flash_args`
- `flash_project_args`

这些文件说明当前仓库使用标准 ESP-IDF 构建链路，没有额外的 OTA 打包脚本。
