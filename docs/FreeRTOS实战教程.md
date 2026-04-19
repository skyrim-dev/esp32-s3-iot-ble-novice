# FreeRTOS 实战教程

> 本教程采用「分支学习法」：每个知识点对应一个 Git 分支，通过创建分支、编码、提交来掌握 FreeRTOS 核心概念。

## 学习路径总览

| 顺序 | 分支名 | 知识点 | 难度 |
|------|--------|--------|------|
| 1 | `learn/01-task-create` | 任务创建与删除 | ⭐ |
| 2 | `learn/02-task-delay` | 任务延时与空闲任务 | ⭐ |
| 3 | `learn/03-queue` | 队列通信 | ⭐⭐ |
| 4 | `learn/04-binary-semaphore` | 二值信号量同步 | ⭐⭐ |
| 5 | `learn/05-counting-semaphore` | 计数信号量 | ⭐⭐ |
| 6 | `learn/06-mutex` | 互斥锁保护共享资源 | ⭐⭐⭐ |
| 7 | `learn/07-event-group` | 事件组 | ⭐⭐⭐ |
| 8 | `learn/08-timer` | 软件定时器 | ⭐⭐⭐ |
| 9 | `learn/09-interrupt-queue` | 中断与队列 | ⭐⭐⭐⭐ |
| 10 | `learn/10-esp32-wifi-task` | 综合实战：WiFi 任务通信 | ⭐⭐⭐⭐ |

---

## 准备工作

### 1. 确认当前分支干净

```bash
git status
# 确保工作区干净，或者暂存当前改动
```

### 2. 确认 ESP-IDF 环境

```bash
idf.py version
# 确认是 5.5.1
```

### 3. 从基础分支开始学习

所有知识点基于当前 `main/main.c` 的框架学习，进入项目根目录后，开始第一个知识点。

---

## 第一阶段：任务基础

### 知识点 1：任务创建与删除

**分支**：`learn/01-task-create`

**学会**：
- `xTaskCreate()` 创建任务
- `vTaskDelete()` 删除任务
- 任务句柄的使用

**Step 1: 创建分支并编写代码**

```bash
# 1. 创建分支
git checkout -b learn/01-task-create

# 2. 复制以下代码到 main/main.c（替换原有 app_main）
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "main";

// 任务句柄
static TaskHandle_t my_task_handle = NULL;

// 任务函数
void my_first_task(void *param)
{
    for (int i = 0; i < 5; i++) {
        ESP_LOGI(TAG, "Task running: %d", i);
        vTaskDelay(pdMS_TO_TICKS(1000));  // 延时 1 秒
    }
    ESP_LOGI(TAG, "Task completed, deleting itself");
    vTaskDelete(NULL);  // 删除自己
}

void app_main(void)
{
    ESP_LOGI(TAG, "Creating my first FreeRTOS task");
    
    // 创建任务
    BaseType_t result = xTaskCreate(
        my_first_task,          // 任务函数
        "my_first_task",       // 任务名称
        4096,                  // 栈大小（字节）
        NULL,                   // 参数
        1,                     // 优先级（1 及以上）
        &my_task_handle         // 任务句柄
    );
    
    if (result == pdTRUE) {
        ESP_LOGI(TAG, "Task created successfully");
    } else {
        ESP_LOGE(TAG, "Failed to create task");
    }
}
```

**Step 3: 编译验证**

```bash
idf.py build
```

**Step 4: 提交学习成果**

```bash
git add main/main.c
git commit -m "feat: 学习任务创建与删除"
```

---

### 知识点 2：任务延时与空闲任务

**分支**：`learn/02-task-delay`

**学会**：
- `vTaskDelay()` 相对延时
- `pdMS_TO_TICKS()` 毫秒转tick
- 空闲任务（CPU 利用率 100% 时运行）
- 优先级抢占

**Step 1: 创建分支**

```bash
git checkout -b learn/02-task-delay
```

**Step 2: 编写代码**

