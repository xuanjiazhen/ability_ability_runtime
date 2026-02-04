# FA模型动态加载实现 - AbilityThread层面

## 1. 架构概述

本次实现从 **AbilityThread 层面**实现 FA 模型的动态加载，根据用户反馈：
> "abilityImplFactory整个都是属于fa模型的，动态加载应该从abilityThread创建FaAbilityThread开始"

### 关键设计点

1. **动态加载入口点**：`AbilityThread::AbilityThreadMain` 函数
2. **FA模型判断**：根据 `AbilityInfo::type` 和 `isStageBasedModel` 判断
3. **动态加载方式**：使用 `dlopen` 加载 `libabilitykit_fa.so`

## 2. 代码修改清单

### 2.1 新增文件

#### fa_ability_loader.h/cpp
- **路径**: `frameworks/native/ability/native/fa_ability_loader.{h,cpp}`
- **功能**: FA能力动态加载器，负责加载FA库并创建FAAbilityThread
- **关键函数**:
  ```cpp
  static sptr<AbilityThread> LoadFAAbilityThread(const std::string& libraryPath);
  static void UnloadFALibrary();
  ```
- **实现细节**:
  - 使用 `dlopen("libabilitykit_fa.so", RTLD_NOW | RTLD_LOCAL)` 加载库
  - 使用 `dlsym` 获取 `CreateFAAbilityThread` 函数指针
  - 使用 `RTLD_LOCAL` 避免符号污染

#### fa_ability_register.h/cpp
- **路径**: `frameworks/native/ability/native/fa_ability_register.{h,cpp}`
- **功能**: 导出 FAAbilityThread 创建函数，供 dlsym 调用
- **导出函数**:
  ```cpp
  extern "C" sptr<AbilityThread> CreateFAAbilityThread();
  ```
- **实现**: 直接调用 `sptr<AbilityRuntime::FAAbilityThread>::MakeSptr()`

### 2.2 修改文件

#### ability_thread.cpp
- **路径**: `frameworks/native/ability/native/ability_thread.cpp`
- **修改内容**:
  1. 移除 `#include "fa_ability_thread.h"`
  2. 添加 `#include "fa_ability_loader.h"`
  3. 修改 FAAbilityThread 创建逻辑：
     ```cpp
     // 之前：
     thread = sptr<AbilityRuntime::FAAbilityThread>::MakeSptr();

     // 修改后：
     TAG_LOGI(AAFwkTag::ABILITY, "Creating FA Ability, loading FA library dynamically");
     thread = FAAbilityLoader::LoadFAAbilityThread();
     ```

#### BUILD.gn
- **路径**: `frameworks/native/ability/native/BUILD.gn`
- **主要修改**:

  **abilitykit_native 库（主库，Stage模型）**:
  - 移除文件：
    - `ability_impl_factory.cpp`（FA模型专用工厂）
    - `fa_ability_thread.cpp`（FA Ability线程）
    - `page_ability_impl.cpp`（PageAbility实现，FA模型）
    - `service_ability_impl.cpp`（ServiceAbility实现，FA模型）
    - `data_ability_impl.cpp`（DataAbility实现，FA模型）
    - `data_ability_helper_impl.cpp`（DataAbility辅助，FA模型）
    - `continuation/distributed/continuation_handler.cpp`（FA模型continuation）
    - `continuation/distributed/continuation_manager.cpp`（FA模型continuation）
    - `continuation/distributed/reverse_continuation_scheduler_primary.cpp`（FA模型continuation）
  - 新增文件：
    - `fa_ability_loader.cpp`（动态加载器）

  **abilitykit_fa 库（新增，FA模型专用）**:
  - 包含所有FA模型特有文件：
    - `page_ability_impl.cpp`
    - `service_ability_impl.cpp`
    - `data_ability_impl.cpp`
    - `data_ability_helper.cpp`
    - `data_ability_helper_impl.cpp`
    - `fa_ability_thread.cpp`
    - `fa_ability_register.cpp`
    - `ability_impl_factory.cpp`
    - FA模型continuation相关文件
    - FA模型辅助类（dummy_values_bucket, data_ability_operation等）
  - 依赖：
    - `continuation_ipc`
    - `ability_manager`
    - `runtime`
    - `abilitykit_utils`
    - `appkit_native`

## 3. 工作流程

