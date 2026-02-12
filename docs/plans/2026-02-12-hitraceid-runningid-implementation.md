# AppRunningRecord HiTraceId RunningID 实现方案

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans 来实现这个计划。

**目标:** 为 AppRunningRecord 新增基于 HiTraceId 的全局唯一 RunningID，保持原有的 startTimeMillis_ 不变

**架构:** 新增成员变量 appRunningUniqueId_ 存储基于 HiTraceId 生成的唯一 ID，原有的 startTimeMillis_ 继续存储时间戳

**技术栈:** C++, HiTraceChain API, OpenHarmony ability_runtime 框架

---

## Task 1: 添加 HiTraceChain 头文件包含

**文件:**
- 修改: `services/appmgr/src/app_running_record.cpp:27`

**Step 1: 添加 HiTraceChain 头文件**

在现有的 hitrace_meter.h 包含后（约第 27-28 行）添加以下包含：

```cpp
#include "hitrace/hitracechain.h"
```

**Step 2: 验证文件编译**

运行: 检查文件是否有正确的包含
预期: 无编译错误

**Step 3: 提交**

**Step 3: 提交**

```bash
git add services/appmgr/src/app_running_record.cpp
git commit -m "feat: 添加 HiTraceChain 头文件到 AppRunningRecord"
```

---

## Task 2: 在头文件中添加新的成员变量声明

**文件:**
- 修改: `services/appmgr/include/app_running_record.h:1329`

**Step 1: 在 startTimeMillis_ 后添加新成员变量**

在第 1329 行的 startTimeMillis_ 声明后添加：

```cpp
    int64_t startTimeMillis_ = 0;   // The time of app start(CLOCK_MONOTONIC)
    uint64_t appRunningUniqueId_ = 0; // The unique running ID generated from HiTraceId
```

**Step 2: 添加 Getter 方法声明**

在类中添加 Getter 方法（约第 802 行附近，在 GetAppStartTime() 后）：

```cpp
uint64_t GetAppRunningUniqueId() const;
```

**Step 3: 验证头文件编译**

运行: 检查头文件是否编译正确
预期: 无编译错误

**Step 4: 提交**

```bash
git add services/appmgr/include/app_running_record.h
git commit -m "feat: 添加 appRunningUniqueId_ 成员变量和 Getter 方法"
```

---

## Task 3: 添加生成唯一 ID 的私有方法

**文件:**
- 修改: `services/appmgr/include/app_running_record.h`
- 修改: `services/appmgr/src/app_running_record.cpp`

**Step 1: 在头文件中添加私有方法声明**

在类的 private 部分添加（约第 802 行附近）：

```cpp
uint64_t GenerateRunningIdFromHitrace() const;
```

**Step 2: 在 cpp 文件中实现该方法**

在 cpp 文件中添加实现（约第 2174 行附近，在 GetAppStartTime() 后）：

```cpp
uint64_t AppRunningRecord::GenerateRunningIdFromHitrace() const
{
#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
    OHOS::HiviewDFX::HiTraceId hitraceId = OHOS::HiviewDFX::HiTraceChain::GetId();
    if (hitraceId.IsValid() == 0) {
        TAG_LOGW(AAFwkTag::APPMGR, "HitraceId 无效，使用随机数生成");
        std::random_device rd;
        std::mt19937_64 engine(rd());
        std::uniform_int_distribution<uint64_t> dist(
            std::numeric_limits<uint64_t>::min(),
            std::numeric_limits<uint64_t>::max()
        );
        return dist(engine);
    }
    uint64_t chainId = static_cast<uint64_t>(hitraceId.GetChainId());
    uint64_t spanId = static_cast<uint64_t>(hitraceId.GetSpanId());
    // 组合 chainId 和 spanId 确保全局唯一性
    return (chainId << 32) | (spanId & 0xFFFFFFFF);
#else
    TAG_LOGW(AAFwkTag::APPMGR, "Hitrace 未启用，使用随机数生成");
    std::random_device rd;
    std::mt19937_64 engine(rd());
    std::uniform_int_distribution<uint64_t> dist(
        std::numeric_limits<uint64_t>::min(),
        std::numeric_limits<uint64_t>::max()
    );
    return dist(engine);
#endif
}
```

**Step 3: 添加必要的头文件包含**

在 cpp 文件顶部添加（约第 27 行附近）：

```cpp
#include <random>
#include <limits>
```

**Step 4: 验证方法编译**

运行: 检查方法实现是否编译正确
预期: 无编译错误