```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "main";

// 高优先级任务
void high_priority_task(void *param)
{
    for (int i = 0; i < 10; i++) {
        ESP_LOGI(TAG, "High priority: %d", i);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    vTaskDelete(NULL);
}

// 低优先级任务  
void low_priority_task(void *param)
{
    for (int i = 0; i < 10; i++) {
        ESP_LOGI(TAG, "Low priority: %d", i);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    vTaskDelete(NULL);
}

// 关键：证明空闲任务存在
void idle_task_demo(void *param)
{
    ESP_LOGI(TAG, "Idle task demonstrating idle hook");
    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== Task Delay Demo ===");
    
    xTaskCreate(high_priority_task, "high", 4096, NULL, 2, NULL);
    xTaskCreate(low_priority_task, "low", 4096, NULL, 1, NULL);
    
    // 注册空闲任务钩子（需要开启配置）
    // esp_register_freertos_idlehook();
}
```

**Step 3: 编译并提交**

```bash
idf.py build
git add main/main.c
git commit -m "feat: 学习任务延时与空闲任务"
```

---

## 第二阶段：任务间通信

### 知识点 3：队列通信

**分支**：`learn/03-queue`

**学会**：
- `xQueueCreate()` 创建队列
- `xQueueSend()` 发送消息
- `xQueueReceive()` 接收消息
- 队列句柄的使用

**Step 1: 创建分支**

```bash
git checkout -b learn/03-queue
```

**Step 2: 编写代码**

```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

static const char *TAG = "main";

// 队列句柄（全局）
static QueueHandle_t my_queue = NULL;

// 发送任务
void sender_task(void *param)
{
    int count = 0;
    while (1) {
        // 发送整型数据
        if (xQueueSend(my_queue, &count, pdMS_TO_TICKS(100)) == pdTRUE) {
            ESP_LOGI(TAG, "Sent: %d", count);
            count++;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// 接收任务
void receiver_task(void *param)
{
    int received = 0;
    while (1) {
        if (xQueueReceive(my_queue, &received, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "Received: %d", received);
        }
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== Queue Demo ===");
    
    // 创建队列：长度 5，存储 int 类型
    my_queue = xQueueCreate(5, sizeof(int));
    
    if (my_queue != NULL) {
        xTaskCreate(receiver_task, "receiver", 4096, NULL, 2, NULL);
        xTaskCreate(sender_task, "sender", 4096, NULL, 1, NULL);
    } else {
        ESP_LOGE(TAG, "Queue create failed");
    }
}
```

**Step 3: 编译并提交**

```bash
idf.py build
git add main/main.c
git commit -m "feat: 学习队列通信"
```

---

### 知识点 4：二值信号量同步

**分支**：`learn/04-binary-semaphore`

**学会**：
- `xSemaphoreCreateBinary()` 创建二值信号量
- `xSemaphoreGive()` 释放信号量
- `xSemaphoreTake()` 获取信号量
- 任务同步场景

**Step 1: 创建分支**

```bash
git checkout -b learn/04-binary-semaphore
```

**Step 2: 编写代码**

```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

static const char *TAG = "main";

// 二值信号量（用于同步）
static SemaphoreHandle_t sync_sem = NULL;

// 等待信号量的任务
void waiter_task(void *param)
{
    while (1) {
        // 获取信号量（阻塞等待）
        if (xSemaphoreTake(sync_sem, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "Semaphore received! Processing...");
            // 模拟处理
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}

// 发送信号量的任务
void giver_task(void *param)
{
    int count = 0;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(2000));
        count++;
        ESP_LOGI(TAG, "Giving semaphore #%d", count);
        xSemaphoreGive(sync_sem);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== Binary Semaphore Demo ===");
    
    // 创建二值信号量（初始为 0）
    sync_sem = xSemaphoreCreateBinary();
    
    if (sync_sem != NULL) {
        // 先给一次，让 waiter 可以运行
        xSemaphoreGive(sync_sem);
        
        xTaskCreate(waiter_task, "waiter", 4096, NULL, 2, NULL);
        xTaskCreate(giver_task, "giver", 4096, NULL, 1, NULL);
    }
}
```