### 3.1 Stage模型应用启动流程
```
App Start
  → AbilityThreadMain()
    → 检查: abilityInfo->type == AbilityType::PAGE && isStageBasedModel
    → 创建: UIAbilityThread（直接创建，无需加载FA库）
    → 内存: 不加载 libabilitykit_fa.so
```

### 3.2 FA模型应用启动流程
```
App Start
  → AbilityThreadMain()
    → 检查: 非 (PAGE + StageBasedModel) 且非 EXTENSION
    → 调用: FAAbilityLoader::LoadFAAbilityThread()
      → dlopen("libabilitykit_fa.so")
      → dlsym("CreateFAAbilityThread")
      → 调用创建函数
    → 返回: FAAbilityThread
    → 内存: 加载 libabilitykit_fa.so（仅FA应用需要）
```

## 4. 内存收益分析

### 4.1 预估收益
- **单进程节省**: ~1MB
  - FA模型代码段和数据段不加载到内存
  - FA模型相关的continuation、data_ability等模块不加载

- **系统级别节省**: ~20-50MB
  - 假设20-50个Stage模型应用进程
  - 每个进程节省约1MB

### 4.2 验证方法
```bash
# Stage模型应用进程
cat /proc/<pid>/maps | grep abilitykit_fa  # 应该为空

# FA模型应用进程
cat /proc/<pid>/maps | grep abilitykit_fa  # 应该看到libabilitykit_fa.so
```

## 5. 关键技术点

### 5.1 符号隔离
- 使用 `RTLD_LOCAL` 标志避免FA库符号污染主库命名空间
- FA库内部符号对其他库不可见

### 5.2 错误处理
- dlopen失败时返回nullptr
- dlsym失败时关闭库句柄并返回nullptr
- 创建失败时清理资源

### 5.3 生命周期管理
- FA库句柄在进程内缓存（faLibraryHandle_静态变量）
- 多个FA Ability共享同一个库句柄
- 进程退出时自动释放

## 6. 与之前方案的区别

### 之前方案（AbilityImplFactory层面）
- ❌ 错误假设：AbilityImplFactory是FA/Stage共享的
- ❌ 在Factory层面实现动态加载
- ❌ 需要修改Factory内部逻辑

### 当前方案（AbilityThread层面）
- ✅ 正确理解：AbilityImplFactory整个属于FA模型
- ✅ 在AbilityThread创建时判断模型类型
- ✅ FA模型库完全独立，按需加载

## 7. 测试验证

### 7.1 编译验证
```bash
./build.sh --product-name <product> --build-target ability_runtime
./build.sh --product-name <product> --build-target abilitykit_fa
```

### 7.2 功能验证
```bash
# 启动Stage模型应用
aa start -a <stage-ability> -b <stage-bundle>
# 验证：正常运行，不加载libabilitykit_fa.so

# 启动FA模型应用
aa start -a <fa-ability> -b <fa-bundle>
# 验证：正常运行，动态加载libabilitykit_fa.so
```

### 7.3 内存验证
```bash
# 检查Stage应用内存映射
dumpsys meminfo <stage-package> | grep abilitykit

# 检查FA应用内存映射
dumpsys meminfo <fa-package> | grep abilitykit
```

## 8. 后续优化

1. **懒加载优化**: 可以考虑在第一次真正使用FA功能时才加载库
2. **库预加载**: 对于频繁使用FA的应用场景，可以提供预加载选项
3. **卸载机制**: 实现FA库的主动卸载机制，释放内存
4. **监控指标**: 添加动态加载耗时、成功率等监控指标

## 9. 注意事项

1. **ABI兼容性**: FA库接口必须保持稳定，避免破坏兼容性
2. **线程安全**: dlopen/dlsym在多线程环境下的安全性
3. **错误恢复**: FA库加载失败时的降级处理
4. **调试支持**: 保留FA库的调试符号便于问题定位

## 10. 文件清单

```
新增文件:
├── frameworks/native/ability/native/fa_ability_loader.h
├── frameworks/native/ability/native/fa_ability_loader.cpp
└── frameworks/native/ability/native/fa_ability_register.h

修改文件:
├── frameworks/native/ability/native/ability_thread.cpp
├── frameworks/native/ability/native/fa_ability_register.cpp
└── frameworks/native/ability/native/BUILD.gn

生成文件:
└── FA_MODEL_DYNAMIC_LOADING_ABILITYTHREAD.patch
```
