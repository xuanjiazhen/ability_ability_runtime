# AppServiceExtension TYPE 模式支持设计文档

**Issue**: #14623
**日期**: 2026-03-02
**作者**: Claude Sonnet + zhang_hao_zheng

## 概述

为 `AppServiceExtensionAbility` 添加对 `ExtensionProcessMode::TYPE` 模式的支持，使每个 `AppServiceExtensionAbility` 能够运行在独立的进程中。

## 背景

当前 `ExtensionProcessMode` 枚举定义了四种模式：
- `INSTANCE = 0` - 每个实例一个进程
- `TYPE = 1` - 按类型独立进程
- `BUNDLE = 2` - 默认，按 bundle 共享进程
- `RUN_WITH_MAIN_PROCESS = 3` - 与主进程共享

然而，`AppServiceExtension` 当前未在 `EXTENSION_RECORD_CONFIG_MAP` 中配置，不支持 `TYPE` 模式。

## 需求

1. 当 `AppServiceExtension` 的 `ExtensionProcessMode` 设置为 `TYPE` 时
2. 每个 `AppServiceExtensionAbility` 运行在独立的进程中
3. 进程名称格式：`bundleName:abilityName`
4. 不同的 `AppServiceExtensionAbility` 运行在不同进程中
5. 保证现有功能的兼容性，不破坏现有行为

## 架构设计

### 核心改动点

1. **在 `extension_record_factory.cpp` 中添加配置**
   - 为 `ExtensionAbilityType::APP_SERVICE` 添加 `EXTENSION_RECORD_CONFIG_MAP` 条目
   - 配置 `processModeSupport` 包含 `PROCESS_MODE_SUPPORT_DEFAULT | PROCESS_MODE_TYPE`
   - 利用现有的 `TYPE` 模式处理逻辑

2. **进程名称生成**
   - 利用现有的 `MakeProcessName` 函数
   - 当 `extensionProcessMode` 为 `TYPE` 时，进程名称格式应为 `bundleName:abilityName`
   - 在 `MakeProcessName` 中为 `APP_SERVICE` 类型添加特殊逻辑

3. **无需新建 Factory 子类**
   - 直接使用现有的 `ExtensionRecordFactory`
   - `GetExtensionProcessMode()` 方法已正确处理配置映射

## 数据流

### 1. 配置阶段（module.json5）
- 开发者在 `AppServiceExtensionAbility` 的配置中设置 `extensionProcessMode` 为 `type`
- BundleManager 解析并设置到 `AbilityInfo.extensionProcessMode`

### 2. 启动阶段
- 客户端调用 `connectAbility()` 请求连接
- `AbilityConnectManager` 创建 `AbilityRequest`
- `ExtensionRecordFactory::CreateRecord()` 被调用
- `GetExtensionProcessMode()` 读取配置，识别出 `PROCESS_MODE_TYPE`

### 3. 进程创建阶段
- `AbilityConnectManager::LoadAbility()` 构建 `LoadParam`
- `loadParam.extensionProcessMode` 被设置为 `PROCESS_MODE_TYPE`
- `AppMgrServiceInner::MakeProcessName()` 被调用
- 根据配置生成进程名 `bundleName:abilityName`

### 4. 进程匹配阶段
- `AppRunningManager` 检查进程是否已存在
- 不同 `abilityName` 对应不同进程，需要独立创建

## 文件修改清单

### 1. `services/abilitymgr/src/extension_record/extension_record_factory.cpp`

在 `EXTENSION_RECORD_CONFIG_MAP` 中添加 `APP_SERVICE` 配置项：

```cpp
{ AppExecFwk::ExtensionAbilityType::SERVICE,
  { PROCESS_MODE_BUNDLE, PROCESS_MODE_SUPPORT_DEFAULT | PROCESS_MODE_TYPE,
    PRE_CHECK_FLAG_NONE }}
```

### 2. `services/appmgr/src/app_mgr_service_inner.cpp`

在 `MakeProcessName()` 函数中添加对 `APP_SERVICE` 类型 `TYPE` 模式的处理：
- 读取 `extensionProcessMode` 参数
- 生成格式为 `bundleName:abilityName` 的进程名

### 3. 可能需要确认的文件
- `services/abilitymgr/src/ability_connect_manager.cpp` - 检查如何传递 `extensionProcessMode`
- `interfaces/kits/native/.../ability_info.h` - 确认枚举定义

## 边界情况和错误处理

### 1. 进程名称冲突
- 当多个 `AppServiceExtensionAbility` 同时启动时，需要确保每个都有唯一进程名
- 复用现有的 `appIndex` 机制：`bundleName:abilityName:index`
- 仅在确实需要时添加 index 后缀

### 2. 无效的进程模式配置
- 如果开发者配置了不支持的 `extensionProcessMode` 值
- `GetExtensionProcessMode()` 应返回默认值 `PROCESS_MODE_BUNDLE`
- 添加日志警告提示配置无效

### 3. 从 BUNDLE 切换到 TYPE 的兼容性
- 已有应用使用 BUNDLE 模式运行
- 新代码不应破坏现有行为
- 只有显式配置 `type` 时才使用 TYPE 模式

### 4. 进程数限制
- 考虑系统对进程数量的限制
- 复用 `GetLimitMaximumExtensionsPerDevice()` 检查
- 超过限制时返回适当错误码

## 测试策略

### 1. 单元测试
- 修改 `extension_record_factory_test.cpp`
- 验证 `APP_SERVICE` 配置项正确注册
- 测试 `GetExtensionProcessMode()` 返回正确的模式值
- 验证无效模式回退到 BUNDLE

### 2. 集成测试
- 修改 `app_mgr_service_inner_test.cpp`
- 测试 `MakeProcessName()` 生成正确的进程名格式
- 验证进程名冲突处理逻辑
- 测试多实例场景

### 3. 兼容性测试
- 现有 `APP_SERVICE` 应用（无 `extensionProcessMode` 配置）继续用 BUNDLE 模式
- 显式配置 `type` 的应用使用独立进程
- 确保不破坏其他 Extension 类型

### 4. Demo 验证
- 创建测试 Demo 应用
- 配置多个 `AppServiceExtensionAbility`，`extensionProcessMode` 为 `type`
- 使用 `aa dump -i` 或 `ps` 验证进程名格式
- 确认不同 Ability 运行在不同进程中

## 成功标准

- [ ] `AppServiceExtension` 支持 `ExtensionProcessMode::TYPE` 配置
- [ ] 进程名称格式为 `bundleName:abilityName`
- [ ] 不同 `AppServiceExtensionAbility` 运行在不同进程中
- [ ] 所有单元测试通过
- [ ] Demo 验证成功
- [ ] 现有功能无回归
