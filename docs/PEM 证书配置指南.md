# PEM 证书配置指南

## 什么是 PEM 证书

PEM (Privacy Enhanced Mail) 是一种常见的证书格式，用于 SSL/TLS 加密通信。在 ESP32 连接华为云 IoT 平台时，需要使用 CA 证书来验证服务器身份。

## 项目中的证书文件

```
components/protocol/hw_iot_mqtt/
├── cert.pem              # CA 根证书
├── hw_iot_mqtt_config.c  # MQTT 配置
└── ...
```

## 配置步骤

### 1. 准备证书文件

将 CA 证书文件（如 `GlobalSign-rootca.pem`）放入组件目录：

```bash
cp /path/to/your/cert.pem components/protocol/hw_iot_mqtt/cert.pem
```

### 2. 配置 CMakeLists.txt

在组件的 `CMakeLists.txt` 中添加 `EMBED_FILES`：

```cmake
idf_component_register(
    SRCS 
        "hw_iot_mqtt_config.c"
        "hw_iot_mqtt_json.c"
        "hw_iot_mqtt_topic.c"
    INCLUDE_DIRS "."
    REQUIRES esp_netif mqtt nvs_flash json esp_wifi
    EMBED_FILES "cert.pem"      # 添加证书文件
)
```

### 3. 代码中使用

在 C 代码中声明并使用嵌入式证书：

```c
// 声明嵌入式文件符号
extern const char _binary_cert_pem_start[] asm("_binary_cert_pem_start");
extern const char _binary_cert_pem_end[] asm("_binary_cert_pem_end");

// 使用证书
esp_mqtt_client_config_t mqtt_cfg = {
    .broker.verification.certificate = _binary_cert_pem_start,
    // ...
};
```

## 符号命名规则

ESP-IDF 会根据文件名自动生成符号名：

| 文件名 | 符号名 |
|--------|--------|
| `cert.pem` | `_binary_cert_pem_start` / `_binary_cert_pem_end` |
| `ca.pem` | `_binary_ca_pem_start` / `_binary_ca_pem_end` |
| `server.crt` | `_binary_server_crt_start` / `_binary_server_crt_end` |

**注意**：
- 文件名中的 `.` 会被替换为 `_`
- 文件名中的 `-` 会被替换为 `_`
- 建议使用简单文件名（如 `cert.pem`）避免混淆

## EMBED_FILES vs 头文件

| 方式 | 优点 | 缺点 |
|------|------|------|
| **EMBED_FILES** | 证书独立、安全、可读性好 | 配置稍复杂 |
| **头文件** | 简单直接 | 证书明文在代码中 |

**推荐使用 EMBED_FILES 方式**，便于证书管理和更新。

## 常见问题

### Q: 编译报错 `undefined reference to _binary_xxx_start`

**A**: 检查以下几点：
1. 证书文件是否在正确位置
2. CMakeLists.txt 是否添加了 `EMBED_FILES`
3. 文件名是否包含特殊字符（建议只用字母和数字）

### Q: 如何更换证书？

1. 替换 `cert.pem` 文件
2. 清理并重新编译：

```bash
idf.py fullclean
idf.py build
```

### Q: 证书文件放在哪里？

建议放在使用证书的组件目录下：
```
components/protocol/hw_iot_mqtt/
├── CMakeLists.txt
└── cert.pem
```

## 相关文件

- `components/protocol/hw_iot_mqtt/cert.pem` - CA 证书
- `components/protocol/hw_iot_mqtt/hw_iot_mqtt_config.c` - MQTT 配置