**Step 3: 编译并提交**

```bash
idf.py build
git add main/main.c
git commit -m "feat: 学习二值信号量同步"
```

---

### 知识点 5：计数信号量

**分支**：`learn/05-counting-semaphore`

**学会**：
- `xSemaphoreCreateCounting()` 创建计数信号量
- 资源池场景（限流器）

**Step 1: 创建分支**

```bash
git checkout -b learn/05-counting-semaphore
```

**Step 2: 编写代码**

```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

static const char *TAG = "main";

// 计数信号量（资源池：最多 3 个并发）
static SemaphoreHandle_t resource_pool = NULL;

// 使用资源的任务
void resource_user_task(void *param)
{
    TaskHandle_t handle = xTaskGetCurrentTaskHandle();
    const char *name = pcTaskGetName(handle);
    
    while (1) {
        ESP_LOGI(TAG, "%s waiting for resource...", name);
        
        if (xSemaphoreTake(resource_pool, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "%s acquired resource!", name);
            
            // 模拟使用资源（1-3 秒）
            vTaskDelay(pdMS_TO_TICKS(2000 + (esp_random() % 2000)));
            
            ESP_LOGI(TAG, "%s releasing resource!", name);
            xSemaphoreGive(resource_pool);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== Counting Semaphore Demo ===");
    
    // 创建计数信号量：最大计数 3，初始 3
    resource_pool = xSemaphoreCreateCounting(3, 3);
    
    if (resource_pool != NULL) {
        // 创建 5 个用户竞争 3 个资源
        for (int i = 0; i < 5; i++) {
            char name[16];
            snprintf(name, sizeof(name), "user_%d", i);
            xTaskCreate(resource_user_task, name, 4096, NULL, 1, NULL);
        }
    }
}
```

**Step 3: 编译并提交**

```bash
idf.py build
git add main/main.c
git commit -m "feat: 学习计数信号量"
```

---

## 第三阶段：同步与保护

### 知识点 6：互斥锁

**分支**：`learn/06-mutex`

**学会**：
- `xSemaphoreCreateMutex()` 创建互斥锁
- `xSemaphoreTakeRecursive()` 递归获取
- `xSemaphoreGiveRecursive()` 递归释放
- 保护共享资源

**Step 1: 创建分支**

```bash
git checkout -b learn/06-mutex
```

**Step 2: 编写代码**

```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

static const char *TAG = "main";

// 互斥锁（保护共享资源）
static SemaphoreHandle_t mutex = NULL;

// 共享资源
static int shared_counter = 0;

// 任务 A：递增共享计数器
void increment_task(void *param)
{
    for (int i = 0; i < 1000; i++) {
        xSemaphoreTake(mutex, portMAX_DELAY);
        shared_counter++;  // 临界区
        xSemaphoreGive(mutex);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    ESP_LOGI(TAG, "Increment task done, counter=%d", shared_counter);
    vTaskDelete(NULL);
}

// 任务 B：递减共享计数器
void decrement_task(void *param)
{
    for (int i = 0; i < 1000; i++) {
        xSemaphoreTake(mutex, portMAX_DELAY);
        shared_counter--;  // 临界区
        xSemaphoreGive(mutex);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    ESP_LOGI(TAG, "Decrement task done, counter=%d", shared_counter);
    vTaskDelete(NULL);
}

// 任务 C：读取共享计数器（验证）
void read_task(void *param)
{
    for (int i = 0; i < 10; i++) {
        xSemaphoreTake(mutex, portMAX_DELAY);
        ESP_LOGI(TAG, "Current counter: %d", shared_counter);
        xSemaphoreGive(mutex);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== Mutex Demo ===");
    
    // 创建互斥锁
    mutex = xSemaphoreCreateMutex();
    
    if (mutex != NULL) {
        xTaskCreate(increment_task, "increment", 4096, NULL, 1, NULL);
        xTaskCreate(decrement_task, "decrement", 4096, NULL, 1, NULL);
        xTaskCreate(read_task, "read", 4096, NULL, 1, NULL);
    }
}
```

