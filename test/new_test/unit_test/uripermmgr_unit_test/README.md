# UriPermissionManager 单元测试

## 项目概述

本项目为 `services/uripermmgr` 中的 C++ 文件创建单元测试，使用新测试框架 (`test/new_test/unit_test`)。

## 目录结构

```
uripermmgr_unit_test/
├── BUILD.gn                           # 构建配置
├── README.md                          # 本文件
├── IMPLEMENTATION_STATUS.md           # 实现状态跟踪
├── mock/                              # Mock 文件目录
│   ├── include/                       # Mock 头文件
│   └── src/                          # Mock 实现文件
└── src/                              # 测试源文件
    └── uri_permission_manager_service_test.cpp
```

## 已完成工作

### 1. Mock 层（100% 完成）

所有必要的 Mock 文件已创建完成，包括：

- `mock_system_ability_manager.h/cpp` - 系统能力管理器
- `mock_accesstoken_kit.h/cpp` - 访问令牌工具
- `mock_tokenid_kit.h/cpp` - Token ID 工具
- `mock_bundle_mgr_client.h/cpp` - Bundle 管理客户端
- `mock_permission_verification.h/cpp` - 权限验证
- `mock_sandbox_manager_kit.h/cpp` - 沙箱管理器
- `mock_media_permission_manager.h/cpp` - 媒体权限管理器
- `mock_storage_manager.h/cpp` - 存储管理器
- `mock_file_permission_manager.h/cpp` - 文件权限管理器
- `mock_udmf_client.h/cpp` - UDMF 客户端

此外，BUILD.gn 已配置为使用现有的全局 mock（在 `test/new_test/mock/` 中）。

### 2. 测试框架（12.5% 完成）

已完成的测试文件：

- ✅ `uri_permission_manager_service_test.cpp`（7 个测试用例）

待实现的测试文件：

- ⏳ `uri_permission_manager_stub_impl_test.cpp`（最重要，30-40 个用例）
- ⏳ `tokenid_permission_test.cpp`（10-15 个用例）
- ⏳ `media_permission_manager_test.cpp`（8-12 个用例）
- ⏳ `file_permission_manager_test.cpp`（10-15 个用例）
- ⏳ `file_uri_distribution_utils_test.cpp`（12-18 个用例）
- ⏳ `batch_uri_test.cpp`（15-20 个用例）
- ⏳ `upms_udmf_utils_test.cpp`（6-10 个用例）

### 3. 构建配置（100% 完成）

- ✅ `uripermmgr_unit_test/BUILD.gn` - 完整的构建配置
- ✅ `test/new_test/unit_test/BUILD.gn` - 已更新包含新测试

## 构建和运行

### 构建测试

```bash
# 构建特定测试
./build.sh --product-name <product> --build-target uripermmgr_unit_test

# 或构建所有测试
./build.sh --product-name <product> --build-target ability_runtime_test
```

### 运行测试

```bash
# 运行特定测试
run -t UT -ts uripermmgr_unit_test

# 运行并生成覆盖率报告
run -t UT -ts uripermmgr_unit_test --coverage
```

## 测试策略

### 1. Mock 策略

- 使用 `oh_mock_utils.h` 提供的轻量级 Mock 宏
- 复用现有的全局 mock（`test/new_test/mock/`）
- 条件编译的 mock 使用 `#ifdef` 包裹

### 2. 测试覆盖重点

- **核心功能**：URI 权限授予、撤销、验证
- **边界条件**：大量 URI、无效 URI、空 URI 列表
- **异常处理**：权限冲突、非法参数、系统错误
- **并发安全**：多线程场景（如需要）

### 3. 测试用例命名规范

```cpp
HWTEST_F(TestClassName, TestFeature_TestScenario_XXX, TestSize.Level1)
```

例如：
- `UriPermissionManagerService_LifeCycle_001`
- `VerifyUriPermission_ValidUri_WithReadPermission_001`

## 下一步工作

### 优先级 P0（必须完成）

1. **uri_permission_manager_stub_impl_test.cpp**（2-3 天）
   - 这是最复杂的测试文件
   - 覆盖核心权限管理逻辑
   - 30-40 个测试用例

2. **batch_uri_test.cpp**（1 天）
   - 批量 URI 处理逻辑
   - 15-20 个测试用例

3. **file_uri_distribution_utils_test.cpp**（1 天）
   - URI 工具函数测试
   - 12-18 个测试用例

### 优先级 P1（重要）

4. **tokenid_permission_test.cpp**（0.5-1 天）
5. **file_permission_manager_test.cpp**（0.5-1 天）
6. **media_permission_manager_test.cpp**（0.5-1 天）

### 优先级 P2（可选）

7. **upms_udmf_utils_test.cpp**（0.5 天，条件编译）

## 潜在风险和缓解措施

### 高风险

| 风险 | 缓解措施 |
|------|---------|
| Mock 复杂性高 | 分阶段实现 Mock，优先 Mock 核心接口 |
| 条件编译依赖 | 使用条件编译宏隔离测试 |
| 单例模式依赖 | 使用 `#define private public` 访问私有成员 |
| IPC 调用依赖 | Mock IPC 相关全局函数 |

### 中风险

| 风险 | 缓解措施 |
|------|---------|
| 动态库加载 | Mock 动态库加载函数 |
| 大量边界条件 | 优先测试核心流程，使用覆盖率工具验证 |
| 状态管理复杂 | 每个测试用例独立初始化和清理 |

## 参考资料

### 现有测试示例

- `test/unittest/uri_permission_manager_test/` - 传统测试框架
- `test/new_test/unit_test/ability_connection_unit_test/` - 新测试框架
- `test/new_test/unit_test/ability_auto_startup_client_unit_test/` - 客户端测试

### Mock 工具

- `test/new_test/mock_utils/oh_mock_utils.h` - Mock 工具宏
- `test/new_test/mock/` - 全局 Mock 组件

### 源文件

- 源代码：`services/uripermmgr/src/`
- 头文件：`services/uripermmgr/include/`
- 构建配置：`services/uripermmgr/BUILD.gn`

## 贡献指南

### 添加新测试用例

1. 在 `src/` 目录创建对应的测试文件
2. 使用 `HWTEST_F` 宏定义测试用例
3. 确保每个测试用例独立运行
4. 在 `SetUp()` 和 `TearDown()` 中初始化和清理状态

### Mock 新接口

1. 在 `mock/include/` 创建 Mock 头文件
2. 使用 `OH_MOCK_METHOD` 或 `OH_MOCK_METHOD_RET_SPTR` 宏
3. 在 `mock/src/` 创建对应的实现文件
4. 更新 BUILD.gn 添加必要的依赖

### 代码规范

- 遵循项目编码规范
- 使用有意义的测试用例名称
- 添加必要的注释和文档
- 确保无编译警告

## 联系方式

如有问题，请联系项目维护者或参考项目文档。

---

**最后更新时间**：2025-01-27
**当前状态**：Mock 层完成，测试框架搭建完成，开始测试用例实现
**预计完成时间**：7-10 天