**Step 5: 提交**

```bash
git add services/appmgr/include/app_running_record.h services/appmgr/src/app_running_record.cpp
git commit -m "feat: 添加 GenerateRunningIdFromHitrace 方法实现"
```

---

## Task 4: 实现 GetAppRunningUniqueId 方法

**文件:**
- 修改: `services/appmgr/src/app_running_record.cpp`

**Step 1: 实现 GetAppRunningUniqueId 方法**

在 cpp 文件中添加实现（约第 2174 行附近，在 GenerateRunningIdFromHitrace() 后）：

```cpp
uint64_t AppRunningRecord::GetAppRunningUniqueId() const
{
    return appRunningUniqueId_;
}
```

**Step 2: 验证方法编译**

运行: 检查方法实现是否编译正确
预期: 无编译错误

**Step 3: 提交**

```bash
git add services/appmgr/src/app_running_record.cpp
git commit -m "feat: 实现 GetAppRunningUniqueId 方法"
```

---

## Task 5: 在构造函数中初始化 appRunningUniqueId_

**文件:**
- 修改: `services/appmgr/src/app_running_record.cpp:74-78`

**Step 1: 在构造函数中添加 appRunningUniqueId_ 初始化**

在第 78 行后添加：

```cpp
    appRunningUniqueId_ = GenerateRunningIdFromHitrace();
```

**Step 2: 验证构造函数编译**

运行: 检查构造函数是否编译正确
预期: 无编译错误

**Step 3: 提交**

```bash
git add services/appmgr/src/app_running_record.cpp
git commit -m "feat: 在构造函数中初始化 appRunningUniqueId_"
```

---

## Task 6: 更新 LaunchApplication 方法使用新的唯一 ID

**文件:**
- 修改: `services/appmgr/src/app_running_record.cpp:386`

**Step 1: 检查当前实现**

查看第 386 行附近的代码，确认当前如何设置 appRunningUniqueId

**Step 2: 更新为使用新的成员变量**

将：
```cpp
launchData.SetAppRunningUniqueId(std::to_string(startTimeMillis_));
```

改为：
```cpp
launchData.SetAppRunningUniqueId(std::to_string(appRunningUniqueId_));
```

**Step 3: 验证修改编译**

运行: 检查修改是否编译正确
预期: 无编译错误

**Step 4: 提交**

```bash
git add services/appmgr/src/app_running_record.cpp
git commit -m "feat: 更新 LaunchApplication 使用 appRunningUniqueId_"
```

---

## Task 7: 更新 AppRunningManager GetAppRunningUniqueIdByPid()

**文件:**
- 修改: `services/appmgr/src/app_running_manager.cpp:1951-1962`

**Step 1: 检查当前实现**

查看第 1951-1962 行的 GetAppRunningUniqueIdByPid() 方法实现

**Step 2: 更新为使用新的 Getter 方法**

将：
```cpp
appRunningUniqueId = std::to_string(appRecord->GetAppStartTime());
```

改为：
```cpp
appRunningUniqueId = std::to_string(appRecord->GetAppRunningUniqueId());
```

**Step 3: 验证修改编译**

运行: 检查修改是否是否编译正确
预期: 无编译错误

**Step 4: 提交**

```bash
git add services/appmgr/src/app_running_manager.cpp
git commit -m "feat: 更新 GetAppRunningUniqueIdByPid 使用新的 Getter 方法"
```

---

## Task 8: 构建并验证更改

**文件:**
- 构建目标: `ability_runtime`

**Step 1: 构建 ability_runtime 组件**

运行:
```bash
./build.sh --product-name <product> --build-target ability_runtime
```

预期: 构建成功，无错误

**Step 2: 检查编译警告**

运行: 查看构建输出中的警告
预期: 无类型转换或缺失包含相关的警告

**Step 3: 提交**

```bash
git commit --allow-empty -m "test: 验证 HiTraceId RunningID 更改构建成功"
```

---

## Task 9: 运行单元测试

**文件:**
- 测试目标: `ability_runtime_test`

**Step 1: 构建并运行单元测试**

运行:
```bash
./build.sh --product-name <product> --build-target ability_runtime_test
run -t UT -tp ability_runtime
```

预期: 所有测试通过

**Step 2: 运行 AppRunningRecord 相关测试**

运行:
```bash
run -t UT -ts app_running_record_test
```

预期: 所有测试通过

**Step 3: 提交**

```bash
git commit --allow-empty -m "test: 验证所有单元测试通过"
```

---