**Step 3: 编译并提交**

```bash
idf.py build
git add main/main.c
git commit -m "feat: 学习互斥锁保护共享资源"
```

---

### 知识点 7：事件组

**分支**：`learn/07-event-group`

**学会**：
- `xEventGroupCreate()` 创建事件组
- `xEventGroupSetBits()` 设置事件位
- `xEventGroupWaitBits()` 等待事件位
- 多任务同步

**Step 1: 创建分支**

```bash
git checkout -b learn/07-event-group
```

**Step 2: 编写代码**

```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

static const char *TAG = "main";

// 定义事件位
#define BIT_WIFI_CONNECTED   (1 << 0)  // 0b0001
#define BIT_MQTT_CONNECTED (1 << 1)  // 0b0010
#define BIT_NTP_SYNCED     (1 << 2)  // 0b0100
#define BIT_ALL_READY    (BIT_WIFI_CONNECTED | BIT_MQTT_CONNECTED | BIT_NTP_SYNCED)

// 事件组
static EventGroupHandle_t system_events = NULL;

// WiFi 任务
void wifi_task(void *param)
{
    ESP_LOGI(TAG, "WiFi connecting...");
    vTaskDelay(pdMS_TO_TICKS(1000));
    xEventGroupSetBits(system_events, BIT_WIFI_CONNECTED);
    ESP_LOGI(TAG, "WiFi connected!");
    vTaskDelete(NULL);
}

// MQTT 任务
void mqtt_task(void *param)
{
    ESP_LOGI(TAG, "MQTT connecting...");
    vTaskDelay(pdMS_TO_TICKS(1500));
    xEventGroupSetBits(system_events, BIT_MQTT_CONNECTED);
    ESP_LOGI(TAG, "MQTT connected!");
    vTaskDelete(NULL);
}

// NTP 任务
void ntp_task(void *param)
{
    ESP_LOGI(TAG, "NTP syncing...");
    vTaskDelay(pdMS_TO_TICKS(2000));
    xEventGroupSetBits(system_events, BIT_NTP_SYNCED);
    ESP_LOGI(TAG, "NTP synced!");
    vTaskDelete(NULL);
}

// 主任务：等待所有事件
void main_task(void *param)
{
    ESP_LOGI(TAG, "Waiting for all services...");
    
    // 等待所有服务就绪（清除位）
    EventBits_t bits = xEventGroupWaitBits(
        system_events,
        BIT_ALL_READY,
        pdTRUE,  // 清除已设置位
        pdTRUE,  // 全部等待
        portMAX_DELAY
    );
    
    if ((bits & BIT_ALL_READY) == BIT_ALL_READY) {
        ESP_LOGI(TAG, "=== ALL SERVICES READY ===");
        ESP_LOGI(TAG, "System ready to work!");
    }
    
    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== Event Group Demo ===");
    
    // 创建事件组
    system_events = xEventGroupCreate();
    
    if (system_events != NULL) {
        xTaskCreate(wifi_task, "wifi", 4096, NULL, 1, NULL);
        xTaskCreate(mqtt_task, "mqtt", 4096, NULL, 1, NULL);
        xTaskCreate(ntp_task, "ntp", 4096, NULL, 1, NULL);
        xTaskCreate(main_task, "main", 4096, NULL, 1, NULL);
    }
}
```

**Step 3: 编译并提交**

```bash
idf.py build
git add main/main.c
git commit -m "feat: 学习事件组"
```

---

## 第四阶段：定时器与中断

### 知识点 8：软件定时器

**分支**：`learn/08-timer`

**学会**：
- `xTimerCreate()` 创建软件定时器
- `xTimerStart()` 启动定时器
- 回调函数执行上下文

**Step 1: 创建分支**

```bash
git checkout -b learn/08-timer
```

**Step 2: 编写代码**

