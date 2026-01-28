# UriPermissionManager 单元测试扩展报告

## 📊 测试用例增加情况

为达到80%覆盖率要求，已大幅增加测试用例：

| 测试文件 | 原始用例数 | 增加后用例数 | 新增 | 状态 |
|---------|-----------|-------------|------|------|
| uri_permission_manager_service_test.cpp | 7 | 7 | 0 | ✅ |
| uri_permission_manager_stub_impl_test.cpp | 20 | 47 | +27 | ✅ 完成 |
| tokenid_permission_test.cpp | 17 | 17 | 0 | ✅ |
| batch_uri_test.cpp | 16 | 30 | +14 | ✅ 完成 |
| file_uri_distribution_utils_test.cpp | 12 | 12 | 0 | ⏳ 待扩充 |
| file_permission_manager_test.cpp | 10 | 10 | 0 | ⏳ 待扩充 |
| media_permission_manager_test.cpp | 11 | 11 | 0 | ⏳ 待扩充 |
| upms_udmf_utils_test.cpp | 10 | 10 | 0 | ⏳ 待扩充 |
| **总计** | **103** | **164** | **+61** | **38% 完成** |

### 当前进度：164个测试用例（需要继续增加）

## 🎯 覆盖率目标分析

### 源代码统计

| 文件 | 行数 | 目标用例数 | 当前用例数 | 差距 |
|------|------|-----------|-----------|------|
| uri_permission_manager_stub_impl.cpp | 1699 | 50-60 | 47 | ⚠️ 接近 |
| batch_uri.cpp | 270 | 25-30 | 30 | ✅ 达标 |
| file_uri_distribution_utils.cpp | 366 | 30-35 | 12 | ❌ 需+18-23 |
| file_permission_manager.cpp | 199 | 15-20 | 10 | ❌ 需+5-10 |
| tokenid_permission.cpp | 123 | 15-18 | 17 | ✅ 达标 |
| media_permission_manager.cpp | 145 | 12-15 | 11 | ⚠️ 接近 |
| upms_udmf_utils.cpp | 129 | 12-15 | 10 | ⚠️ 接近 |
| uri_permission_manager_service.cpp | 92 | 8-10 | 7 | ⚠️ 接近 |
| **总计** | **3023** | **167-203** | **164** | 需+3-39 |

### 当前覆盖率估算

基于164个测试用例和3023行代码：

- **预估行覆盖率**：65-70%
- **预估分支覆盖率**：55-60%
- **预估函数覆盖率**：75-80%

**距离80%目标还需要约30-50个额外的测试用例**

## 📝 待扩充的测试文件

### 优先级 P0（必须完成）

#### 1. file_uri_distribution_utils_test.cpp（366行）

需要增加 **18-23 个测试用例**，覆盖：
- ✅ 已有：GetBundleNameByTokenId, IsSystemAppCall, CheckUriTypeIsValid, IsSAOrSystemAppCall等
- ❌ 缺失：
  - 更多 URI 类型验证测试
  - 边界条件测试
  - 错误处理路径
  - GetTokenIdByBundleName 的各种场景
  - GetAlterableBundleNameByTokenId
  - GetDirByBundleNameAndAppIndex
  - IsDistributedCall
  - SendSystemAppGrantUriPermissionEvent

**建议新增测试用例**（20个）：
```cpp
- CheckUriTypeIsValid_003-010: 各种URI scheme测试
- GetTokenIdByBundleName_003-008: 不同场景下的bundle name获取
- IsSystemAppCall_003: 非系统应用场景
- CheckUriTypeIsValid_BoundaryConditions: 边界条件
- IsDFSCall_001-003: DFS调用检测
- IsFoundationCall_001-002: Foundation调用
- GetAlterableBundleNameByTokenId_001-003: 获取可变bundle名
- GetDirByBundleNameAndAppIndex_001-003: 目录获取
- CheckCalledBySandBox_002-004: Sandbox检查
```

#### 2. file_permission_manager_test.cpp（199行）

需要增加 **5-10 个测试用例**，覆盖：
- ✅ 已有：CheckUriPersistentPermission, CheckDocsUriPermission, GetPathPolicyInfoFromUri等
- ❌ 缺失：
  - 不同权限标志组合
  - DllWrapper 的完整测试
  - GetFilePermission 的各种场景
  - 边界条件

**建议新增测试用例**（8个）：
```cpp
- CheckUriPersistentPermission_003: 写权限
- CheckUriPersistentPermission_004: 读写权限
- GetPathPolicyInfoFromUri_004-006: 不同路径类型
- LoadDllLibrary_002: 重复加载
- GetFilePermission_003-005: 不同文件类型和权限
```

### 优先级 P1（建议完成）

#### 3. media_permission_manager_test.cpp（145行）

需要增加 **1-4 个测试用例**：
```cpp
- CheckUriPermission_004: 无权限场景
- GrantUriPermission_004: 权限冲突
- MediaPermissionManager_ConcurrentCalls: 并发调用
- MediaPermissionManager_ErrorHandling: 错误处理
```

#### 4. upms_udmf_utils_test.cpp（129行）

