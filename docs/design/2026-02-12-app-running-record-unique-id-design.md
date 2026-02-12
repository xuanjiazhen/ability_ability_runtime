# AppRunningRecord 全局唯一 RunningID 需求开发方案

## 1. 需求分析

### 1.1 需求背景

**Issue:** #14528 - AppRunningRecord 使用全局唯一生成的 RunningID

**问题描述:**
- DFX 场景使用 RunningID 关联进程的各种执行记录
- 当前 RunningID 是进程启动时的时间戳，不同设备可能产生重复
- 需要实现全局唯一的 RunningID 生成机制

### 1.2 需求目标

1. **全局唯一性**: RunningID 在分布式环境下全局唯一
2. **兼容性**: 保持现有功能不变，不破坏现有逻辑
3. **性能**: ID 生成过程高效，不影响应用启动性能
4. **可测试性**: 提供完整的单元测试覆盖

### 1.3 功能要求

- 新增 `appRunningUniqueId_` 成员变量存储全局唯一 ID
- 保持 `startTimeMillis_` 不变，继续存储时间戳
- 使用时间戳+随机数组合生成唯一 ID
- 更新所有使用 RunningID 的地方

### 1.4 非功能要求

- **性能**: ID 生成时间 < 1ms
- **可靠性**: 冲突概率 < 2^-32
- **可维护性**: 代码清晰，注释完整
- **可测试性**: 单元测试覆盖率 > 90%

---

## 2. 设计方案

### 2.1 架构设计

#### 2.1.1 整体架构

```
AppRunningRecord
├── startTimeMillis_ (int64_t)        // 原有：应用启动时间戳
├── appRunningUniqueId_ (uint64_t)     // 新增：全局唯一 ID
├── GenerateRunningId()                 // 新增：生成唯一 ID 的私有方法
└── GetAppRunningUniqueId()             // 新增：获取唯一 ID 的公共方法
```

#### 2.1.2 ID 生成算法

**算法选择**: 时间戳（高32位）+ 随机数（低32位）

**理由**:
1. 时间戳提供时间维度唯一性
2. 随机数解决同一毫秒内多次启动的冲突
3. 冲突概率极低（约 2^-32）
4. 性能优秀，无需系统调用

**实现**:
```cpp
uint64_t GenerateRunningId() const {
    // 获取 CLOCK_MONOTONIC 时间戳（毫秒）
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    uint64_t timestamp = static_cast<uint64_t>(
        ((t.tv_sec) * NANOSECONDS + t.tv_nsec) / MICROSECONDS
    );
    
    // 生成 32 位随机数
    std::random_device rd;
    std::mt19937_64 engine(rd());
    std::uniform_int_distribution<uint64_t> dist(0, 0xFFFFFFFF);
    uint64_t random = dist(engine);
    
    // 组合：高32位时间戳，低32位随机数
    return (timestamp << 32) | random;
}
```

### 2.2 类设计

#### 2.2.1 AppRunningRecord 类修改

**新增成员变量**:
```cpp
private:
    uint64_t appRunningUniqueId_ = 0; // 全局唯一运行 ID
```

**新增方法**:
```cpp
private:
    uint64_t GenerateRunningId() const;

public:
    uint64_t GetAppRunningUniqueId() const;
```

#### 2.2.2 调用链路

```
AppRunningRecord 构造函数
    ↓
GenerateRunningId()
    ↓
appRunningUniqueId_ = GenerateRunningId()
    ↓
LaunchApplication()
    ↓
launchData.SetAppRunningUniqueId(std::to_string(appRunningUniqueId_))
```

### 2.3 兼容性设计

#### 2.3.1 保持向后兼容

- `startTimeMillis_` 保持不变，继续存储时间戳
- `GetAppStartTime()` 方法保持不变
- 新增 `GetAppRunningUniqueId()` 方法提供新的唯一 ID

#### 2.3.2 迁移策略

- `LaunchApplication()` 方法使用新的 `appRunningUniqueId_`
- `GetAppRunningUniqueIdByPid()` 方法使用新的 Getter
- 其他使用 `GetAppStartTime()` 的地方保持不变

### 2.4 错误处理

#### 2.4.1 异常情况处理

| 异常情况 | 处理方式 |
|---------|---------|
| clock_gettime 失败 | 使用默认值 0，记录错误日志 |
| 随机数生成失败 | 使用时间戳作为 ID，记录错误日志 |
| ID 重复 | 理论上不可能，不处理 |

#### 2.4.2 日志记录

```cpp
TAG_LOGI(AAFwkTag::APPMGR, "GenerateRunningId: %{public}lu", appRunningUniqueId_);
```

---

## 3. 测试方案

### 3.1 测试策略

#### 3.1.1 测试金字塔

```
    /\
   /  \      单元测试 (70%)
  /____\
 /      \    集成测试 (20%)
/________\
/          \  系统测试 (10%)
```

#### 3.1.2 测试覆盖

- **单元测试**: 覆盖所有新增和修改的方法
- **集成测试**: 验证调用链路正确性
- **回归测试**: 确保不破坏现有功能

### 3.2 单元测试设计