```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_log.h"

static const char *TAG = "main";

// 定时器句柄
static TimerHandle_t periodic_timer = NULL;

// 定時器回调函数（注意：在中断上下文中运行）
void timer_callback(TimerHandle_t timer)
{
    static int count = 0;
    count++;
    ESP_LOGI(TAG, "Timer tick: %d", count);
}

// 普通任务
void normal_task(void *param)
{
    for (int i = 0; i < 10; i++) {
        ESP_LOGI(TAG, "Normal task running");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    ESP_LOGI(TAG, "Normal task done");
    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== Software Timer Demo ===");
    
    // 创建周期性定时器：名称，周期 1000ms，自动重载
    periodic_timer = xTimerCreate(
        "periodic",
        pdMS_TO_TICKS(1000),
        pdTRUE,           // 自动重载
        NULL,
        timer_callback
    );
    
    if (periodic_timer != NULL) {
        xTimerStart(periodic_timer, 0);
        ESP_LOGI(TAG, "Timer started");
    }
    
    xTaskCreate(normal_task, "normal", 4096, NULL, 1, NULL);
}
```

**Step 3: 编译并提交**

```bash
idf.py build
git add main/main.c
git commit -m "feat: 学习软件定时器"
```

---

### 知识点 9：中断与队列

**分支**：`learn/09-interrupt-queue`

**学会**：
- ISR（中断服务程序）中调用FreeRTOS API
- `xQueueSendFromISR()` 从中断发送
- `portEND_SWITCHING_ISR()` 立即触发任务切换

**Step 1: 创建分支**

```bash
git checkout -b learn/09-interrupt-queue
```

**Step 2: 编写代码**

```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_idf_version.h"

static const char *TAG = "main";

// GPIO 35（可改为其他外部中断引脚）
#define GPIO_INPUT_IO_0     35

// 队列（用于中断到任务的通信）
static QueueHandle_t event_queue = NULL;

// 按键中断处理（外部中断）
void IRAM_ATTR gpio_isr_handler(void *arg)
{
    // 从 ISR 发送数据到队列
    int data = gpio_get_level(GPIO_INPUT_IO_0);
    BaseType_t higher_priority_task_woken = pdFALSE;
    
    xQueueSendFromISR(event_queue, &data, &higher_priority_task_woken);
    
    if (higher_priority_task_woken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

// 处理队列的任务
void event_handler_task(void *param)
{
    int value;
    while (1) {
        if (xQueueReceive(event_queue, &value, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "GPIO value changed: %d", value);
        }
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== Interrupt + Queue Demo ===");
    
    // 创建队列
    event_queue = xQueueCreate(10, sizeof(int));
    
    // GPIO 配置
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_INPUT_IO_0),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    gpio_config(&io_conf);
    
    // 安装中断驱动
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, NULL);
    
    // 创建处理任务
    xTaskCreate(event_handler_task, "handler", 4096, NULL, 2, NULL);
    
    ESP_LOGI(TAG, "Interrupt configured on GPIO %d", GPIO_INPUT_IO_0);
}
```

**Step 3: 编译并提交**

```bash
idf.py build
git add main/main.c
git commit -m "feat: 学习中断与队列通信"
```

---

## 第五阶段：综合实战

### 知识点 10：综合实战 - WiFi 任务通信

**分支**：`learn/10-esp32-wifi-task`

**学会**：
- 综合运用：队列 + 信号量 + 任务创建
- 将 WiFi 事件通过队列上报
- 主任务处理 WiFi 事件

**Step 1: 创建分支**

```bash
git checkout -b learn/10-esp32-wifi-task
```

**Step 2: 编写代码**

