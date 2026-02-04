# FA模型动态加载 - 实施总结

## 已完成的修改

### 1. 新创建的核心文件

| 文件 | 路径 | 说明 |
|-----|------|------|
| ability_model_manager.h | frameworks/native/ability/native/ | 模型管理器头文件 |
| ability_model_manager.cpp | frameworks/native/ability/native/ | 模型管理器实现 |
| fa_module_loader.h | frameworks/native/ability/native/ | FA模块加载器头文件 |
| fa_module_loader.cpp | frameworks/native/ability/native/ | FA模块加载器实现 |
| fa_ability_register.h | frameworks/native/ability/native/ | FA能力注册接口 |
| fa_ability_register.cpp | frameworks/native/ability/native/ | FA能力注册实现 |

### 2. 修改的现有文件

| 文件 | 修改内容 |
|-----|---------|
| ability_impl_factory.h | 添加RegisterAbilityCreator方法和abilityCreators_成员 |
| ability_impl_factory.cpp | 支持FA/Stage模型区分和动态加载 |
| BUILD.gn | 1. 添加abilitykit_fa独立库构建配置<br>2. 在abilitykit_native中添加新文件<br>3. 从abilitykit_native中移除FA相关文件 |

### 3. BUILD.gn详细修改

#### 3.1 主库（abilitykit_native）修改

**新增文件**：
```gni
"${ability_runtime_native_path}/ability/native/ability_model_manager.cpp",
"${ability_runtime_native_path}/ability/native/fa_module_loader.cpp",
```

**移除文件（注释掉）**：
```gni
# "${ability_runtime_native_path}/ability/native/data_ability_helper_impl.cpp",
# "${ability_runtime_native_path}/ability/native/data_ability_impl.cpp",
# "${ability_runtime_native_path}/ability/native/service_ability_impl.cpp",
```

在`ability_runtime_graphics`条件中：
```gni
# "${ability_runtime_native_path}/ability/native/page_ability_impl.cpp",
```

#### 3.2 新增FA独立库

在BUILD.gn末尾添加了`abilitykit_fa`共享库，包含：
- FA模型特有实现（page_ability_impl、service_ability_impl、data_ability_impl等）
- FA模型的continuation相关代码
- FA辅助类
- 导出注册函数（CreatePageAbilityImpl等）

### 4. 架构说明

```
应用启动
    ↓
AbilityModelManager检测模型类型
    ↓
    ├─ Stage模型 → 直接创建UIAbility/Extension
    │                （不加载abilitykit_fa.so）
    │
    └─ FA模型 → dlopen加载abilitykit_fa.so
                 → 注册FA Ability创建器
                 → 创建PageAbility/ServiceAbility/DataAbility
```

## 编译和测试

### 编译命令

```bash
# 编译FA独立库
./build.sh --product-name <product> --build-target abilitykit_fa

# 编译主库
./build.sh --product-name <product> --build-target abilitykit_native

# 编译完整组件
./build.sh --product-name <product> --build-target ability_runtime
```

### 验证步骤

1. **检查SO库生成**：
   ```bash
   ls -l out/{product}/lib/libabilitykit_fa.so
   ls -l out/{product}/lib/libabilitykit_native.so
   ```

2. **检查符号导出**：
   ```bash
   nm -D out/{product}/lib/libabilitykit_fa.so | grep Create
   # 应该看到：CreatePageAbilityImpl, CreateServiceAbilityImpl, CreateDataAbilityImpl
   ```

3. **运行时验证**：
   - 启动Stage应用，检查日志中是否出现"Creating Stage ability implementation"
   - 启动FA应用，检查日志中是否出现"Loading FA modules dynamically"和"Creating FA ability implementation"

## 预期效果

| 指标 | 预期 |
|-----|------|
| Stage应用内存节省 | ~1MB |
| 系统级内存节省（20进程） | ~20MB |
| FA应用首次加载延迟 | ~15-25ms |

## 注意事项

1. **编译顺序**：确保abilitykit_fa在abilitykit_native之后才能被dlopen
2. **符号导出**：FA库中的CreateXXX函数必须正确导出
3. **降级开关**：如需回滚，恢复ability_impl_factory.cpp并重新包含FA文件即可

## 后续工作

1. 添加单元测试
2. 添加集成测试
3. 性能测试和优化
4. 更新bundle.json
5. 更新上层BUILD.gn

---
*实施日期: 2026-02-03*
