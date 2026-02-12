# AppRunningRecord 时间戳+随机数 RunningID 实现方案

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans 来实现这个计划。

**目标:** 为 AppRunningRecord 新增基于时间戳+随机数的全局唯一 RunningID，保持原有的 startTimeMillis_ 不变

**架构:** 
- 新增成员变量 `appRunningUniqueId_` 存储基于时间戳+随机数的唯一 ID
- 原有的 `startTimeMillis_` 继续存储时间戳
- 使用 CLOCK_MONOTONIC 时间戳（高32位）+ 随机数（低32位）组合生成唯一 ID

**技术栈:** C++, OpenHarmony ability_runtime 框架

---

## Task 1: 添加随机数生成所需的头文件

**文件:**
- 修改: `services/appmgr/src/app_running_record.cpp:27`

**Step 1: 添加头文件包含**

在现有的头文件包含后（约第 27 行附近）添加以下包含：

```cpp
#include <random>
#include <limits>
```

**Step 2: 验证文件编译**

运行: 检查文件是否有正确的包含
预期: 无编译错误

**Step 3: 提交**

```bash
git add services/appmgr/src/app_running_record.cpp
git commit -m "feat: 添加随机数生成所需的头文件"
```

---

## Task 2: 在头文件中添加新的成员变量声明

**文件:**
- 修改: `services/appmgr/include/app_running_record.h:1329`

**Step 1: 在 startTimeMillis_ 后添加新成员变量**

在第 1329 行的 startTimeMillis_ 声明后添加：

```cpp
    int64_t startTimeMillis_ = 0;   // The time of app start(CLOCK_MONOTONIC)
    uint64_t appRunningUniqueId_ = 0; // The unique running ID generated from timestamp + random
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
uint64_t GenerateRunningId() const;
```

**Step 2: 在 cpp 文件中实现该方法**

在 cpp 文件中添加实现（约第 2174 行附近，在 GetAppStartTime() 后）：

```cpp
uint64_t AppRunningRecord::GenerateRunningId() const
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    uint64_t timestamp = static_cast<uint64_t>(((t.tv_sec) * NANOSECONDS + t.tv_nsec) / MICROSECONDS);
    
    std::random_device rd;
    std::mt19937_64 engine(rd());
    std::uniform_int_distribution<uint64_t> dist(0, 0xFFFFFFFF);
    uint64_t random = dist(engine);
    
    // 高32位使用时间戳，低32位使用随机数，确保全局唯一性
    return (timestamp << 32) | random;
}
```

**Step 3: 验证方法编译**

运行: 检查方法实现是否编译正确
预期: 无编译错误

**Step 4: 提交**

```bash
git add services/appmgr/include/app_running_record.h services/appmgr/src/app_running_record.cpp
git commit -m "feat: 添加 GenerateRunningId 方法实现"
```

---

## Task 4: 实现 GetAppRunningUniqueId 方法

**文件:**
- 修改: `services/appmgr/src/app_running_record.cpp`

**Step 1: 实现 GetAppRunningUniqueId 方法**

在 cpp 文件中添加实现（约第 2174 行附近，在 GenerateRunningId() 后）：

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
    appRunningUniqueId_ = GenerateRunningId();
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

运行: 检查修改是否编译正确
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
git commit --allow-empty -m "test: 验证时间戳+随机数 RunningID 更改构建成功"
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

**Step 1: 为 GenerateRunningId 添加注释**

在方法声明前添加：
```cpp
/**
 * @brief 生成全局唯一的运行 ID
 * @return uint64_t 组合时间戳和随机数的唯一运行 ID
 *
 * 该方法通过组合 CLOCK_MONOTONIC 时间戳（高32位）和随机数（低32位）
 * 生成全局唯一的运行 ID。
 * - 时间戳提供时间维度唯一性
 * - 随机数解决同一毫秒内多次启动的冲突
 * - 冲突概率极低（约 2^-32）
 */
uint64_t GenerateRunningId() const;
```

**Step 2: 为 GetAppRunningUniqueId 添加注释**

在方法声明前添加：
```cpp
/**
 * @brief 获取应用的唯一运行 ID
 * @return uint64_t 从时间戳+随机数生成的唯一运行 ID
 *
 * 注意：该方法返回的是基于时间戳+随机数生成的全局唯一 ID，
 * 用于唯一标识应用运行实例。
 */
uint64_t GetAppRunningUniqueId() const;
```

**Step 3: 为成员变量添加注释**

更新成员变量声明：
```cpp
    int64_t startTimeMillis_ = 0;   // 应用启动时间戳 (CLOCK_MONOTONIC)
    uint64_t appRunningUniqueId_ = 0; // 从时间戳+随机数生成的唯一运行 ID
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
git commit --allow-empty -m "feat: 为 AppRunningRecord 实现基于时间戳+随机数的 RunningID

- 新增 appRunningUniqueId_ 成员变量存储基于时间戳+随机数的唯一 ID
- 保持原有的 startTimeMillis_ 不变，继续存储时间戳
- 使用 CLOCK_MONOTONIC 时间戳（高32位）+ 随机数（低32位）生成全局唯一 ID
- 时间戳提供时间维度唯一性，随机数解决同一毫秒内冲突
- 更新 LaunchApplication 和 GetAppRunningUniqueIdByPid 使用新 ID

Closes #14528"
```

---

## 更改总结

### 修改的文件:
1. `services/appmgr/include/app_running_record.h`
   - 新增 `appRunningUniqueId_` 成员变量（uint64_t）
   - 新增 `GenerateRunningId()` 私有方法声明
   - 新增 `GetAppRunningUniqueId()` 公共方法声明

2. `services/appmgr/src/app_running_record.cpp`
   - 添加 `#include <random>` 和 `#include <limits>`
   - 实现 `GenerateRunningId()` 方法
   - 实现 `GetAppRunningUniqueId()` 方法
   - 在构造函数中初始化 `appRunningUniqueId_`
   - 更新 `LaunchApplication()` 使用 `appRunningUniqueId_`

3. `services/appmgr/src/app_running_manager.cpp`
   - 更新 `GetAppRunningUniqueIdByPid()` 使用新的 Getter 方法

### 关键特性:
- **保持兼容性**: 原有的 `startTimeMillis_` 保持不变
- **全局唯一性**: 使用时间戳（高32位）+ 随机数（低32位）生成全局唯一 ID
- **高可靠性**: 时间戳提供时间维度唯一性，随机数解决同一毫秒内冲突
- **低冲突概率**: 冲突概率约 2^-32，实际应用中几乎不可能发生
- **高性能**: 使用 CLOCK_MONOTONIC 和 std::mt19937_64，性能优秀

### ID 生成算法:
```cpp
uint64_t timestamp = GetMonotonicTime();  // 高32位
uint64_t random = GetRandom32Bits();       // 低32位
return (timestamp << 32) | random;         // 组合
```

### 测试:
- 构建验证
- 单元测试 (ability_runtime_test)
- AppRunningRecord 相关测试

### 设计说明:
- `startTimeMillis_`: 继续存储应用启动时间戳，用于时间相关逻辑
- `appRunningUniqueId_`: 新增成员变量，存储基于时间戳+随机数的全局唯一 ID，用于唯一标识应用运行实例
- 两者分离，各司其职，互不影响
