# FA模型动态加载 - 补丁文件说明

## 生成的补丁文件

**文件名**: `FA_MODEL_DYNAMIC_LOADING_CHANGES.patch`
**大小**: 3769行
**类型**: Git unified diff patch

## 应用补丁的方法

### 方法一：使用 git apply

```bash
# 在ability_runtime根目录执行
git apply FA_MODEL_DYNAMIC_LOADING_CHANGES.patch
```

### 方法二：使用 patch 命令

```bash
# 在ability_runtime根目录执行
patch -p1 < FA_MODEL_DYNAMIC_LOADING_CHANGES.patch
```

## 补丁包含的修改

### 新增文件（6个）

| 文件 | 说明 |
|-----|------|
| frameworks/native/ability/native/ability_model_manager.h | 模型管理器头文件 |
| frameworks/native/ability/native/ability_model_manager.cpp | 模型管理器实现 |
| frameworks/native/ability/native/fa_module_loader.h | FA模块加载器头文件 |
| frameworks/native/ability/native/fa_module_loader.cpp | FA模块加载器实现 |
| frameworks/native/ability/native/fa_ability_register.h | FA能力注册接口 |
| frameworks/native/ability/native/fa_ability_register.cpp | FA能力注册实现 |

### 修改文件（3个）

| 文件 | 修改内容 |
|-----|---------|
| frameworks/native/ability/native/BUILD.gn | 添加FA独立库配置，移除主库FA代码 |
| frameworks/native/ability/native/ability_impl_factory.cpp | 支持FA/Stage模型区分 |
| interfaces/kits/native/ability/native/ability_impl_factory.h | 添加动态注册支持 |

## 主要变更说明

### 1. BUILD.gn修改

**主库 (abilitykit_native)**:
- 新增：`ability_model_manager.cpp`、`fa_module_loader.cpp`
- 移除：`page_ability_impl.cpp`、`service_ability_impl.cpp`、`data_ability_impl.cpp`、`data_ability_helper_impl.cpp`

**新增FA独立库 (abilitykit_fa)**:
- 包含所有FA模型相关实现
- 安装路径：`lib/libabilitykit_fa.so`
- 可被dlopen动态加载

### 2. ability_impl_factory修改

**头文件**：
```cpp
+  std::unordered_map<AbilityType, std::function<std::shared_ptr<AbilityImpl>()>>
+      abilityCreators_;
+
+  void RegisterAbilityCreator(AbilityType type,
+      std::function<std::shared_ptr<AbilityImpl>()> creator);
```

**实现文件**：
```cpp
+  // 检测应用模型类型
+  auto modelManager = DelayedSingleton<AbilityModelManager>::GetInstance();
+  auto modelType = modelManager->DetectApplicationModel(info);
+
+  if (modelType == AbilityModelManager::ApplicationModel::FA_MODEL) {
+      // FA模型：通过AbilityModelManager动态加载并创建
+      return modelManager->CreateFAAbilityImpl(info);
+  }
+  // Stage模型：直接创建...
```

## 回滚补丁

如果需要回滚所有修改：

```bash
git reset --hard HEAD
```

或者只回滚特定文件：
```bash
git checkout HEAD -- frameworks/native/ability/native/ability_model_manager.*
git checkout HEAD -- frameworks/native/ability/native/fa_module_loader.*
git checkout HEAD -- frameworks/native/ability/native/fa_ability_register.*
git checkout HEAD -- frameworks/native/ability/native/ability_impl_factory.*
git checkout HEAD -- interfaces/kits/native/ability/native/ability_impl_factory.h
git checkout HEAD -- frameworks/native/ability/native/BUILD.gn
```

## 验证补丁应用

应用补丁后，检查以下文件是否存在：

```bash
ls -l frameworks/native/ability/native/ability_model_manager.*
ls -l frameworks/native/ability/native/fa_module_loader.*
ls -l frameworks/native/ability/native/fa_ability_register.*
```

并验证BUILD.gn中是否包含`abilitykit_fa`库配置。

---
*生成时间: 2026-02-03*
