# AbilityManagerClient Mock 实现说明

## 概述

为 `uri_permission_manager_stub_impl_test.cpp` 添加了 AbilityManagerClient 的 mock 实现，解决了对 AbilityManagerClient 依赖的问题。

## 创建的文件

### 1. Mock 头文件

#### `mock/include/mock_ability_manager_client.h`
- **作用**: 定义 `MockIAbilityManagerCollaborator` 类，实现 `IAbilityManagerCollaborator` 接口
- **特点**:
  - 仅实现 uripermmgr 需要的方法：`GrantUriPermission()` 和 `RevokeUriPermission()`
  - 使用回调函数模式，允许测试用例自定义返回值
  - 其他方法返回默认值（0 或空实现）

#### `mock/include/ability_manager_client.h`
- **作用**: 替换真实的 `AbilityManagerClient` 类声明
- **特点**:
  - 单例模式
  - 提供 `SetMockCollaborator()` 方法设置 mock collaborator
  - 提供 `ResetMock()` 方法重置 mock 状态

### 2. Mock 实现文件

#### `mock/src/ability_manager_client.cpp`
- **作用**: 实现 `AbilityManagerClient` 类的方法
- **实现**:
  - `GetInstance()`: 返回单例实例
  - `GetAbilityManagerCollaborator()`: 返回 mock collaborator
  - `SetMockCollaborator()`: 设置 mock collaborator
  - `ResetMock()`: 重置所有静态成员

### 3. 测试文件修改

#### `src/uri_permission_manager_stub_impl_test.cpp`
- **添加的 include**:
  ```cpp
  #include "mock_ability_manager_client.h"
  #include "ability_manager_client.h"
  ```

- **添加的静态成员**:
  ```cpp
  static sptr<MockIAbilityManagerCollaborator> mockCollaborator_;
  static std::shared_ptr<AbilityManagerClient> mockAbilityClient_;
  ```

- **SetUpTestCase() 初始化**:
  ```cpp
  void UriPermissionManagerStubImplTest::SetUpTestCase()
  {
      // 创建 Mock Collaborator
      mockCollaborator_ = new MockIAbilityManagerCollaborator();

      // 设置默认行为：返回成功
      mockCollaborator_->SetGrantUriPermissionCallback(...);
      mockCollaborator_->SetRevokeUriPermissionCallback(...);

      // 设置到 AbilityManagerClient
      AbilityManagerClient::SetMockCollaborator(mockCollaborator_);
  }
  ```

### 4. BUILD.gn 修改

添加了 mock 实现文件到 sources：
```gn
sources += [
    ...
    "mock/src/ability_manager_client.cpp",
]
```

## 使用方法

### 基本使用

测试会自动在 `SetUpTestCase()` 中初始化 mock，默认返回 `ERR_OK`。

### 自定义返回值

在测试用例中可以修改 mock 的行为：

```cpp
// 修改 GrantUriPermission 返回值
mockCollaborator_->SetGrantUriPermissionCallback(
    [](const std::vector<std::string>& uris, uint32_t flag,
       uint32_t targetTokenId, const std::string& bundleName) -> int32_t {
        // 自定义逻辑
        return ERR_INVALID_VALUE;
    });

// 修改 RevokeUriPermission 返回值
mockCollaborator_->SetRevokeUriPermissionCallback(
    [](uint32_t tokenId) -> int32_t {
        // 自定义逻辑
        return ERR_NO_INIT;
    });
```

## 接口覆盖

### 已实现的 IAbilityManagerCollaborator 方法

| 方法 | 用途 | 测试覆盖 |
|------|------|---------|
| `GrantUriPermission()` | 授予 Content URI 权限 | ✅ |
| `RevokeUriPermission()` | 撤销 Content URI 权限 | ✅ |

### 返回默认值的方法

其他 `IAbilityManagerCollaborator` 方法（如 `NotifyStartAbility()`, `NotifyPreloadAbility()` 等）返回默认值 0，因为 uripermmgr 不使用这些方法。

## 依赖关系

```
uri_permission_manager_stub_impl_test.cpp
    ├── mock_ability_manager_client.h (MockIAbilityManagerCollaborator)
    ├── ability_manager_client.h (简化版 AbilityManagerClient)
    └── ability_manager_client.cpp (实现)

ability_manager_client.h
    └── iability_manager_collaborator.h (系统接口)
```

## 编译验证

编译命令：
```bash
./build.sh --product-name <product> --build-target uripermmgr_unit_test
```

## 注意事项

1. **include 顺序**: `ability_manager_client.h` 必须在 `mock_ability_manager_client.h` 之后 include，以确保使用 mock 版本

2. **生命周期管理**:
   - Mock collaborator 在 `SetUpTestCase()` 中创建
   - 可在 `TearDownTestCase()` 中调用 `AbilityManagerClient::ResetMock()` 清理

3. **线程安全**: 当前实现不是线程安全的，单元测试通常在单线程中运行

4. **与真实实现的区别**:
   - 真实的 `AbilityManagerClient` 通过 IPC 与 AbilityManagerService 通信
   - Mock 版本直接返回预设值，不进行 IPC 调用

## 测试场景覆盖

以下测试场景使用了 AbilityManagerClient mock：

1. **Content URI 权限授予**: `GrantUriPermission_ContentUri_001`
2. **Content URI 权限撤销**: `RevokeUriPermission_ContentUri_001`
3. **多个 Content URI 批量处理**: 涉及 `GrantContentUriPermission()`
4. **权限验证**: `VerifyUriPermission()` 涉及 content URI 的场景

## 后续改进

如需扩展 mock 功能，可以：

1. 添加更多的 IAbilityManagerCollaborator 方法实现
2. 支持更复杂的返回值逻辑（如根据参数返回不同值）
3. 添加调用计数和参数验证功能
4. 支持线程安全