```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"

static const char *TAG = "main";

// WiFi 事件队列
static QueueHandle_t wifi_event_queue = NULL;

// WiFi 事件类型
typedef enum {
    WIFI_EVENT_CONNECTED,
    WIFI_EVENT_DISCONNECTED,
    WIFI_EVENT_GOT_IP
} wifi_event_type_t;

// WiFi 事件结构
typedef struct {
    wifi_event_type_t type;
    char info[64];
} wifi_event_t;

// WiFi 事件处理任务
void wifi_event_handler_task(void *param)
{
    wifi_event_t event;
    while (1) {
        if (xQueueReceive(wifi_event_queue, &event, portMAX_DELAY) == pdTRUE) {
            switch (event.type) {
                case WIFI_EVENT_CONNECTED:
                    ESP_LOGI(TAG, "[HANDLER] WiFi connected: %s", event.info);
                    break;
                case WIFI_EVENT_DISCONNECTED:
                    ESP_LOGI(TAG, "[HANDLER] WiFi disconnected: %s", event.info);
                    break;
                case WIFI_EVENT_GOT_IP:
                    ESP_LOGI(TAG, "[HANDLER] Got IP: %s", event.info);
                    break;
            }
        }
    }
}

// WiFi 初始化任务
void wifi_init_task(void *param)
{
    // 初始化 NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }
    
    // 初始化 TCP/IP 栈
    ESP_ERROR_CHECK(esp_netif_init());
    
    // 创建默认 WiFi STA
    esp_netif_create_default_wifi_sta();
    
    // WiFi 配置
    wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init_config));
    
    // 注册事件
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, NULL, &instance_got_ip));
    
    // WiFi 配置参数（从 Kconfig）
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    
    // 从 NVS 读取配置或使用默认
    #ifdef CONFIG_WIFI_SSID
    strcpy((char *)wifi_config.sta.ssid, CONFIG_WIFI_SSID);
    strcpy((char *)wifi_config.sta.password, CONFIG_WIFI_PASSWORD);
    #endif
    
    ESP_LOGI(TAG, "Connecting to SSID: %s", wifi_config.sta.ssid);
    
    // 启动 WiFi
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "wifi_init_task finished");
    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== WiFi Task Communication Demo ===");
    
    // 创建 WiFi 事件队列
    wifi_event_queue = xQueueCreate(10, sizeof(wifi_event_t));
    
    // 创建处理任务（高优先级）
    xTaskCreate(wifi_event_handler_task, "wifi_handler", 4096, NULL, 2, NULL);
    
    // 创建 WiFi 初始化任务（低优先级）
    xTaskCreate(wifi_init_task, "wifi_init", 8192, NULL, 1, NULL);
    
    // 注意：完整 WiFi 事件处理需要配合 esp_event_loop
    // 这里仅演示任务间通信模式
    ESP_LOGI(TAG, "Demo requires full WiFi configuration");
}
```

**Step 3: 编译并提交**

```bash
idf.py build
git add main/main.c
git commit -m "feat: 综合实战-FreeRTOS任务通信"
```

---

## 学习总结

完成所有分支后，你的 FreeRTOS 技能树：

```
FreeRTOS 核心概念
├── 任务管理
│   ├── xTaskCreate()      ✅
│   ├── vTaskDelete()      ✅
│   └── vTaskDelay()       ✅
├── 任务间通信
│   ├── 队列 (Queue)      ✅
│   └── 信号量 (Semaphore) ✅
│       ├── 二值信号量    ✅
│       └── 计数信号量    ✅
├── 同步与保护
│   ├── 互斥锁 (Mutex)    ✅
│   └── 事件组 (Event)   ✅
├── 定时器
│   └── 软件定时器       ✅
└── 中断
    └── xQueueSendFromISR() ✅
```

### 查看学习成果

```bash
# 查看所有学习分支
git branch --list 'learn/*'

# 查看提交历史
git log --oneline --graph --all --decorate
```

### 清理学习分支（可选）

```bash
# 删除所有学习分支
git branch --list 'learn/*' | xargs -r git branch -D

# 切回主分支
git checkout main
```

## 下一步建议

1. **深入学习**：阅读 FreeRTOS 官方文档 https://freertos.org/
2. **实战项目**：将本项目的 WiFi、MQTT 代码重构为多任务架构
3. **内核调试**：学习 `FreeRTOS+trace` 或 `esp_idf_system_view`
4. **性能优化**：学习任务栈大小计算、优先级设计原则