## Task 10: 添加文档注释

**文件:**
- 修改: `services/appmgr/include/app_running_record.h`

**Step 1: 为 GenerateRunningIdFromHitrace 添加注释**

在方法声明前添加：
```cpp
/**
 * @brief 从 HiTraceId 生成全局唯一的运行 ID
 * @return uint64_t 组合 chainId 和 spanId 的唯一运行 ID
 *
 * 该方法通过组合 HiTraceId 的 chainId 和 spanId 生成全局唯一的运行 ID。
 * 如果 HiTraceId 不可用，则回退到随机数生成以确保向后兼容性。
 */
uint64_t GenerateRunningIdFromHitrace() const;
```

**Step 2: 为 GetAppRunningUniqueId 添加注释**

在方法声明前添加：
```cpp
/**
 * @brief 获取应用的唯一运行 ID
 * @return uint64_t 从 HiTraceId 生成的唯一运行 ID
 *
 * 注意：该方法返回的是基于 HiTraceId 生成的全局唯一 ID，
 * 用于在分布式场景中唯一标识应用运行实例。
 */
uint64_t GetAppRunningUniqueId() const;
```

**Step 3: 为成员变量添加注释**

更新成员变量声明：
```cpp
    int64_t startTimeMillis_ = 0;   // 应用启动时间戳 (CLOCK_MONOTONIC)
    uint64_t appRunningUniqueId_ = 0; // 从 HiTraceId 生成的唯一运行 ID
```

**Step 4: 提交**

```bash
git add services/appmgr/include/app_running_record.h
git commit -m "docs: 添加 RunningID 生成方法的文档注释"
```

---

## Task 11: 最终验证和测试

**文件:**
- 所有修改的文件

**Step 1: 最终构建**

运行:
```bash
./build.sh --product-name <product> --build-target ability_runtime --clean
```

预期: 清理构建成功

**Step 2: 运行所有测试**

运行:
```bash
run -t UT -tp ability_runtime
```

预期: 所有测试通过

**Step 3: 验证 git 状态**

运行:
```bash
git status
```

预期: 所有更改已提交

**Step 4: 创建总结提交**

```bash
git commit --allow-empty -m "feat: 为 AppRunningRecord 实现基于 HiTraceId 的 RunningID

- 新增 appRunningUniqueId_ 成员变量存储基于 HiTraceId 的唯一 ID
- 保持原有的 startTimeMillis_ 不变，继续存储时间戳
- 使用 HiTraceId (chainId + spanId) 生成全局唯一 ID
- 当 HiTraceId 不可用时回退到随机数生成
- 更新 LaunchApplication 和 GetAppRunningUniqueIdByPid 使用新 ID

Closes #14528"
```

---

## 更改总结

### 修改的文件:
1. `services/appmgr/include/app_running_record.h`
   - 新增 `appRunningUniqueId_` 成员变量（uint64_t）
   - 新增 `GenerateRunningIdFromHitrace()` 私有方法声明
   - 新增 `GetAppRunningUniqueId()` 公共方法声明

2. `services/appmgr/src/app_running_record.cpp`
   - 添加 `#include "hitrace/hitracechain.h"`
   - 添加 `#include <random>` 和 `#include <limits>`
   - 实现 `GenerateRunningIdFromHitrace()` 方法
   - 实现 `GetAppRunningUniqueId()` 方法
   - 在构造函数中初始化 `appRunningUniqueId_`
   - 更新 `LaunchApplication()` 使用 `appRunningUniqueId_`

3. `services/appmgr/src/app_running_manager.cpp`
   - 更新 `GetAppRunningUniqueIdByPid()` 使用新的 Getter 方法

### 关键特性:
- **保持兼容性**: 原有的 `startTimeMillis_` 保持不变
- **全局唯一性**: 使用 HiTraceId (chainId +) 生成全局唯一 ID
- **回退机制**: HiTraceId 不可用时使用随机数生成
- **类型安全**: 使用 `uint64_t` 表示无符号 ID
- **条件编译**: 遵循 `ABILITY_RUNTIME_HITRACE_ENABLE` 标志

### 测试:
- 构建验证
- 单元测试 (ability_runtime_test)
- AppRunningRecord 相关测试

### 设计说明:
- `startTimeMillis_`: 继续存储应用启动时间戳，用于时间相关逻辑
- `appRunningUniqueId_`: 新增成员变量，存储基于 HiTraceId 的全局唯一 ID，用于分布式场景下的唯一标识
- 两者分离，各司其职，互不影响