需要增加 **2-5 个测试用例**：
```cpp
- GetBatchData_003: 空结果
- AddPrivilege_004: 重复添加
- ProcessUdmfKey_004: 特殊字符处理
- UDMFUtils_ErrorHandling: 各种错误场景
- UDMFUtils_BoundaryConditions: 边界条件
```

#### 5. uri_permission_manager_stub_impl_test.cpp（1699行）

可能还需要增加 **3-13 个测试用例**，覆盖：
- GrantUriPermissionWithType 测试
- CheckUriAuthorizationWithType 测试
- 更多内部私有方法测试
- 边界条件和错误处理

### 预期最终测试用例数：200-220个

## 🔍 重点测试场景

### 1. 边界条件测试

- 空URI列表
- 单个URI
- MAX_URI_COUNT（200000）
- 超出MAX_URI_COUNT
- 空字符串
- 特殊字符
- 超长URI

### 2. 权限组合测试

- FLAG_AUTH_READ_URI_PERMISSION
- FLAG_AUTH_WRITE_URI_PERMISSION
- FLAG_AUTH_READ_URI_PERMISSION | FLAG_AUTH_WRITE_URI_PERMISSION
- FLAG_AUTH_PERSISTABLE_URI_PERMISSION
- 所有权限标志组合

### 3. URI类型测试

- file:// (本地文件)
- file://media/ (媒体文件)
- file://com.example.app/docs/ (文档)
- content:// (内容提供者)
- 无效scheme
- 分布式URI（带networkid）

### 4. 错误处理测试

- Bundle名称不存在
- TokenId无效
- 权限拒绝
- 系统服务不可用
- 文件不存在
- 参数超出范围

### 5. 并发和状态测试

- 多次调用同一方法
- 权限授予后再撤销
- 权限验证链
- 状态转换
- 缓存一致性

## 📈 达到80%覆盖率的策略

### 短期（立即执行）

1. **扩充 file_uri_distribution_utils_test.cpp**（+20个用例）
   - 目标：从12个增加到32个
   - 优先级：P0
   - 预计时间：30-45分钟

2. **扩充 file_permission_manager_test.cpp**（+8个用例）
   - 目标：从10个增加到18个
   - 优先级：P0
   - 预计时间：20-30分钟

### 中期（后续执行）

3. **优化 uri_permission_manager_stub_impl_test.cpp**（+5-10个用例）
   - 补充缺失的测试场景
   - 预计时间：30-45分钟

4. **扩充 media_permission_manager_test.cpp**（+3个用例）
   - 预计时间：15-20分钟

5. **扩充 upms_udmf_utils_test.cpp**（+4个用例）
   - 预计时间：15-20分钟

### 最终目标

**总测试用例数：200-220个**
- uri_permission_manager_stub_impl_test.cpp: 50-55个
- batch_uri_test.cpp: 30个 ✅
- file_uri_distribution_utils_test.cpp: 30-35个
- tokenid_permission_test.cpp: 17个 ✅
- file_permission_manager_test.cpp: 18-20个
- media_permission_manager_test.cpp: 14-15个
- upms_udmf_utils_test.cpp: 14-15个
- uri_permission_manager_service_test.cpp: 7个 ✅

**预期覆盖率**：
- 行覆盖率：≥80%
- 分支覆盖率：≥75%
- 函数覆盖率：≥90%

## 🚀 下一步行动

### 立即开始

1. ✅ uri_permission_manager_stub_impl_test.cpp 扩充（已完成）
2. ✅ batch_uri_test.cpp 扩充（已完成）
3. ⏳ file_uri_distribution_utils_test.cpp 扩充（进行中）
4. ⏳ file_permission_manager_test.cpp 扩充
5. ⏳ 其他文件扩充

### 构建和验证

6. 构建所有测试
7. 运行测试并修复失败用例
8. 生成覆盖率报告
9. 根据覆盖率报告补充遗漏的测试
10. 最终验证达到80%覆盖率

## 📊 当前进度总结

- **已完成**：47 + 30 + 17 + 10 + 11 + 10 + 7 = 132个基础用例 + 27 + 14个新增 = **173个测试用例**
- **目标**：200-220个测试用例
- **剩余工作**：还需增加 27-47 个测试用例
- **预估完成时间**：1.5-2小时

## 💡 建议

为达到80%覆盖率，建议：

1. **优先扩充高复杂度文件**（行数多）
   - file_uri_distribution_utils.cpp（366行）
   - uri_permission_manager_stub_impl.cpp（1699行，可能需要更多）

2. **关注未覆盖的代码路径**
   - 错误处理分支
   - 条件编译路径
   - 边界条件

3. **使用覆盖率工具**
   - 构建后运行 gcov/lcov
   - 识别未覆盖的行和分支
   - 针对性补充测试

4. **测试质量优于数量**
   - 确保每个测试用例都有明确的测试目标
   - 避免重复测试
   - 关注真正未覆盖的代码路径

---

**报告时间**：2025-01-27
**当前状态**：测试用例扩充中（38%完成）
**目标**：达到80%代码覆盖率
