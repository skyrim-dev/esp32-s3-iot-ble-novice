# AGENTS.md

## 仓库定位
- 这是一个 ESP-IDF 5.5.1 的 ESP32-S3 工程；根构建入口是 `CMakeLists.txt`，应用入口是 `main/main.c` 里的 `app_main()`。
- 仓库没有 `README*`、`.github/workflows/`、`.pre-commit-config.yaml`、`opencode.json`；判断流程时以 `CMakeLists.txt`、`sdkconfig`、`components/*/Kconfig`、`docs/*.md` 和本文件为准。

## 先读哪里
- `main/main.c`：真实启动顺序是 LED 初始化 → 打印版本 → `wifi_init()` → 等待 `wifi_connected_semaphore` → `hw_iot_mqtt_init()` → `hw_iot_mqtt_properties_publish()`。
- `main/CMakeLists.txt`：主组件只显式依赖 `protocol`、`network`、`esp_driver_gpio`。
- `components/network/`：Wi-Fi 层；`wifi.h` 直接把 `CONFIG_WIFI_*` 映射成 `WIFI_*` 宏。
- `components/protocol/`：华为云 IoT MQTT 层；`components/protocol/CMakeLists.txt` 通过 `EMBED_FILES "hw_iot_mqtt/cert.pem"` 嵌入证书，并依赖 `service`。
- `components/service/ota/`：OTA 服务层；工程已经接入 OTA 相关代码和分区。
- `docs/项目层级结构.md` 是目标分层说明，不是完整现状；当前实际组件只有 `network`、`protocol`、`service`。

## 开发命令（仓库内可验证）
- `idf.py menuconfig`：调整 Wi-Fi、华为云 IoT 等 Kconfig 参数。
- `idf.py reconfigure`：Kconfig 改动未生效时重新配置。
- `idf.py build`：这个仓库没有 repo-local 测试/lint/typecheck 命令时，默认最小验证就是它。
- `idf.py flash` / `idf.py -p PORT flash`：`idf.py build` 成功后烧录固件；更底层的 esptool 参数以生成后的 `build/flash_args` 为准。
- `idf.py fullclean && idf.py build`：改了 `components/protocol/hw_iot_mqtt/cert.pem` 或遇到 Kconfig / 嵌入资源缓存问题时用这个组合。`docs/PEM 证书配置指南.md` 和 `docs/Kconfig 配置指南.md` 都明确提到了这条路径。

## 配置与生成物约束
- `sdkconfig` 文件头明确写了 `Automatically generated file. DO NOT EDIT.`；改配置用 `menuconfig` / Kconfig，不要手改。
- `.gitignore` 把 `build/`、`sdkconfig`、`sdkconfig.old`、`dependencies.lock` 当作本地/生成产物处理；不要默认这些文件应该被直接编辑或提交。
- 当前分区表不是默认模板，而是 `sdkconfig` 指向的 `custom_partitions_two_ota.csv`；里面有 `factory`、`ota_0`、`ota_1` 三个 1500k app 分区。改 OTA 逻辑前先确认分区表仍匹配。
- `build/` 下的 `flash_args`、`flasher_args.json`、`partition_table/partition-table.bin`、由证书生成的 `.S` 文件都属于构建输出；不要直接修改。需要核对烧录布局时，优先看生成后的 `build/flash_args` 或 `build/flasher_args.json`。

## 改动时容易踩坑的点
- Wi-Fi 参数真实来源是 `components/network/Kconfig`，协议参数真实来源是 `components/protocol/Kconfig`；不要把 SSID、密码、MQTT 端点再硬编码到源码。
- `components/protocol/CMakeLists.txt` 依赖 `service`，说明 MQTT 层和 OTA 服务存在耦合；改协议层时别只看 `main/`。
- 证书文件路径固定为 `components/protocol/hw_iot_mqtt/cert.pem`；如果新增嵌入文件，沿用 `EMBED_FILES` 模式，不要手写二进制数组。
- 仓库没有 CI/预提交钩子帮你兜底；提交前至少自己跑一遍 `idf.py build`。

## 新增代码时的默认落点
- 入口编排继续放在 `main/`。
- 可复用功能优先放 `components/`，并在对应 `CMakeLists.txt` 里补 `REQUIRES`。
- 配置项优先放组件自己的 `Kconfig`，不要把可配置参数藏进 `.c/.h` 常量。