#### 3.2.1 AppRunningRecord 测试

**测试文件**: `test/unittest/app_running_record_test/`

**测试用例**:

| 测试用例 | 描述 | 预期结果 |
|---------|------|---------|
| `test_GenerateRunningId_Uniqueness` | 验证生成的 ID 唯一性 | 连续生成 1000 个 ID，无重复 |
| `test_GenerateRunningId_Format` | 验证 ID 格式正确性 | ID 为 uint64_t，非零 |
| `test_GetAppRunningUniqueId` | 验证 Getter 方法正确性 | 返回值与成员变量一致 |
| `test_Constructor_GeneratesUniqueId` | 验证构造函数生成 ID | 构造后 appRunningUniqueId_ 非零 |
| `test_UniqueId_Persistence` | 验证 ID 在生命周期内不变 | 多次调用 GetAppRunningUniqueId() 返回相同值 |

**测试代码示例**:

```cpp
HWTEST_F(AppRunningRecordTest, test_GenerateRunningId_Uniqueness, TestSize.Level1)
{
    std::set<uint64_t> uniqueIds;
    const int iterationCount = 1000;
    
    for (int i = 0; i < iterationCount; i++) {
        auto record = CreateAppRunningRecord();
        uint64_t uniqueId = record->GetAppRunningUniqueId();
        uniqueIds.insert(uniqueId);
    }
    
    EXPECT_EQ(uniqueIds.size(), iterationCount);
}

HWTEST_F(AppRunningRecordTest, test_GetAppRunningUniqueId, TestSize.Level1)
{
    auto record = CreateAppRunningRecord();
    uint64_t uniqueId = record->GetAppRunningUniqueId();
    
    EXPECT_NE(uniqueId, 0);
    EXPECT_EQ(record->GetAppRunningUniqueId(), uniqueId);
}

HWTEST_F(AppRunningRecordTest, test_UniqueId_Persistence, TestSize.Level1)
{
    auto record = CreateAppRunningRecord();
    uint64_t firstCall = record->GetAppRunningUniqueId();
    uint64_t secondCall = record->GetAppRunningUniqueId();
    uint64_t thirdCall = record->GetAppRunningUniqueId();
    
    EXPECT_EQ(firstCall, secondCall);
    EXPECT_EQ(secondCall, thirdCall);
}
```

#### 3.2.2 AppRunningManager 测试

**测试文件**: `test/unittest/app_running_manager_test/`

**测试用例**:

| 测试用例 | 描述 | 预期结果 |
|---------|------|---------|
| `test_GetAppRunningUniqueIdByPid` | 验证通过 PID 获取唯一 ID | 返回正确的唯一 ID |
| `test_GetAppRunningUniqueIdByPid_NotFound` | 验证 PID 不存在的情况 | 返回空字符串 |

**测试代码示例**:

```cpp
HWTEST_F(AppRunningManagerTest, test_GetAppRunningUniqueIdByPid, TestSize.Level1)
{
    auto record = CreateAppRunningRecord();
    pid_t pid = record->GetPid();
    
    std::string uniqueId = appRunningManager_->GetAppRunningUniqueIdByPid(pid);
    
    EXPECT_FALSE(uniqueId.empty());
    EXPECT_EQ(std::stoull(uniqueId), record->GetAppRunningUniqueId());
}
```

### 3.3 集成测试设计

#### 3.3.1 LaunchApplication 集成测试

**测试用例**: `test_LaunchApplication_UsesUniqueId`

**描述**: 验证 LaunchApplication 方法正确使用 appRunningUniqueId_

**测试代码**:

```cpp
HWTEST_F(AppRunningRecordTest, test_LaunchApplication_UsesUniqueId, TestSize.Level1)
{
    auto record = CreateAppRunningRecord();
    uint64_t expectedUniqueId = record->GetAppRunningUniqueId();
    
    // Mock LaunchApplication 方法
    AppLaunchData launchData;
    EXPECT_CALL(*mock_, LaunchApplication(_, _))
        .WillOnce(DoAll(SaveArg<0>(&launchData), Return(ERR_OK)));
    
    record->LaunchApplication(want, launchData);
    
    std::string actualUniqueId = launchData.GetAppRunningUniqueId();
    EXPECT_EQ(std::stoull(actualUniqueId), expectedUniqueId);
}
```

### 3.4 性能测试

#### 3.4.1 ID 生成性能测试

**测试用例**: `test_GenerateRunningId_Performance`

**描述**: 验证 ID 生成性能满足要求

**测试代码**:

```cpp
HWTEST_F(AppRunningRecordTest, test_GenerateRunningId_Performance, TestSize.Level1)
{
    auto record = CreateAppRunningRecord();
    const int iterationCount = 10000;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterationCount; i++) {
        uint64_t uniqueId = record->GetAppRunningUniqueId();
        (void)uniqueId; // 避免编译器优化
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        endTime - startTime
    ).count();
    
    double avgTime = static_cast<double>(duration) / iterationCount;
    EXPECT_LT(avgTime, 1000.0); // 平均时间 < 1ms
}
```

### 3.5 回归测试

#### 3.5.1 现有功能回归测试

