# Kconfig 配置指南

## 什么是 Kconfig

Kconfig 是 ESP-IDF 提供的配置管理系统，允许用户在编译前通过图形界面（menuconfig）配置各种参数，而无需修改代码。

## 基本语法

### 配置项类型

```kconfig
# 字符串类型
config EXAMPLE_STRING
    string "Example String"
    default "default_value"
    help
        Help text describing this option.

# 整数类型
config EXAMPLE_INT
    int "Example Integer"
    default 100
    range 0 1000

# 布尔类型
config EXAMPLE_BOOL
    bool "Enable Example"
    default y
```

### 菜单结构

```kconfig
# 顶层菜单
menu "Main Menu"
    config ITEM1
        bool "Item 1"
    config ITEM2
        bool "Item 2"
endmenu

# 条件菜单（依赖其他配置）
menu "Advanced Options"
    depends on EXAMPLE_BOOL
    config ADVANCED_ITEM
        bool "Advanced Item"
endmenu
```

## 项目中的 Kconfig

### WiFi 配置示例

项目中的 `components/network/Kconfig` 提供了 WiFi 配置选项：

```kconfig
menu "WiFi Configuration"

    config WIFI_SSID
        string "WiFi SSID"
        default "MyWiFi"
        help
            SSID (network name) for the ESP32 to connect to.

    config WIFI_PASSWORD
        string "WiFi Password"
        default "password"
        help
            WiFi password (WPA or WPA2) for the ESP32 to use.

    config WIFI_CONNECT_TIMEOUT_MS
        int "WiFi connect timeout (ms)"
        default 30000
        range 5000 120000
        help
            Timeout in milliseconds for WiFi connection.

endmenu
```

## 使用方法

### 1. 进入配置菜单

```bash
idf.py menuconfig
```

### 2. 找到配置项

配置项通常在以下位置：
- `Component config` - 组件级配置
- `Example Configuration` - 示例配置
- 顶级菜单 - 直接显示

### 3. 修改配置

- 使用方向键导航
- 按 `Y` 启用布尔选项（显示 `[*]`）
- 按 `N` 禁用布尔选项（显示 `[ ]`）
- 按 `Enter` 进入字符串/整数选项编辑

### 4. 保存退出

- 按 `Esc` 返回上级菜单
- 选择 "Save" 保存配置
- 选择 "Exit" 退出

## 代码中使用配置

配置项会自动生成 `CONFIG_*` 宏：

```c
#include "sdkconfig.h"

// 使用字符串配置
const char* ssid = CONFIG_WIFI_SSID;

// 使用整数配置
int timeout_ms = CONFIG_WIFI_CONNECT_TIMEOUT_MS;

// 使用布尔配置
#if CONFIG_WIFI_RECONNECT
    // 启用重连功能
#endif
```

## 头文件中引用

```c
// wifi.h
#ifndef WIFI_H
#define WIFI_H

// WiFi配置 (从 Kconfig 读取)
#define WIFI_SSID      CONFIG_WIFI_SSID
#define WIFI_PWD       CONFIG_WIFI_PASSWORD
#define WIFI_TIMEOUT   CONFIG_WIFI_CONNECT_TIMEOUT_MS

#endif
```

## 常见问题

### Q: 修改 Kconfig 后没有生效

**A:** 需要重新配置项目：

```bash
idf.py reconfigure
```

或者清理后重新构建：

```bash
idf.py fullclean
idf.py build
```

### Q: 找不到配置项

**A:** 检查 Kconfig 文件是否在组件目录下，且 CMakeLists.txt 正确配置。

### Q: 如何添加新的配置项

**A:**

1. 在组件目录下创建/编辑 `Kconfig` 文件：

```kconfig
menu "My Component Config"

    config MY_FEATURE_ENABLED
        bool "Enable My Feature"
        default y

    config MY_FEATURE_TIMEOUT
        int "Feature timeout (ms)"
        default 1000
        range 100 10000

endmenu
```

2. 在代码中使用：

```c
#if CONFIG_MY_FEATURE_ENABLED
    // 功能代码
#endif
```

## 相关文件

- `components/network/Kconfig` - WiFi 配置
- `sdkconfig` - 编译后的配置文件（不提交到 git）