**测试范围**:
- 所有 AppRunningRecord 现有测试用例
- 所有 AppRunningManager 现有测试用例
- 所有使用 startTimeMillis_ 的测试用例

**验证点**:
- `GetAppStartTime()` 返回值不变
- 现有功能不受影响
- 无性能退化

---

## 4. 实现计划

### 4.1 任务分解

| 任务 | 描述 | 优先级 |
|------|------|--------|
| Task 1 | 添加随机数生成所需的头文件 | 高 |
| Task 2 | 在头文件中添加新的成员变量声明 | 高 |
| Task 3 | 添加生成唯一 ID 的私有方法 | 高 |
| Task 4 | 实现 GetAppRunningUniqueId 方法 | 高 |
| Task 5 | 在构造函数中初始化 appRunningUniqueId_ | 高 |
| Task 6 | 更新 LaunchApplication 方法使用新的唯一 ID | 高 |
| Task 7 | 更新 AppRunningManager GetAppRunningUniqueIdByPid() | 高 |
| Task 8 | 编写单元测试 | 高 |
| Task 9 | 运行测试并修复问题 | 高 |
| Task 10 | 代码审查和优化 | 中 |
| Task 11 | 添加文档注释 | 中 |
| Task 12 | 最终验证和测试 | 高 |

### 4.2 开发流程

```
1. 理解需求 ✓
    ↓
2. 设计方案 ✓
    ↓
3. 编写测试用例 (TDD)
    ↓
4. 实现功能代码
    ↓
5. 运行测试
    ↓
6. 代码审查
    ↓
7. 修复问题
    ↓
8. 提交代码
```

---

## 5. 代码规范

### 5.1 命名规范

| 类型 | 命名规则 | 示例 |
|------|---------|------|
| 成员变量 | 小写下划线 | `appRunningUniqueId_` |
| 方法 | 大驼峰 | `GetAppRunningUniqueId()` |
| 私有方法 | 大驼峰 | `GenerateRunningId()` |
| 常量 | 大写下划线 | `NANOSECONDS` |

### 5.2 注释规范

#### 5.2.1 成员变量注释

```cpp
uint64_t appRunningUniqueId_ = 0; // 全局唯一运行 ID（时间戳+随机数）
```

#### 5.2.2 方法注释

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

### 5.3 错误处理规范

- 使用 `TAG_LOGE` 记录错误
- 关键路径必须有错误处理
- 不要吞掉异常

### 5.4 性能规范

- 避免在构造函数中进行耗时操作
- 使用高效的随机数生成器
- 避免不必要的内存分配

---

## 6. 代码审查清单

### 6.1 功能审查

- [ ] ID 生成算法正确
- [ ] 唯一性保证
- [ ] 兼容性保证
- [ ] 错误处理完善

### 6.2 性能审查

- [ ] ID 生成时间 < 1ms
- [ ] 无内存泄漏
- [ ] 无不必要的拷贝

### 6.3 代码质量审查

- [ ] 命名规范
- [ ] 注释完整
- [ ] 代码风格一致
- [ ] 无编译警告

### 6.4 测试审查

- [ ] 单元测试覆盖率 > 90%
- [ ] 所有测试用例通过
- [ ] 边界条件测试
- [ ] 性能测试通过

---

## 7. 风险评估

| 风险 | 可能性 | 影响 | 缓解措施 |
|------|--------|------|---------|
| ID 冲突 | 极低 | 高 | 使用时间戳+随机数，冲突概率极低 |
| 性能退化 | 低 | 中 | 添加性能测试，监控性能指标 |
| 兼容性问题 | 低 | 高 | 保持原有 API 不变，添加新 API |
| 测试覆盖不足 | 中 | 中 | 严格遵循 TDD，确保测试覆盖率 |

---

## 8. 交付标准

### 8.1 代码交付

- [ ] 所有代码通过编译
- [ ] 无编译警告
- [ ] 代码风格符合规范
- [ ] 注释完整

### 8.2 测试交付

- [ ] 单元测试覆盖率 > 90%
- [ ] 所有测试用例通过
- [ ] 性能测试通过
- [ ] 回归测试通过

### 8.3 文档交付

- [ ] 代码注释完整
- [ ] 设计文档完整
- [ ] 测试文档完整

---

## 9. 后续优化

### 9.1 短期优化

- 添加 ID 生成统计信息
- 优化随机数生成器性能
- 添加 ID 冲突检测

### 9.2 长期优化

- 考虑使用 HiTraceId 替代
- 添加分布式 ID 生成支持
- 实现 ID 池管理

---

## 附录

### A. 参考资料

- [OpenHarmony DFX 文档](https://docs.openharmony.cn/)
- [C++ 随机数生成最佳实践](https://en.cppreference.com/w/cpp/numeric/random)
- [TDD 开发指南](https://github.com/testng/testng)

### B. 相关 Issue

- #14528 - AppRunningRecord 使用全局唯一生成的 RunningID

### C. 变更记录

| 版本 | 日期 | 变更内容 | 作者 |
|------|------|---------|------|
| 1.0 | 2026-02-12 | 初始版本 | Claude |
